// RemoteSyncManager handles GitHub leaderboard synchronization configuration and scheduling.
// The actual network upload will be implemented separately; this class tracks state and timing.

#ifndef RLSUDOKU_REMOTESYNCMANAGER_H
#define RLSUDOKU_REMOTESYNCMANAGER_H

#include "pch.hxx"

struct RemoteSyncConfig {
    std::string remoteUrl;
    std::string branch;
    std::string username;
    std::string token;
    bool autoSyncOnSubmit = false;
    bool periodicSyncEnabled = false;
    double periodicIntervalSeconds = 300.0;
    bool syncOnExit = true;
};

class RemoteSyncManager {
public:
    RemoteSyncManager();
    ~RemoteSyncManager() = default;

    void LoadConfig();
    void SaveConfig() const;

    const RemoteSyncConfig& GetConfig() const;
    void UpdateConfig(const RemoteSyncConfig& newConfig);

    void QueueLeaderboardUpdate();
    void QueueStatisticsUpdate();

    void Update();
    void ForceSync();
    void OnExit();
    void PerformInitialPull();

private:
    bool HasDirtyData() const;
    bool ShouldAttemptPeriodicSync(std::chrono::steady_clock::time_point now) const;
    bool PerformSync();
    bool ParseRemoteUrl();

    struct HttpResponse {
        int statusCode = 0;
        std::string body;
    };

    struct GitHubFileInfo {
        bool requestSucceeded = false;
        bool exists = false;
        std::string sha;
        std::string error;
    };

    GitHubFileInfo FetchFileInfo(const std::string& fileName);
    bool UploadFile(const std::string& fileName, const std::filesystem::path& localPath);
    bool DownloadFile(const std::string& fileName, const std::filesystem::path& localPath);

#ifdef _WIN32
    std::wstring BuildPath(const std::string& relative) const;
    bool SendRequest(const std::wstring& method, const std::wstring& path, const std::string& body, HttpResponse& response);

    std::wstring apiHost;
    std::wstring apiBasePath;
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
    bool initialPullPerformed = false;
    bool remoteUrlValid = false;
};

#endif // RLSUDOKU_REMOTESYNCMANAGER_H
