#include "RemoteSyncManager.h"
#include "GameData.h"
#include "Storage/LeaderboardManager.h"
#include "Storage/StatisticsManager.h"

#include <sstream>
#include <iomanip>
#include <ctime>
#include <iterator>
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

RemoteSyncConfig GetDefaultConfig() {
    RemoteSyncConfig cfg;
    cfg.remoteUrl.clear();
    cfg.branch = "main";
    cfg.username.clear();
    cfg.token.clear();
    cfg.autoSyncOnSubmit = false;
    cfg.periodicSyncEnabled = true;
    cfg.periodicIntervalSeconds = 300.0;
    cfg.syncOnExit = true;
    return cfg;
}

std::string Base64Encode(const std::vector<uint8_t>& data) {
    static constexpr char table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string encoded;
    encoded.reserve(((data.size() + 2) / 3) * 4);

    for (size_t i = 0; i < data.size(); i += 3) {
        uint32_t value = static_cast<uint32_t>(data[i]) << 16;
        if (i + 1 < data.size()) {
            value |= static_cast<uint32_t>(data[i + 1]) << 8;
        }
        if (i + 2 < data.size()) {
            value |= static_cast<uint32_t>(data[i + 2]);
        }

        encoded.push_back(table[(value >> 18) & 0x3F]);
        encoded.push_back(table[(value >> 12) & 0x3F]);
        if (i + 1 < data.size()) {
            encoded.push_back(table[(value >> 6) & 0x3F]);
        } else {
            encoded.push_back('=');
        }
        if (i + 2 < data.size()) {
            encoded.push_back(table[value & 0x3F]);
        } else {
            encoded.push_back('=');
        }
    }

    return encoded;
}

#ifdef _WIN32
std::wstring ToWide(const std::string& value) {
    return std::wstring(value.begin(), value.end());
}
#endif

std::vector<uint8_t> Base64Decode(const std::string& input) {
    static constexpr char table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    static int lookup[256];
    static bool initialized = false;
    if (!initialized) {
        std::fill(std::begin(lookup), std::end(lookup), -1);
        for (int i = 0; i < 64; ++i) {
            lookup[static_cast<unsigned char>(table[i])] = i;
        }
        initialized = true;
    }

    std::vector<uint8_t> output;
    int val = 0;
    int valb = -8;
    for (char c : input) {
        unsigned char uc = static_cast<unsigned char>(c);
        if (uc == '\r' || uc == '\n') {
            continue;
        }
        if (c == '=') {
            break;
        }
        int decoded = lookup[uc];
        if (decoded < 0) {
            continue;
        }
        val = (val << 6) | decoded;
        valb += 6;
        if (valb >= 0) {
            output.push_back(static_cast<uint8_t>((val >> valb) & 0xFF));
            valb -= 8;
        }
    }
    return output;
}

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

}

RemoteSyncManager::RemoteSyncManager() {
    configPath = std::filesystem::path("./remote_sync.json");
    config = GetDefaultConfig();
    LoadConfig();
    auto now = std::chrono::steady_clock::now();
    lastSyncAttempt = now;
    lastSuccessfulSync = now;
#ifndef _WIN32
    curlAvailable = EnsureCurlInitialized();
#endif
    PerformInitialPull();
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
        config.remoteUrl = data.value("remoteUrl", "");
        config.branch = data.value("branch", std::string("main"));
        config.username = data.value("username", "");
        config.token = data.value("token", "");
        config.autoSyncOnSubmit = data.value("autoSyncOnSubmit", false);
        config.periodicSyncEnabled = data.value("periodicSyncEnabled", true);
        config.periodicIntervalSeconds = data.value("periodicIntervalSeconds", 300.0);
        config.syncOnExit = data.value("syncOnExit", true);
    } catch (const std::exception&) {
        config = GetDefaultConfig();
    }

    remoteUrlValid = ParseRemoteUrl();
}

void RemoteSyncManager::SaveConfig() const {
    nlohmann::json data;
    data["remoteUrl"] = config.remoteUrl;
    data["branch"] = config.branch;
    data["username"] = config.username;
    data["token"] = config.token;
    data["autoSyncOnSubmit"] = config.autoSyncOnSubmit;
    data["periodicSyncEnabled"] = config.periodicSyncEnabled;
    data["periodicIntervalSeconds"] = config.periodicIntervalSeconds;
    data["syncOnExit"] = config.syncOnExit;

    std::ofstream output(configPath);
    output << data.dump(4);
}

const RemoteSyncConfig& RemoteSyncManager::GetConfig() const {
    return config;
}

void RemoteSyncManager::UpdateConfig(const RemoteSyncConfig& newConfig) {
    config = newConfig;
    warnedMissingCredentials = false;
    SaveConfig();
    remoteUrlValid = ParseRemoteUrl();
    initialPullPerformed = false;
    PerformInitialPull();
}

void RemoteSyncManager::QueueLeaderboardUpdate() {
    leaderboardDirty = true;
    if (config.autoSyncOnSubmit) {
        ForceSync();
    }
}

void RemoteSyncManager::QueueStatisticsUpdate() {
    statisticsDirty = true;
    if (config.autoSyncOnSubmit) {
        ForceSync();
    }
}

void RemoteSyncManager::Update() {
    auto now = std::chrono::steady_clock::now();
    if (!HasDirtyData()) {
        return;
    }

    if (!config.periodicSyncEnabled) {
        return;
    }

    if (ShouldAttemptPeriodicSync(now)) {
        ForceSync();
    }
}

void RemoteSyncManager::ForceSync() {
    if (!HasDirtyData()) {
        return;
    }

    if (PerformSync()) {
        lastSuccessfulSync = std::chrono::steady_clock::now();
    }
}

void RemoteSyncManager::PerformInitialPull() {
    if (initialPullPerformed) {
        return;
    }

    if (config.remoteUrl.empty() || config.branch.empty() || config.token.empty()) {
        if (!warnedMissingCredentials) {
            std::cout << "[RemoteSync] Remote configuration incomplete; skipping initial pull." << std::endl;
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

    initialPullPerformed = true;
    warnedMissingCredentials = false;
    bool pulledAny = false;
    if (DownloadFile("leaderboards.bin", std::filesystem::path("./leaderboards.bin"))) {
        pulledAny = true;
    }
    if (DownloadFile("stats.json", std::filesystem::path("./stats.json"))) {
        pulledAny = true;
    }

    if (pulledAny) {
        std::cout << "[RemoteSync] Initial data pulled from remote." << std::endl;
    }
}

void RemoteSyncManager::OnExit() {
    if (!config.syncOnExit) {
        return;
    }

    ForceSync();
}

bool RemoteSyncManager::HasDirtyData() const {
    return leaderboardDirty || statisticsDirty;
}

bool RemoteSyncManager::ShouldAttemptPeriodicSync(std::chrono::steady_clock::time_point now) const {
    if (!config.periodicSyncEnabled) {
        return false;
    }

    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - lastSyncAttempt);
    return elapsed.count() >= static_cast<int64_t>(std::max(1.0, config.periodicIntervalSeconds));
}

bool RemoteSyncManager::PerformSync() {
    lastSyncAttempt = std::chrono::steady_clock::now();

    if (config.remoteUrl.empty() || config.branch.empty() || config.token.empty()) {
        if (!warnedMissingCredentials) {
            std::cout << "[RemoteSync] Missing remote configuration; skipping sync." << std::endl;
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

    bool success = true;

    if (leaderboardDirty) {
        if (UploadFile("leaderboards.bin", std::filesystem::path("./leaderboards.bin"))) {
            leaderboardDirty = false;
        } else {
            success = false;
        }
    }

    if (statisticsDirty) {
        if (UploadFile("stats.json", std::filesystem::path("./stats.json"))) {
            statisticsDirty = false;
        } else {
            success = false;
        }
    }

    return success;
}

#ifdef _WIN32

bool RemoteSyncManager::ParseRemoteUrl() {
    apiHost.clear();
    apiBasePath.clear();

    if (config.remoteUrl.empty()) {
        return false;
    }

    const std::string prefix = "https://";
    if (config.remoteUrl.rfind(prefix, 0) != 0) {
        return false;
    }

    size_t hostStart = prefix.size();
    size_t slashPos = config.remoteUrl.find('/', hostStart);

    std::string host = (slashPos == std::string::npos)
        ? config.remoteUrl.substr(hostStart)
        : config.remoteUrl.substr(hostStart, slashPos - hostStart);

    if (host.empty()) {
        return false;
    }

    std::string path = (slashPos == std::string::npos)
        ? std::string("/")
        : config.remoteUrl.substr(slashPos);

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

    HINTERNET connection = WinHttpConnect(session, apiHost.c_str(), INTERNET_DEFAULT_HTTPS_PORT, 0);
    if (!connection) {
        WinHttpCloseHandle(session);
        return false;
    }

    HINTERNET request = WinHttpOpenRequest(connection, method.c_str(), path.c_str(), nullptr, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);
    if (!request) {
        WinHttpCloseHandle(connection);
        WinHttpCloseHandle(session);
        return false;
    }

    DWORD secureProtocols = WINHTTP_FLAG_SECURE_PROTOCOL_TLS1 | WINHTTP_FLAG_SECURE_PROTOCOL_TLS1_1 | WINHTTP_FLAG_SECURE_PROTOCOL_TLS1_2;
    WinHttpSetOption(request, WINHTTP_OPTION_SECURE_PROTOCOLS, &secureProtocols, sizeof(secureProtocols));

    std::wstring headers = L"User-Agent: RLSudoku/1.0\r\nAccept: application/vnd.github+json\r\n";
    if (!config.token.empty()) {
        headers += L"Authorization: Bearer " + ToWide(config.token) + L"\r\n";
    }
    if (method == L"PUT") {
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
    if (config.remoteUrl.empty()) {
        return false;
    }

    const std::string prefix = "https://";
    if (config.remoteUrl.rfind(prefix, 0) != 0) {
        return false;
    }

    apiBaseUrl = config.remoteUrl;
    if (!apiBaseUrl.empty() && apiBaseUrl.back() == '/') {
        apiBaseUrl.pop_back();
    }
    return true;
}

std::string RemoteSyncManager::BuildUrl(const std::string& relative) const {
    std::string base = apiBaseUrl.empty() ? config.remoteUrl : apiBaseUrl;
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
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "RLSudoku/1.0");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, +[](char* ptr, size_t size, size_t nmemb, void* userdata) -> size_t {
        auto* out = static_cast<std::string*>(userdata);
        out->append(ptr, size * nmemb);
        return size * nmemb;
    });
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Accept: application/vnd.github+json");
    if (!config.token.empty()) {
        std::string auth = "Authorization: Bearer " + config.token;
        headers = curl_slist_append(headers, auth.c_str());
    }

    if (method == "PUT") {
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, static_cast<long>(body.size()));
    } else {
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, method.c_str());
        if (!body.empty()) {
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
            curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, static_cast<long>(body.size()));
        }
    }

    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
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

RemoteSyncManager::GitHubFileInfo RemoteSyncManager::FetchFileInfo(const std::string& fileName) {
    GitHubFileInfo info;
    std::string relative = "contents/" + fileName + "?ref=" + config.branch;
    HttpResponse response;

#ifdef _WIN32
    if (!SendRequest(L"GET", BuildPath(relative), "", response)) {
#else
    if (!SendCurlRequest("GET", BuildUrl(relative), "", response)) {
#endif
        info.error = "Failed to contact GitHub API";
        return info;
    }

    info.requestSucceeded = true;

    if (response.statusCode == 200) {
        try {
            auto json = nlohmann::json::parse(response.body);
            info.exists = true;
            info.sha = json.value("sha", "");
        } catch (const std::exception& e) {
            info.error = std::string("Failed to parse GitHub response: ") + e.what();
            info.requestSucceeded = false;
        }
    } else if (response.statusCode == 404) {
        info.exists = false;
    } else {
        info.error = "GitHub responded with status " + std::to_string(response.statusCode);
        info.requestSucceeded = false;
    }

    return info;
}

bool RemoteSyncManager::UploadFile(const std::string& fileName, const std::filesystem::path& localPath) {
    if (!std::filesystem::exists(localPath)) {
        std::cout << "[RemoteSync] Local file missing: " << localPath.string() << std::endl;
        return false;
    }

    std::ifstream file(localPath, std::ios::binary);
    if (!file.good()) {
        std::cout << "[RemoteSync] Failed to read " << localPath.string() << std::endl;
        return false;
    }
    std::vector<uint8_t> data((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    GitHubFileInfo info = FetchFileInfo(fileName);
    if (!info.requestSucceeded) {
        if (!info.error.empty()) {
            std::cout << "[RemoteSync] " << info.error << std::endl;
        }
        return false;
    }

    std::string encoded = Base64Encode(data);

    std::ostringstream message;
    message << "Update " << fileName;
    if (!config.username.empty()) {
        message << " (" << config.username << ")";
    }
    std::time_t now = std::time(nullptr);
    std::tm* gmt = std::gmtime(&now);
    if (gmt) {
        message << " @ " << std::put_time(gmt, "%Y-%m-%d %H:%M:%SZ");
    }

    nlohmann::json payload;
    payload["message"] = message.str();
    payload["content"] = encoded;
    payload["branch"] = config.branch;

    nlohmann::json committer;
    std::string committerName = config.username.empty() ? std::string("RLSudoku") : config.username;
    committer["name"] = committerName;
    committer["email"] = committerName + "@example.com";
    payload["committer"] = committer;

    if (info.exists && !info.sha.empty()) {
        payload["sha"] = info.sha;
    }

    std::string relative = "contents/" + fileName;
    HttpResponse response;

#ifdef _WIN32
    if (!SendRequest(L"PUT", BuildPath(relative), payload.dump(), response)) {
#else
    if (!SendCurlRequest("PUT", BuildUrl(relative), payload.dump(), response)) {
#endif
        std::cout << "[RemoteSync] Failed to upload " << fileName << " to GitHub." << std::endl;
        return false;
    }

    if (response.statusCode != 200 && response.statusCode != 201) {
        std::cout << "[RemoteSync] GitHub rejected upload of " << fileName << " (status " << response.statusCode << ")" << std::endl;
        return false;
    }

    std::cout << "[RemoteSync] Uploaded " << fileName << " (" << data.size() << " bytes)." << std::endl;
    return true;
}

bool RemoteSyncManager::DownloadFile(const std::string& fileName, const std::filesystem::path& localPath) {
    std::string relative = "contents/" + fileName + "?ref=" + config.branch;
    HttpResponse response;

#ifdef _WIN32
    if (!SendRequest(L"GET", BuildPath(relative), "", response)) {
#else
    if (!SendCurlRequest("GET", BuildUrl(relative), "", response)) {
#endif
        std::cout << "[RemoteSync] Failed to contact GitHub for " << fileName << std::endl;
        return false;
    }

    if (response.statusCode == 404) {
        std::cout << "[RemoteSync] Remote file not found: " << fileName << std::endl;
        return false;
    }

    if (response.statusCode != 200) {
        std::cout << "[RemoteSync] GitHub responded " << response.statusCode << " for " << fileName << std::endl;
        return false;
    }

    try {
        auto json = nlohmann::json::parse(response.body);
        std::string encoding = json.value("encoding", "");
        if (encoding != "base64") {
            std::cout << "[RemoteSync] Unexpected encoding for " << fileName << std::endl;
            return false;
        }

        std::string content = json.value("content", "");
        auto decoded = Base64Decode(content);

        std::ofstream output(localPath, std::ios::binary);
        if (!output.good()) {
            std::cout << "[RemoteSync] Failed to write " << localPath.string() << std::endl;
            return false;
        }
        if (!decoded.empty()) {
            output.write(reinterpret_cast<const char*>(decoded.data()), static_cast<std::streamsize>(decoded.size()));
        } else {
            output.flush();
        }
        output.close();

        if (fileName == "leaderboards.bin") {
            if (GameData::leaderboardManager) {
                GameData::leaderboardManager->Load();
            }
        } else if (fileName == "stats.json") {
            if (GameData::statisticsManager) {
                GameData::statisticsManager->Load();
            }
        }

        std::cout << "[RemoteSync] Downloaded " << fileName << " (" << decoded.size() << " bytes)." << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cout << "[RemoteSync] Failed to parse GitHub response for " << fileName << ": " << e.what() << std::endl;
        return false;
    }
}
