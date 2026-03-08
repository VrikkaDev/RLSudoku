// RemoteSyncManager handles server-based leaderboard synchronization configuration and scheduling.

#ifndef RLSUDOKU_REMOTESYNCMANAGER_H
#define RLSUDOKU_REMOTESYNCMANAGER_H

#include "pch.hxx"

struct LeaderboardEntry;

struct RemoteSyncConfig {
    std::string serverIp;
    int serverPort = 8000;
    bool useHttps = false;
    std::string username;
};

class RemoteSyncManager {
public:
    enum class ConnectionState {
        NotConfigured,
        Connecting,
        Connected,
        Error,
        Disabled
    };

    RemoteSyncManager();
    ~RemoteSyncManager();

    void LoadConfig();
    void SaveConfig() const;

    const RemoteSyncConfig& GetConfig() const;
    void UpdateConfig(const RemoteSyncConfig& newConfig);

    void QueueLeaderboardUpdate();
    void QueueLeaderboardSubmission(const LeaderboardEntry& entry);
    void QueueStatisticsUpdate();

    void Update();
    void ForceSync();
    void OnExit();
    void PerformInitialPull();
    bool RefreshLeaderboardNow();
    bool RefreshLeaderboardForPlayerNow(const std::string& username);
    std::string GetConnectionStatusText() const;
    ConnectionState GetConnectionState() const;

private:
    void StartInitialPullAsync();
    void JoinInitialPullThread();
    void StartKeepAliveAsync();
    void JoinKeepAliveThread();
    void StartSyncAsync();
    void JoinSyncThread();
    void LogFailedSubmission(const nlohmann::json& payload, int statusCode, const std::string& reason) const;
    bool HasDirtyData() const;
    bool PerformSync();
    bool ParseRemoteUrl();
    bool EnsureAuthToken();
    bool PushLocalLeaderboardsToServer();
    bool PushMyStatsToServer();
    bool PullGlobalLeaderboardFromServer(const std::string& usernameFilter = std::string());
    bool FetchMyStatsFromServer();
    std::string BuildRunId(const LeaderboardEntry& entry) const;
    static std::string TimeToIso8601(std::time_t value);
    static std::time_t ParseIso8601(const std::string& value);

    struct HttpResponse {
        int statusCode = 0;
        std::string body;
    };

#ifdef _WIN32
    std::wstring BuildPath(const std::string& relative) const;
    bool SendRequest(const std::wstring& method, const std::wstring& path, const std::string& body, HttpResponse& response);

    std::wstring apiHost;
    std::wstring apiBasePath;
    uint16_t apiPort = 443;
    bool useHttps = true;
#else
    std::string BuildUrl(const std::string& relative) const;
    bool SendCurlRequest(const std::string& method, const std::string& url, const std::string& body, HttpResponse& response);

    std::string apiBaseUrl;
    bool curlAvailable = false;
#endif

    RemoteSyncConfig config{};
    std::filesystem::path configPath;
    bool leaderboardDirty = false;
    bool statisticsDirty = false;
    mutable bool warnedMissingCredentials = false;
    std::chrono::steady_clock::time_point lastSyncAttempt;
    std::chrono::steady_clock::time_point lastSuccessfulSync;
    std::chrono::steady_clock::time_point lastOutboundPacket;
    std::chrono::steady_clock::time_point lastReconnectAttempt;
    bool initialPullPerformed = false;
    bool remoteUrlValid = false;
    std::string authToken;
    std::vector<LeaderboardEntry> pendingLeaderboardSubmissions;

    std::thread initialPullThread;
    std::thread keepAliveThread;
    std::thread syncThread;
    mutable std::mutex syncMutex;
    std::atomic<ConnectionState> connectionState{ConnectionState::NotConfigured};
    std::atomic<bool> keepAliveInFlight{false};
    std::atomic<bool> syncInFlight{false};

    std::filesystem::path failedSubmissionsPath;
};

#endif // RLSUDOKU_REMOTESYNCMANAGER_H
