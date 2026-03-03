//
// Created for RLSudoku Leaderboard System
//

#ifndef RLSUDOKU_LEADERBOARDMANAGER_H
#define RLSUDOKU_LEADERBOARDMANAGER_H

#include "pch.hxx"
#include <vector>
#include <string>
#include <fstream>
#include <cstdint>

enum class MoveAction : uint8_t {
    Value = 0,
    Notes = 1,
    ManualCandidates = 2
};

struct MoveRecord {
    int tileNumber = 0;               // Which tile was modified
    int value = 0;                    // What value was placed (0 for erase)
    double timestamp = 0.0;           // Time from puzzle start in seconds
    uint16_t notesMask = 0;           // Bitmask for manual notes (1-9)
    uint16_t removedMask = 0;         // Bitmask for manually removed auto candidates
    bool autoCandidatesEnabled = false; // Whether auto candidates were active
    MoveAction action = MoveAction::Value; // What kind of action this record represents

    MoveRecord() = default;
    MoveRecord(int tile, int val, double time,
               uint16_t notes, uint16_t removed,
               bool autoMode, MoveAction act)
        : tileNumber(tile), value(val), timestamp(time),
          notesMask(notes), removedMask(removed),
          autoCandidatesEnabled(autoMode), action(act) {}
};

struct LeaderboardEntry {
    std::string playerName;
    double completionTime;      // Total time in seconds
    std::string initialBoard;   // Starting puzzle (81 chars)
    std::string solutionBoard;  // Solution for verification
    int difficulty;             // 0-100
    
    // Flags for assists used
    bool usedAutoCandidates;
    bool usedAutoCheck;
    bool usedConflictHighlight;
    
    // Replay data - compressed move history
    std::vector<MoveRecord> moves;
    
    // Timestamps
    std::time_t startedAt;      // When puzzle was started
    std::time_t completedAt;    // When puzzle was completed
    
    LeaderboardEntry() 
        : completionTime(0), difficulty(0), 
          usedAutoCandidates(false), usedAutoCheck(false), 
          usedConflictHighlight(false),
          startedAt(0), completedAt(0) {}
};

class LeaderboardManager {
public:
    // Maximum entries per category (difficulty + assist settings combination)
    static constexpr int MAX_ENTRIES_PER_CATEGORY = 20;
    
    LeaderboardManager();
    
    // Load/Save leaderboards from encrypted file
    void Load();
    void Save();
    
    // Add a new entry (will sort automatically)
    void AddEntry(const LeaderboardEntry& entry);

    const std::vector<LeaderboardEntry>& GetAllEntries() const { return entries; }
    void ReplaceAllEntries(const std::vector<LeaderboardEntry>& newEntries);
    
    // Get top entries for a difficulty range
    // filterAssisted: if true, exclude runs with auto-candidates/autocheck
    std::vector<LeaderboardEntry> GetTopEntries(
        int minDifficulty, 
        int maxDifficulty, 
        int count = 10,
        bool filterAssisted = false
    );
    
    // Get all entries for a specific difficulty
    std::vector<LeaderboardEntry> GetEntriesForDifficulty(
        int difficulty,
        bool filterAssisted = false
    );
    
    // Check if a time would make it to the leaderboard
    bool IsLeaderboardWorthy(double time, int difficulty, bool assisted);
    
private:
    std::vector<LeaderboardEntry> entries;
    std::string filename = "./leaderboards.bin";
    
    // Simple XOR encryption key (can be made more complex)
    const uint8_t encryptionKey = 0x5A;
    
    // Serialize entry to binary
    std::vector<uint8_t> SerializeEntry(const LeaderboardEntry& entry);
    LeaderboardEntry DeserializeEntry(const uint8_t* data, size_t dataSize, size_t& offset);
    
    // Encryption helpers
    void EncryptData(std::vector<uint8_t>& data);
    void DecryptData(std::vector<uint8_t>& data);
    
    // Compress move data
    std::vector<uint8_t> CompressMoves(const std::vector<MoveRecord>& moves);
    std::vector<MoveRecord> DecompressMoves(const uint8_t* data, size_t length);
};

#endif //RLSUDOKU_LEADERBOARDMANAGER_H
