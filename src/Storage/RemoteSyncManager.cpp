#include "RemoteSyncManager.h"
#include "GameData.h"
#include "Storage/LeaderboardManager.h"
#include "Storage/StatisticsManager.h"

#include <iomanip>
#include <sstream>
#include <cctype>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define NOGDI
#define NOUSER
#include <windows.h>
#include <winhttp.h>
#pragma comment(lib, "winhttp.lib")
#else
#include <curl/curl.h>
#endif

namespace {

constexpr const char* kClientVersion = "1.0.0";
constexpr int kHttpResolveTimeoutMs = 1500;
constexpr int kHttpConnectTimeoutMs = 2000;
constexpr int kHttpSendTimeoutMs = 2500;
constexpr int kHttpReceiveTimeoutMs = 2500;

std::string BuildRemoteUrl(const std::string& serverIp, int serverPort, bool useHttps) {
    const std::string host = serverIp.empty() ? std::string("127.0.0.1") : serverIp;
    const int port = (serverPort > 0 && serverPort <= 65535) ? serverPort : 8000;
    std::ostringstream url;
    url << (useHttps ? "https://" : "http://") << host << ':' << port;
    return url.str();
}

RemoteSyncConfig GetDefaultConfig() {
    RemoteSyncConfig cfg;
    cfg.serverIp = "127.0.0.1";
    cfg.serverPort = 8000;
    cfg.useHttps = false;
    cfg.username.clear();
    return cfg;
}

std::string ToLowerCopy(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return value;
}

std::string UrlEncodeComponent(const std::string& value) {
    std::ostringstream encoded;
    encoded << std::hex << std::uppercase;
    for (unsigned char ch : value) {
        if ((ch >= 'a' && ch <= 'z') ||
            (ch >= 'A' && ch <= 'Z') ||
            (ch >= '0' && ch <= '9') ||
            ch == '-' || ch == '_' || ch == '.' || ch == '~') {
            encoded << static_cast<char>(ch);
        } else {
            encoded << '%' << std::setw(2) << std::setfill('0') << static_cast<int>(ch);
        }
    }
    return encoded.str();
}

#ifdef _WIN32
std::wstring ToWide(const std::string& value) {
    return std::wstring(value.begin(), value.end());
}
#endif

#ifndef _WIN32
bool EnsureCurlInitialized() {
    static bool attempted = false;
    static bool initialized = false;
    if (!attempted) {
        attempted = true;
        CURLcode code = curl_global_init(CURL_GLOBAL_DEFAULT);
        if (code == CURLE_OK) {
            initialized = true;
        } else {
            std::cout << "[RemoteSync] Failed to initialize libcurl (" << curl_easy_strerror(code) << ")." << std::endl;
        }
    }
    return initialized;
}
#endif

nlohmann::json MoveToJson(const MoveRecord& move) {
    auto maskToList = [](uint16_t mask) {
        nlohmann::json values = nlohmann::json::array();
        for (int index = 0; index < 9; ++index) {
            if ((mask & static_cast<uint16_t>(1u << index)) != 0) {
                values.push_back(index + 1);
            }
        }
        return values;
    };

    nlohmann::json value;
    value["tileNumber"] = move.tileNumber;
    value["value"] = move.value;
    value["timestamp"] = move.timestamp;
    value["notesMask"] = move.notesMask;
    value["removedMask"] = move.removedMask;
    value["notes"] = maskToList(move.notesMask);
    value["removedCandidates"] = maskToList(move.removedMask);
    value["autoCandidatesEnabled"] = move.autoCandidatesEnabled;
    value["action"] = static_cast<int>(move.action);
    return value;
}

MoveRecord JsonToMove(const nlohmann::json& value) {
    auto listToMask = [](const nlohmann::json& listValue) {
        uint16_t mask = 0;
        if (!listValue.is_array()) {
            return mask;
        }
        for (const auto& item : listValue) {
            if (!item.is_number_integer()) {
                continue;
            }
            int candidate = item.get<int>();
            if (candidate >= 1 && candidate <= 9) {
                mask |= static_cast<uint16_t>(1u << (candidate - 1));
            }
        }
        return mask;
    };

    MoveRecord move;
    move.tileNumber = value.value("tileNumber", 0);
    move.value = value.value("value", 0);
    move.timestamp = value.value("timestamp", 0.0);
    move.notesMask = static_cast<uint16_t>(value.value("notesMask", 0));
    move.removedMask = static_cast<uint16_t>(value.value("removedMask", 0));
    if (move.notesMask == 0 && value.contains("notes")) {
        move.notesMask = listToMask(value["notes"]);
    }
    if (move.removedMask == 0 && value.contains("removedCandidates")) {
        move.removedMask = listToMask(value["removedCandidates"]);
    }
    move.autoCandidatesEnabled = value.value("autoCandidatesEnabled", false);
    move.action = static_cast<MoveAction>(value.value("action", 0));
    return move;
}

} // namespace

RemoteSyncManager::RemoteSyncManager() {
    configPath = std::filesystem::path("./remote_sync.json");
    failedSubmissionsPath = std::filesystem::path("./remote_failed_submissions.jsonl");
    config = GetDefaultConfig();
    LoadConfig();

    auto now = std::chrono::steady_clock::now();
    lastSyncAttempt = now;
    lastSuccessfulSync = now;
    lastOutboundPacket = now;
    lastReconnectAttempt = now - std::chrono::seconds(30);

#ifndef _WIN32
    curlAvailable = EnsureCurlInitialized();
#endif

    StartInitialPullAsync();
}

RemoteSyncManager::~RemoteSyncManager() {
    JoinInitialPullThread();
    JoinKeepAliveThread();
    JoinSyncThread();
}

void RemoteSyncManager::LoadConfig() {
    if (!std::filesystem::exists(configPath)) {
        config = GetDefaultConfig();
        SaveConfig();
        return;
    }

    std::ifstream input(configPath);
    if (!input.good()) {
        config = GetDefaultConfig();
        return;
    }

    try {
        nlohmann::json data = nlohmann::json::parse(input, nullptr, true, true);
        config.serverIp = data.value("serverIp", std::string(""));
        config.serverPort = data.value("serverPort", 8000);
        config.useHttps = data.value("useHttps", false);

        config.username = data.value("username", "");
    } catch (const std::exception&) {
        config = GetDefaultConfig();
    }

    if (config.serverIp.empty()) {
        config.serverIp = "127.0.0.1";
    }
    if (config.serverPort <= 0 || config.serverPort > 65535) {
        config.serverPort = 8000;
    }
    authToken.clear();

    remoteUrlValid = ParseRemoteUrl();
}

void RemoteSyncManager::SaveConfig() const {
    nlohmann::json data;
    data["serverIp"] = config.serverIp;
    data["serverPort"] = config.serverPort;
    data["useHttps"] = config.useHttps;
    data["username"] = config.username;

    std::ofstream output(configPath);
    output << data.dump(4);
}

const RemoteSyncConfig& RemoteSyncManager::GetConfig() const {
    return config;
}

void RemoteSyncManager::UpdateConfig(const RemoteSyncConfig& newConfig) {
    JoinInitialPullThread();
    JoinKeepAliveThread();
    JoinSyncThread();

    config = newConfig;
    if (config.serverIp.empty()) {
        config.serverIp = "127.0.0.1";
    }
    if (config.serverPort <= 0 || config.serverPort > 65535) {
        config.serverPort = 8000;
    }
    authToken.clear();
    warnedMissingCredentials = false;
    SaveConfig();
    remoteUrlValid = ParseRemoteUrl();
    initialPullPerformed = false;
    keepAliveInFlight.store(false);
    lastOutboundPacket = std::chrono::steady_clock::now();
    lastReconnectAttempt = lastOutboundPacket - std::chrono::seconds(30);
    StartInitialPullAsync();
}

void RemoteSyncManager::QueueLeaderboardUpdate() {
    requestedLeaderboardUsernameFilter.clear();
    leaderboardDirty = true;
    StartSyncAsync();
}

void RemoteSyncManager::QueueLeaderboardRefreshForPlayer(const std::string& username) {
    requestedLeaderboardUsernameFilter = username;
    leaderboardDirty = true;
    StartSyncAsync();
}

void RemoteSyncManager::QueueLeaderboardSubmission(const LeaderboardEntry& entry) {
    {
        std::lock_guard<std::mutex> lock(syncMutex);
        pendingLeaderboardSubmissions.push_back(entry);
        leaderboardDirty = true;
    }
    StartSyncAsync();
}

void RemoteSyncManager::QueueStatisticsUpdate() {
    statisticsDirty = true;
    StartSyncAsync();
}

void RemoteSyncManager::Update() {
    if (keepAliveThread.joinable() && !keepAliveInFlight.load()) {
        keepAliveThread.join();
    }

    if (syncThread.joinable() && !syncInFlight.load()) {
        syncThread.join();
    }

    constexpr auto reconnectRetryInterval = std::chrono::seconds(30);
    const auto now = std::chrono::steady_clock::now();

    if (connectionState.load() == ConnectionState::Error &&
        !config.serverIp.empty() &&
        !config.username.empty() &&
        (now - lastReconnectAttempt) >= reconnectRetryInterval) {
        lastReconnectAttempt = now;
        if (!initialPullPerformed) {
            StartInitialPullAsync();
        } else {
            StartKeepAliveAsync();
        }
        return;
    }

    if (!initialPullPerformed) {
        return;
    }

    if (connectionState.load() != ConnectionState::Connected) {
        return;
    }

    if (HasDirtyData() && !syncInFlight.load()) {
        StartSyncAsync();
        return;
    }

    if (keepAliveInFlight.load()) {
        return;
    }

    constexpr auto keepAliveInterval = std::chrono::minutes(14);
    if (now - lastOutboundPacket < keepAliveInterval) {
        return;
    }

    StartKeepAliveAsync();
}

void RemoteSyncManager::ForceSync() {
    if (!HasDirtyData()) {
        return;
    }

    std::lock_guard<std::mutex> lock(syncMutex);
    try {
        if (PerformSync()) {
            lastSuccessfulSync = std::chrono::steady_clock::now();
        }
    } catch (const std::exception& e) {
        std::cout << "[RemoteSync] ForceSync failed: " << e.what() << std::endl;
        connectionState.store(ConnectionState::Error);
    } catch (...) {
        std::cout << "[RemoteSync] ForceSync failed with unknown error." << std::endl;
        connectionState.store(ConnectionState::Error);
    }
}

void RemoteSyncManager::StartSyncAsync() {
    if (syncInFlight.load()) {
        return;
    }

    if (!HasDirtyData()) {
        return;
    }

    if (config.serverIp.empty() || config.username.empty()) {
        return;
    }

    if (syncThread.joinable()) {
        syncThread.join();
    }

    syncInFlight.store(true);
    syncThread = std::thread([this]() {
        try {
            {
                std::lock_guard<std::mutex> lock(syncMutex);
                if (PerformSync()) {
                    lastSuccessfulSync = std::chrono::steady_clock::now();
                }
            }
        } catch (const std::exception& e) {
            std::cout << "[RemoteSync] Background sync failed: " << e.what() << std::endl;
            connectionState.store(ConnectionState::Error);
        } catch (...) {
            std::cout << "[RemoteSync] Background sync failed with unknown error." << std::endl;
            connectionState.store(ConnectionState::Error);
        }
        syncInFlight.store(false);
    });
}

void RemoteSyncManager::JoinSyncThread() {
    if (syncThread.joinable()) {
        syncThread.join();
    }
    syncInFlight.store(false);
}

void RemoteSyncManager::StartInitialPullAsync() {
    if (config.username.empty()) {
        connectionState.store(ConnectionState::NotConfigured);
        return;
    }

#ifndef _WIN32
    if (!curlAvailable) {
        connectionState.store(ConnectionState::Disabled);
        return;
    }
#endif

    connectionState.store(ConnectionState::Connecting);

    if (initialPullThread.joinable()) {
        initialPullThread.join();
    }

    initialPullThread = std::thread([this]() {
        try {
            std::lock_guard<std::mutex> lock(syncMutex);
            PerformInitialPull();

            if (initialPullPerformed) {
                connectionState.store(ConnectionState::Connected);
            } else {
                if (config.username.empty()) {
                    connectionState.store(ConnectionState::NotConfigured);
                } else {
                    connectionState.store(ConnectionState::Error);
                }
            }
        } catch (const std::exception& e) {
            std::cout << "[RemoteSync] Initial pull failed: " << e.what() << std::endl;
            connectionState.store(ConnectionState::Error);
        } catch (...) {
            std::cout << "[RemoteSync] Initial pull failed with unknown error." << std::endl;
            connectionState.store(ConnectionState::Error);
        }
    });
}

void RemoteSyncManager::JoinInitialPullThread() {
    if (initialPullThread.joinable()) {
        initialPullThread.join();
    }
}

void RemoteSyncManager::StartKeepAliveAsync() {
    if (keepAliveInFlight.load()) {
        return;
    }

    if (keepAliveThread.joinable()) {
        keepAliveThread.join();
    }

    keepAliveInFlight.store(true);
    keepAliveThread = std::thread([this]() {
        bool ok = false;
        try {
            {
                std::lock_guard<std::mutex> lock(syncMutex);
                if (EnsureAuthToken()) {
                    if (!versionPolicyChecked) {
                        FetchClientPolicyFromServer();
                    }
                    HttpResponse response;
#ifdef _WIN32
                    ok = SendRequest(L"GET", BuildPath("api/auth/me"), "", response) && response.statusCode == 200;
#else
                    ok = SendCurlRequest("GET", BuildUrl("api/auth/me"), "", response) && response.statusCode == 200;
#endif
                }
            }
        } catch (const std::exception& e) {
            std::cout << "[RemoteSync] Keepalive failed: " << e.what() << std::endl;
            ok = false;
        } catch (...) {
            std::cout << "[RemoteSync] Keepalive failed with unknown error." << std::endl;
            ok = false;
        }

        if (ok) {
            connectionState.store(ConnectionState::Connected);
        } else if (config.username.empty()) {
            connectionState.store(ConnectionState::NotConfigured);
        } else {
            connectionState.store(ConnectionState::Error);
        }

        keepAliveInFlight.store(false);
    });
}

void RemoteSyncManager::JoinKeepAliveThread() {
    if (keepAliveThread.joinable()) {
        keepAliveThread.join();
    }
    keepAliveInFlight.store(false);
}

void RemoteSyncManager::PerformInitialPull() {
    if (initialPullPerformed) {
        return;
    }

    if (config.serverIp.empty() || config.username.empty()) {
        if (!warnedMissingCredentials) {
            std::cout << "[RemoteSync] Server IP or username missing; skipping initial sync pull." << std::endl;
            warnedMissingCredentials = true;
        }
        return;
    }

    if (!remoteUrlValid) {
        remoteUrlValid = ParseRemoteUrl();
        if (!remoteUrlValid) {
            std::cout << "[RemoteSync] Invalid remote URL; skipping initial pull." << std::endl;
            return;
        }
    }

#ifndef _WIN32
    if (!curlAvailable) {
        std::cout << "[RemoteSync] libcurl unavailable; skipping initial pull." << std::endl;
        initialPullPerformed = true;
        return;
    }
#endif

    if (!EnsureAuthToken()) {
        return;
    }

    initialPullPerformed = true;
    warnedMissingCredentials = false;
    FetchClientPolicyFromServer();
    PullGlobalLeaderboardFromServer();
    FetchMyStatsFromServer();
}

void RemoteSyncManager::OnExit() {
    JoinInitialPullThread();
    JoinKeepAliveThread();
    JoinSyncThread();
    // Don't block shutdown with extra network calls when offline.
    if (connectionState.load() != ConnectionState::Connected) {
        return;
    }
    // Final best-effort flush if something remained dirty due to transient failures.
    ForceSync();
}

void RemoteSyncManager::LogFailedSubmission(const nlohmann::json& payload, int statusCode, const std::string& reason) const {
    nlohmann::json line;
    line["timestamp"] = TimeToIso8601(std::time(nullptr));
    line["statusCode"] = statusCode;
    line["reason"] = reason;
    line["payload"] = payload;

    std::cout << "[RemoteSync] Failed to send submission (status " << statusCode << ", " << reason
              << "). Payload logged to " << failedSubmissionsPath.string() << std::endl;
    std::cout << "[RemoteSync] Failed payload: " << payload.dump() << std::endl;

    std::ofstream out(failedSubmissionsPath, std::ios::app);
    if (out.good()) {
        out << line.dump() << "\n";
    }
}

RemoteSyncManager::ConnectionState RemoteSyncManager::GetConnectionState() const {
    return connectionState.load();
}

std::string RemoteSyncManager::GetClientVersion() const {
    return std::string(kClientVersion);
}

bool RemoteSyncManager::IsVersionPolicyChecked() const {
    return versionPolicyChecked.load();
}

bool RemoteSyncManager::HasUpdateAvailable() const {
    return updateAvailable.load();
}

bool RemoteSyncManager::IsUpdateRequired() const {
    return updateRequired.load();
}

std::string RemoteSyncManager::GetVersionPolicyMessage() const {
    std::lock_guard<std::mutex> lock(syncMutex);
    return versionPolicyMessage;
}

std::string RemoteSyncManager::GetUpdateDownloadUrl() const {
    std::lock_guard<std::mutex> lock(syncMutex);
    return updateDownloadUrl;
}

std::string RemoteSyncManager::GetConnectionStatusText() const {
    switch (connectionState.load()) {
        case ConnectionState::Connecting:
            return "Sync: Connecting...";
        case ConnectionState::Connected:
            return HasDirtyData() ? "Sync: Connected (pending upload)" : "Sync: Connected";
        case ConnectionState::Error:
            return "Sync: Connection failed (server may still be starting)";
        case ConnectionState::Disabled:
            return "Sync: Unavailable";
        case ConnectionState::NotConfigured:
        default:
            return "Sync: Not configured";
    }
}

bool RemoteSyncManager::RefreshLeaderboardNow() {
    if (config.serverIp.empty() || config.username.empty()) {
        return false;
    }

    std::lock_guard<std::mutex> lock(syncMutex);
    if (!EnsureAuthToken()) {
        return false;
    }
    return PullGlobalLeaderboardFromServer();
}

bool RemoteSyncManager::RefreshLeaderboardForPlayerNow(const std::string& username) {
    if (username.empty()) {
        return RefreshLeaderboardNow();
    }
    if (config.serverIp.empty() || config.username.empty()) {
        return false;
    }

    std::lock_guard<std::mutex> lock(syncMutex);
    if (!EnsureAuthToken()) {
        return false;
    }
    return PullGlobalLeaderboardFromServer(username);
}

bool RemoteSyncManager::HasDirtyData() const {
    return leaderboardDirty || statisticsDirty;
}

bool RemoteSyncManager::PerformSync() {
    lastSyncAttempt = std::chrono::steady_clock::now();

    if (config.serverIp.empty() || config.username.empty()) {
        if (!warnedMissingCredentials) {
            std::cout << "[RemoteSync] Missing server IP or username; skipping sync." << std::endl;
            warnedMissingCredentials = true;
        }
        return false;
    }

    if (!remoteUrlValid) {
        remoteUrlValid = ParseRemoteUrl();
        if (!remoteUrlValid) {
            std::cout << "[RemoteSync] Invalid remote URL configuration." << std::endl;
            return false;
        }
    }

#ifndef _WIN32
    if (!curlAvailable) {
        if (!warnedMissingCredentials) {
            std::cout << "[RemoteSync] libcurl unavailable; remote sync disabled." << std::endl;
            warnedMissingCredentials = true;
        }
        return false;
    }
#endif

    if (!EnsureAuthToken()) {
        return false;
    }

    bool success = true;

    if (leaderboardDirty) {
        const std::string pullFilter = requestedLeaderboardUsernameFilter;
        requestedLeaderboardUsernameFilter.clear();
        bool pushed = PushLocalLeaderboardsToServer();
        bool pulled = PullGlobalLeaderboardFromServer(pullFilter);
        if (pushed && pulled) {
            leaderboardDirty = false;
        } else {
            success = false;
        }
    }

    if (statisticsDirty) {
        bool pushed = PushMyStatsToServer();
        bool fetched = FetchMyStatsFromServer();
        if (pushed && fetched) {
            statisticsDirty = false;
        } else {
            success = false;
        }
    }

    return success;
}

bool RemoteSyncManager::EnsureAuthToken() {
    HttpResponse meResponse;
#ifdef _WIN32
    if (SendRequest(L"GET", BuildPath("api/auth/me"), "", meResponse) && meResponse.statusCode == 200) {
        return true;
    }
#else
    if (SendCurlRequest("GET", BuildUrl("api/auth/me"), "", meResponse) && meResponse.statusCode == 200) {
        return true;
    }
#endif

    nlohmann::json authPayload;
    authPayload["username"] = config.username;

    HttpResponse loginResponse;
#ifdef _WIN32
    bool loginSent = SendRequest(L"POST", BuildPath("api/auth/login"), authPayload.dump(), loginResponse);
#else
    bool loginSent = SendCurlRequest("POST", BuildUrl("api/auth/login"), authPayload.dump(), loginResponse);
#endif

    if (!loginSent || loginResponse.statusCode == 401) {
        HttpResponse registerResponse;
#ifdef _WIN32
        bool registerSent = SendRequest(L"POST", BuildPath("api/auth/register"), authPayload.dump(), registerResponse);
#else
        bool registerSent = SendCurlRequest("POST", BuildUrl("api/auth/register"), authPayload.dump(), registerResponse);
#endif
        if (!registerSent || (registerResponse.statusCode != 200 && registerResponse.statusCode != 201 && registerResponse.statusCode != 409)) {
            std::cout << "[RemoteSync] Failed to register/login user on server." << std::endl;
            return false;
        }

#ifdef _WIN32
        loginSent = SendRequest(L"POST", BuildPath("api/auth/login"), authPayload.dump(), loginResponse);
#else
        loginSent = SendCurlRequest("POST", BuildUrl("api/auth/login"), authPayload.dump(), loginResponse);
#endif
    }

    if (!loginSent || loginResponse.statusCode != 200) {
        std::cout << "[RemoteSync] Login failed (status " << loginResponse.statusCode << ")." << std::endl;
        return false;
    }

    try {
        nlohmann::json tokenJson = nlohmann::json::parse(loginResponse.body);
        authToken = tokenJson.value("access_token", "");
        if (authToken.empty()) {
            std::cout << "[RemoteSync] Login response missing access token." << std::endl;
            return false;
        }
        return true;
    } catch (const std::exception&) {
        std::cout << "[RemoteSync] Failed to parse login response." << std::endl;
        return false;
    }
}

bool RemoteSyncManager::PushLocalLeaderboardsToServer() {
    if (pendingLeaderboardSubmissions.empty()) {
        return true;
    }

    std::vector<LeaderboardEntry> entriesToSubmit = pendingLeaderboardSubmissions;
    std::vector<LeaderboardEntry> remainingEntries;
    remainingEntries.reserve(entriesToSubmit.size());
    bool overallSuccess = true;

    for (const auto& entry : entriesToSubmit) {
        nlohmann::json payload;
        payload["run_id"] = BuildRunId(entry);
        payload["difficulty"] = entry.difficulty;
        payload["completion_time"] = entry.completionTime;
        payload["used_auto_candidates"] = entry.usedAutoCandidates;
        payload["used_auto_check"] = entry.usedAutoCheck;
        payload["used_conflict_highlight"] = entry.usedConflictHighlight;
        payload["initial_board"] = entry.initialBoard;
        payload["solution_board"] = entry.solutionBoard;

        nlohmann::json movesArray = nlohmann::json::array();
        for (const auto& move : entry.moves) {
            movesArray.push_back(MoveToJson(move));
        }
        payload["moves"] = movesArray;
        payload["moves_json"] = movesArray.dump();
        payload["started_at"] = TimeToIso8601(entry.startedAt);
        payload["completed_at"] = TimeToIso8601(entry.completedAt);

        HttpResponse response;
#ifdef _WIN32
        bool sent = SendRequest(L"POST", BuildPath("api/scores/submit"), payload.dump(), response);
#else
        bool sent = SendCurlRequest("POST", BuildUrl("api/scores/submit"), payload.dump(), response);
#endif

        if (!sent) {
            LogFailedSubmission(payload, 0, "network_error");
            remainingEntries.push_back(entry);
            overallSuccess = false;
            continue;
        }

        if (response.statusCode == 409) {
            continue;
        }

        if (response.statusCode != 200 && response.statusCode != 201) {
            LogFailedSubmission(payload, response.statusCode, "http_error");
            remainingEntries.push_back(entry);
            overallSuccess = false;
        }
    }

    pendingLeaderboardSubmissions = std::move(remainingEntries);

    return overallSuccess;
}

bool RemoteSyncManager::PullGlobalLeaderboardFromServer(const std::string& usernameFilter) {
    HttpResponse response;
    std::string query = "api/scores/leaderboard/global/full?limit=5000";
    if (!usernameFilter.empty()) {
        query += "&username=" + UrlEncodeComponent(usernameFilter);
    }
#ifdef _WIN32
    if (!SendRequest(L"GET", BuildPath(query), "", response)) {
#else
    if (!SendCurlRequest("GET", BuildUrl(query), "", response)) {
#endif
        std::cout << "[RemoteSync] Failed to fetch global leaderboard from server." << std::endl;
        return false;
    }

    if (response.statusCode != 200) {
        std::cout << "[RemoteSync] Server returned " << response.statusCode << " when fetching leaderboard." << std::endl;
        return false;
    }

    if (!GameData::leaderboardManager) {
        return true;
    }

    try {
        auto data = nlohmann::json::parse(response.body);
        if (!data.is_array()) {
            return false;
        }

        std::vector<LeaderboardEntry> synced;
        synced.reserve(data.size());

        for (const auto& item : data) {
            LeaderboardEntry entry;
            entry.playerName = item.value("username", std::string("Player"));
            entry.completionTime = item.value("completion_time", 0.0);
            entry.initialBoard = item.value("initial_board", std::string());
            entry.solutionBoard = item.value("solution_board", std::string());
            entry.difficulty = item.value("difficulty", 0);
            entry.usedAutoCandidates = item.value("used_auto_candidates", false);
            entry.usedAutoCheck = item.value("used_auto_check", false);
            entry.usedConflictHighlight = item.value("used_conflict_highlight", false);
            entry.startedAt = ParseIso8601(item.value("started_at", std::string()));
            entry.completedAt = ParseIso8601(item.value("completed_at", std::string()));

            if (item.contains("moves") && item["moves"].is_array()) {
                for (const auto& moveItem : item["moves"]) {
                    entry.moves.push_back(JsonToMove(moveItem));
                }
            } else {
                std::string movesJsonText = item.value("moves_json", std::string("[]"));
                try {
                    auto movesJson = nlohmann::json::parse(movesJsonText);
                    if (movesJson.is_array()) {
                        for (const auto& moveItem : movesJson) {
                            entry.moves.push_back(JsonToMove(moveItem));
                        }
                    }
                } catch (const std::exception&) {
                }
            }

            if (entry.initialBoard.size() == 81 && entry.solutionBoard.size() == 81 && entry.completionTime > 0.0) {
                synced.push_back(std::move(entry));
            }
        }

        GameData::leaderboardManager->ReplaceAllEntries(synced);
        return true;
    } catch (const std::exception& e) {
        std::cout << "[RemoteSync] Failed to parse global leaderboard response: " << e.what() << std::endl;
        return false;
    }
}

bool RemoteSyncManager::FetchClientPolicyFromServer() {
    HttpResponse response;
    std::string query = std::string("api/meta/client-policy?client_version=") + UrlEncodeComponent(kClientVersion);
#ifdef _WIN32
    bool sent = SendRequest(L"GET", BuildPath(query), "", response);
#else
    bool sent = SendCurlRequest("GET", BuildUrl(query), "", response);
#endif

    if (!sent || response.statusCode != 200) {
        return false;
    }

    try {
        auto data = nlohmann::json::parse(response.body);
        if (!data.is_object()) {
            return false;
        }

        latestServerVersion = data.value("latest_version", std::string(""));
        minimumSupportedVersion = data.value("minimum_supported_version", std::string(""));
        updateAvailable = data.value("update_available", false);
        updateRequired = data.value("update_required", false);
        versionPolicyMessage = data.value("message", std::string(""));
        updateDownloadUrl = data.value("download_url", std::string(""));
        versionPolicyChecked = true;
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

bool RemoteSyncManager::PushMyStatsToServer() {
    if (!GameData::statisticsManager) {
        return true;
    }

    nlohmann::json payload;
    payload["stats_json"] = GameData::statisticsManager->ExportJson();

    HttpResponse response;
#ifdef _WIN32
    bool sent = SendRequest(L"PUT", BuildPath("api/scores/stats/full/me"), payload.dump(), response);
#else
    bool sent = SendCurlRequest("PUT", BuildUrl("api/scores/stats/full/me"), payload.dump(), response);
#endif

    if (!sent) {
        return false;
    }

    return response.statusCode == 200;
}

bool RemoteSyncManager::FetchMyStatsFromServer() {
    HttpResponse response;
#ifdef _WIN32
    bool sent = SendRequest(L"GET", BuildPath("api/scores/stats/full/me"), "", response);
#else
    bool sent = SendCurlRequest("GET", BuildUrl("api/scores/stats/full/me"), "", response);
#endif

    if (!sent) {
        return false;
    }

    if (response.statusCode == 404) {
        // Backward compatibility with older servers that only support summary stats.
#ifdef _WIN32
        sent = SendRequest(L"GET", BuildPath("api/scores/stats/me"), "", response);
#else
        sent = SendCurlRequest("GET", BuildUrl("api/scores/stats/me"), "", response);
#endif
        if (!sent) {
            return false;
        }
        return response.statusCode == 200;
    }

    if (response.statusCode != 200) {
        return false;
    }

    if (!GameData::statisticsManager) {
        return true;
    }

    try {
        auto data = nlohmann::json::parse(response.body);
        if (!data.contains("stats_json") || !data["stats_json"].is_object()) {
            return false;
        }
        return GameData::statisticsManager->ImportJson(data["stats_json"]);
    } catch (const std::exception&) {
        return false;
    }
}

std::string RemoteSyncManager::BuildRunId(const LeaderboardEntry& entry) const {
    std::ostringstream key;
    key << entry.playerName << '|'
        << entry.difficulty << '|'
        << std::fixed << std::setprecision(6) << entry.completionTime << '|'
        << entry.startedAt << '|'
        << entry.completedAt << '|'
        << entry.initialBoard << '|'
        << entry.solutionBoard;

    const std::string keyStr = key.str();
    const size_t hash = std::hash<std::string>{}(keyStr);

    std::ostringstream runId;
    runId << "rls-" << std::hex << hash;
    return runId.str();
}

std::string RemoteSyncManager::TimeToIso8601(std::time_t value) {
    std::tm tmValue{};
#ifdef _WIN32
    gmtime_s(&tmValue, &value);
#else
    gmtime_r(&value, &tmValue);
#endif

    std::ostringstream out;
    out << std::put_time(&tmValue, "%Y-%m-%dT%H:%M:%SZ");
    return out.str();
}

std::time_t RemoteSyncManager::ParseIso8601(const std::string& value) {
    if (value.empty()) {
        return 0;
    }

    std::tm tmValue{};
    std::istringstream stream(value.substr(0, 19));
    stream >> std::get_time(&tmValue, "%Y-%m-%dT%H:%M:%S");
    if (stream.fail()) {
        return 0;
    }

#ifdef _WIN32
    return _mkgmtime(&tmValue);
#else
    return timegm(&tmValue);
#endif
}

#ifdef _WIN32

bool RemoteSyncManager::ParseRemoteUrl() {
    apiHost.clear();
    apiBasePath.clear();
    apiPort = INTERNET_DEFAULT_HTTPS_PORT;
    useHttps = true;

    const std::string remoteUrl = BuildRemoteUrl(config.serverIp, config.serverPort, config.useHttps);

    const size_t schemePos = remoteUrl.find("://");
    if (schemePos == std::string::npos) {
        return false;
    }

    const std::string scheme = ToLowerCopy(remoteUrl.substr(0, schemePos));
    useHttps = (scheme == "https");
    if (!(scheme == "https" || scheme == "http")) {
        return false;
    }

    size_t hostStart = schemePos + 3;
    size_t slashPos = remoteUrl.find('/', hostStart);
    std::string hostPort = (slashPos == std::string::npos)
        ? remoteUrl.substr(hostStart)
        : remoteUrl.substr(hostStart, slashPos - hostStart);

    std::string path = (slashPos == std::string::npos)
        ? std::string("/")
        : remoteUrl.substr(slashPos);

    if (hostPort.empty()) {
        return false;
    }

    std::string host = hostPort;
    std::string portText;
    const size_t colonPos = hostPort.rfind(':');
    if (colonPos != std::string::npos) {
        host = hostPort.substr(0, colonPos);
        portText = hostPort.substr(colonPos + 1);
    }

    if (host.empty()) {
        return false;
    }

    if (!portText.empty()) {
        try {
            int parsedPort = std::stoi(portText);
            if (parsedPort <= 0 || parsedPort > 65535) {
                return false;
            }
            apiPort = static_cast<INTERNET_PORT>(parsedPort);
        } catch (const std::exception&) {
            return false;
        }
    } else {
        apiPort = useHttps ? INTERNET_DEFAULT_HTTPS_PORT : INTERNET_DEFAULT_HTTP_PORT;
    }

    apiHost = ToWide(host);
    apiBasePath = ToWide(path);
    if (apiBasePath.empty()) {
        apiBasePath = L"/";
    }

    return true;
}

std::wstring RemoteSyncManager::BuildPath(const std::string& relative) const {
    std::wstring base = apiBasePath;
    if (base.empty()) {
        base = L"/";
    }

    if (!base.empty() && base.back() == L'/' && !relative.empty() && relative.front() == '/') {
        std::wstring rel(relative.begin(), relative.end());
        base.pop_back();
        return base + rel;
    }

    if (!base.empty() && base.back() != L'/') {
        base.push_back(L'/');
    }

    if (relative.empty()) {
        return base;
    }

    std::wstring rel(relative.begin(), relative.end());
    if (!rel.empty() && rel.front() == L'/') {
        return base + rel.substr(1);
    }
    return base + rel;
}

bool RemoteSyncManager::SendRequest(const std::wstring& method, const std::wstring& path, const std::string& body, HttpResponse& response) {
    response = {};

    HINTERNET session = WinHttpOpen(L"RLSudoku/1.0", WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    if (!session) {
        return false;
    }

    HINTERNET connection = WinHttpConnect(session, apiHost.c_str(), apiPort, 0);
    if (!connection) {
        WinHttpCloseHandle(session);
        return false;
    }

    DWORD requestFlags = useHttps ? WINHTTP_FLAG_SECURE : 0;
    HINTERNET request = WinHttpOpenRequest(connection, method.c_str(), path.c_str(), nullptr, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, requestFlags);
    if (!request) {
        WinHttpCloseHandle(connection);
        WinHttpCloseHandle(session);
        return false;
    }

    if (useHttps) {
        DWORD secureProtocols = WINHTTP_FLAG_SECURE_PROTOCOL_TLS1 | WINHTTP_FLAG_SECURE_PROTOCOL_TLS1_1 | WINHTTP_FLAG_SECURE_PROTOCOL_TLS1_2;
        WinHttpSetOption(request, WINHTTP_OPTION_SECURE_PROTOCOLS, &secureProtocols, sizeof(secureProtocols));
    }

    WinHttpSetTimeouts(request,
                       kHttpResolveTimeoutMs,
                       kHttpConnectTimeoutMs,
                       kHttpSendTimeoutMs,
                       kHttpReceiveTimeoutMs);

    lastOutboundPacket = std::chrono::steady_clock::now();

    std::wstring headers = L"User-Agent: RLSudoku/" + ToWide(kClientVersion) + L"\r\nAccept: application/json\r\n";
    headers += L"X-Client-Version: " + ToWide(kClientVersion) + L"\r\n";
    if (!authToken.empty()) {
        headers += L"Authorization: Bearer " + ToWide(authToken) + L"\r\n";
    }
    if (method == L"POST" || method == L"PUT") {
        headers += L"Content-Type: application/json\r\n";
    }

    const void* optionalData = body.empty() ? WINHTTP_NO_REQUEST_DATA : static_cast<const void*>(body.data());
    DWORD optionalLength = static_cast<DWORD>(body.size());
    bool requestSent = WinHttpSendRequest(request, headers.c_str(), -1L, const_cast<void*>(optionalData), optionalLength, optionalLength, 0);
    if (!requestSent) {
        WinHttpCloseHandle(request);
        WinHttpCloseHandle(connection);
        WinHttpCloseHandle(session);
        return false;
    }

    if (!WinHttpReceiveResponse(request, nullptr)) {
        WinHttpCloseHandle(request);
        WinHttpCloseHandle(connection);
        WinHttpCloseHandle(session);
        return false;
    }

    DWORD statusCode = 0;
    DWORD statusCodeSize = sizeof(statusCode);
    WinHttpQueryHeaders(request, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER, WINHTTP_HEADER_NAME_BY_INDEX, &statusCode, &statusCodeSize, WINHTTP_NO_HEADER_INDEX);
    response.statusCode = static_cast<int>(statusCode);

    std::string buffer;
    DWORD bytesAvailable = 0;
    do {
        if (!WinHttpQueryDataAvailable(request, &bytesAvailable)) {
            break;
        }
        if (bytesAvailable == 0) {
            break;
        }

        std::string chunk;
        chunk.resize(bytesAvailable);
        DWORD bytesRead = 0;
        if (!WinHttpReadData(request, chunk.data(), bytesAvailable, &bytesRead)) {
            break;
        }
        chunk.resize(bytesRead);
        buffer.append(chunk);
    } while (bytesAvailable > 0);

    response.body = std::move(buffer);

    WinHttpCloseHandle(request);
    WinHttpCloseHandle(connection);
    WinHttpCloseHandle(session);
    return true;
}

#else

bool RemoteSyncManager::ParseRemoteUrl() {
    apiBaseUrl.clear();
    const std::string remoteUrl = BuildRemoteUrl(config.serverIp, config.serverPort, config.useHttps);

    const std::string lowered = ToLowerCopy(remoteUrl);
    if (!(lowered.rfind("https://", 0) == 0 || lowered.rfind("http://", 0) == 0)) {
        return false;
    }

    apiBaseUrl = remoteUrl;
    if (!apiBaseUrl.empty() && apiBaseUrl.back() == '/') {
        apiBaseUrl.pop_back();
    }
    return true;
}

std::string RemoteSyncManager::BuildUrl(const std::string& relative) const {
    std::string base = apiBaseUrl;
    if (base.empty()) {
        return relative;
    }

    if (!base.empty() && base.back() == '/' && !relative.empty() && relative.front() == '/') {
        return base + relative.substr(1);
    }

    if (!base.empty() && base.back() != '/' && !relative.empty() && relative.front() != '/') {
        return base + '/' + relative;
    }

    return base + relative;
}

bool RemoteSyncManager::SendCurlRequest(const std::string& method, const std::string& url, const std::string& body, HttpResponse& response) {
    response = {};
    if (!curlAvailable) {
        return false;
    }

    CURL* curl = curl_easy_init();
    if (!curl) {
        return false;
    }

    std::string buffer;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, static_cast<long>(kHttpConnectTimeoutMs));
    curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, static_cast<long>(kHttpConnectTimeoutMs + kHttpSendTimeoutMs + kHttpReceiveTimeoutMs));
    std::string userAgent = std::string("RLSudoku/") + kClientVersion;
    curl_easy_setopt(curl, CURLOPT_USERAGENT, userAgent.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, +[](char* ptr, size_t size, size_t nmemb, void* userdata) -> size_t {
        auto* out = static_cast<std::string*>(userdata);
        out->append(ptr, size * nmemb);
        return size * nmemb;
    });
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Accept: application/json");
    std::string versionHeader = std::string("X-Client-Version: ") + kClientVersion;
    headers = curl_slist_append(headers, versionHeader.c_str());
    if (!authToken.empty()) {
        std::string auth = "Authorization: Bearer " + authToken;
        headers = curl_slist_append(headers, auth.c_str());
    }

    if (method == "POST" || method == "PUT") {
        headers = curl_slist_append(headers, "Content-Type: application/json");
    }

    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, method.c_str());
    if (!body.empty()) {
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, static_cast<long>(body.size()));
    }

    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    lastOutboundPacket = std::chrono::steady_clock::now();
    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        return false;
    }

    long status = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status);
    response.statusCode = static_cast<int>(status);
    response.body = std::move(buffer);

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    return true;
}

#endif // _WIN32
