//
// Created for RLSudoku Leaderboard System
//

#include "LeaderboardManager.h"
#include <algorithm>
#include <cstring>
#include <ctime>
#include <iostream>

LeaderboardManager::LeaderboardManager() {
    Load();
}

void LeaderboardManager::Load() {
    // Intentionally server-first: local file is backup-only and not loaded at startup.
    entries.clear();
}

void LeaderboardManager::Save() {
    std::vector<uint8_t> data;
    
    // Write number of entries (4 bytes)
    uint32_t count = static_cast<uint32_t>(entries.size());
    data.insert(data.end(), 
                reinterpret_cast<uint8_t*>(&count), 
                reinterpret_cast<uint8_t*>(&count) + 4);
    
    // Serialize each entry
    for (const auto& entry : entries) {
        auto entryData = SerializeEntry(entry);
        data.insert(data.end(), entryData.begin(), entryData.end());
    }
    
    // Encrypt
    EncryptData(data);
    
    // Write to file
    std::ofstream file(filename, std::ios::binary);
    file.write(reinterpret_cast<const char*>(data.data()), data.size());
    file.close();
}

void LeaderboardManager::AddEntry(const LeaderboardEntry& entry) {
    // Find all entries with same category (difficulty + assist settings)
    std::vector<size_t> sameCategoryIndices;
    for (size_t i = 0; i < entries.size(); ++i) {
        const auto& existing = entries[i];
        if (existing.difficulty == entry.difficulty &&
            existing.usedAutoCandidates == entry.usedAutoCandidates &&
            existing.usedAutoCheck == entry.usedAutoCheck &&
            existing.usedConflictHighlight == entry.usedConflictHighlight) {
            sameCategoryIndices.push_back(i);
        }
    }
    
    // If we have MAX_ENTRIES_PER_CATEGORY or more in this category
    if (sameCategoryIndices.size() >= MAX_ENTRIES_PER_CATEGORY) {
        // Find the worst (highest time) entry in this category
        size_t worstIdx = sameCategoryIndices[0];
        double worstTime = entries[worstIdx].completionTime;
        
        for (size_t idx : sameCategoryIndices) {
            if (entries[idx].completionTime > worstTime) {
                worstTime = entries[idx].completionTime;
                worstIdx = idx;
            }
        }
        
        // Only add new entry if it's better than the worst one
        if (entry.completionTime < worstTime) {
            // Remove the worst entry
            entries.erase(entries.begin() + worstIdx);
            // Add new entry
            entries.push_back(entry);
        }
        // If new entry is worse than all existing ones, don't add it
    } else {
        // Category has space, just add it
        entries.push_back(entry);
    }
    
    // Sort by difficulty first, then by time
    std::sort(entries.begin(), entries.end(), 
        [](const LeaderboardEntry& a, const LeaderboardEntry& b) {
            if (a.difficulty != b.difficulty) {
                return a.difficulty < b.difficulty;
            }
            return a.completionTime < b.completionTime;
        });
    
    Save();
}

void LeaderboardManager::ReplaceAllEntries(const std::vector<LeaderboardEntry>& newEntries) {
    entries = newEntries;

    std::sort(entries.begin(), entries.end(),
        [](const LeaderboardEntry& a, const LeaderboardEntry& b) {
            if (a.difficulty != b.difficulty) {
                return a.difficulty < b.difficulty;
            }
            return a.completionTime < b.completionTime;
        });

    Save();
}

std::vector<LeaderboardEntry> LeaderboardManager::GetTopEntries(
    int minDifficulty, 
    int maxDifficulty, 
    int count,
    bool filterAssisted
) {
    std::vector<LeaderboardEntry> filtered;
    
    for (const auto& entry : entries) {
        if (entry.difficulty >= minDifficulty && entry.difficulty <= maxDifficulty) {
            if (filterAssisted && (entry.usedAutoCandidates || entry.usedAutoCheck)) {
                continue;
            }
            filtered.push_back(entry);
        }
    }
    
    // Sort by time
    std::sort(filtered.begin(), filtered.end(),
        [](const LeaderboardEntry& a, const LeaderboardEntry& b) {
            return a.completionTime < b.completionTime;
        });
    
    if (filtered.size() > static_cast<size_t>(count)) {
        filtered.resize(count);
    }
    
    return filtered;
}

std::vector<LeaderboardEntry> LeaderboardManager::GetEntriesForDifficulty(
    int difficulty,
    bool filterAssisted
) {
    return GetTopEntries(difficulty, difficulty, 100, filterAssisted);
}

bool LeaderboardManager::IsLeaderboardWorthy(double time, int difficulty, bool assisted) {
    auto topEntries = GetTopEntries(difficulty, difficulty, 10, assisted);
    
    if (topEntries.size() < 10) {
        return true; // Always worthy if less than 10 entries
    }
    
    return time < topEntries.back().completionTime;
}

std::vector<uint8_t> LeaderboardManager::SerializeEntry(const LeaderboardEntry& entry) {
    std::vector<uint8_t> data;
    
    // Player name (length + string)
    uint16_t nameLen = static_cast<uint16_t>(entry.playerName.length());
    data.insert(data.end(), 
                reinterpret_cast<uint8_t*>(&nameLen), 
                reinterpret_cast<uint8_t*>(&nameLen) + 2);
    data.insert(data.end(), entry.playerName.begin(), entry.playerName.end());
    
    // Completion time (8 bytes double)
    data.insert(data.end(), 
                reinterpret_cast<const uint8_t*>(&entry.completionTime), 
                reinterpret_cast<const uint8_t*>(&entry.completionTime) + 8);
    
    // Initial board (81 chars)
    data.insert(data.end(), entry.initialBoard.begin(), entry.initialBoard.end());
    
    // Solution board (81 chars)
    data.insert(data.end(), entry.solutionBoard.begin(), entry.solutionBoard.end());
    
    // Difficulty (4 bytes)
    data.insert(data.end(), 
                reinterpret_cast<const uint8_t*>(&entry.difficulty), 
                reinterpret_cast<const uint8_t*>(&entry.difficulty) + 4);
    
    // Flags (1 byte packed)
    uint8_t flags = 0;
    if (entry.usedAutoCandidates) flags |= 0x01;
    if (entry.usedAutoCheck) flags |= 0x02;
    if (entry.usedConflictHighlight) flags |= 0x04;
    data.push_back(flags);
    
    // Timestamps (8 bytes each)
    data.insert(data.end(), 
                reinterpret_cast<const uint8_t*>(&entry.startedAt), 
                reinterpret_cast<const uint8_t*>(&entry.startedAt) + 8);
    data.insert(data.end(), 
                reinterpret_cast<const uint8_t*>(&entry.completedAt), 
                reinterpret_cast<const uint8_t*>(&entry.completedAt) + 8);
    
    // Moves (compressed)
    auto compressedMoves = CompressMoves(entry.moves);
    uint32_t movesLen = static_cast<uint32_t>(compressedMoves.size());
    data.insert(data.end(), 
                reinterpret_cast<uint8_t*>(&movesLen), 
                reinterpret_cast<uint8_t*>(&movesLen) + 4);
    data.insert(data.end(), compressedMoves.begin(), compressedMoves.end());
    
    return data;
}

LeaderboardEntry LeaderboardManager::DeserializeEntry(const uint8_t* data, size_t dataSize, size_t& offset) {
    LeaderboardEntry entry;
    
    try {
        // Player name (2 bytes length + string)
        if (offset + 2 > dataSize) throw std::runtime_error("Unexpected end of data (name length)");
        uint16_t nameLen;
        std::memcpy(&nameLen, data + offset, 2);
        offset += 2;
        
        if (nameLen > 1000) throw std::runtime_error("Name length too large");
        if (offset + nameLen > dataSize) throw std::runtime_error("Unexpected end of data (name)");
        entry.playerName.assign(reinterpret_cast<const char*>(data + offset), nameLen);
        offset += nameLen;
        
        // Completion time (8 bytes)
        if (offset + 8 > dataSize) throw std::runtime_error("Unexpected end of data (completion time)");
        std::memcpy(&entry.completionTime, data + offset, 8);
        offset += 8;
        
        // Boards (81 bytes each)
        if (offset + 81 > dataSize) throw std::runtime_error("Unexpected end of data (initial board)");
        entry.initialBoard.assign(reinterpret_cast<const char*>(data + offset), 81);
        offset += 81;
        if (offset + 81 > dataSize) throw std::runtime_error("Unexpected end of data (solution board)");
        entry.solutionBoard.assign(reinterpret_cast<const char*>(data + offset), 81);
        offset += 81;
        
        // Difficulty (4 bytes)
        if (offset + 4 > dataSize) throw std::runtime_error("Unexpected end of data (difficulty)");
        std::memcpy(&entry.difficulty, data + offset, 4);
        offset += 4;
        
        // Flags (1 byte)
        if (offset + 1 > dataSize) throw std::runtime_error("Unexpected end of data (flags)");
        uint8_t flags = data[offset++];
        entry.usedAutoCandidates = (flags & 0x01) != 0;
        entry.usedAutoCheck = (flags & 0x02) != 0;
        entry.usedConflictHighlight = (flags & 0x04) != 0;
        
        // Timestamps (8 bytes each)
        if (offset + 8 > dataSize) throw std::runtime_error("Unexpected end of data (startedAt)");
        std::memcpy(&entry.startedAt, data + offset, 8);
        offset += 8;
        if (offset + 8 > dataSize) throw std::runtime_error("Unexpected end of data (completedAt)");
        std::memcpy(&entry.completedAt, data + offset, 8);
        offset += 8;
        
        // Moves (4 bytes length + compressed data)
        if (offset + 4 > dataSize) throw std::runtime_error("Unexpected end of data (moves length)");
        uint32_t movesLen;
        std::memcpy(&movesLen, data + offset, 4);
        offset += 4;
        
        if (movesLen > 1000000) throw std::runtime_error("Moves data too large");
        if (offset + movesLen > dataSize) throw std::runtime_error("Unexpected end of data (moves)");
        entry.moves = DecompressMoves(data + offset, movesLen);
        offset += movesLen;
        
    } catch (const std::exception& e) {
        std::cerr << "Deserialization error at offset " << offset << ": " << e.what() << std::endl;
        throw;
    }
    
    return entry;
}

void LeaderboardManager::EncryptData(std::vector<uint8_t>& data) {
    for (size_t i = 0; i < data.size(); ++i) {
        data[i] ^= encryptionKey ^ (i & 0xFF);
    }
}

void LeaderboardManager::DecryptData(std::vector<uint8_t>& data) {
    // XOR is symmetric, so encrypt and decrypt are the same
    EncryptData(data);
}

std::vector<uint8_t> LeaderboardManager::CompressMoves(const std::vector<MoveRecord>& moves) {
    std::vector<uint8_t> compressed;

    // Header byte to indicate extended encoding followed by version number
    compressed.push_back(0xFF);
    compressed.push_back(0x01);

    // Each move (version 1):
    // 1 byte tile (0-80), 1 byte value (0-9 or 0 for clear),
    // 4 bytes timestamp (float), 2 bytes notes mask,
    // 2 bytes removed mask, 1 byte auto mode flag, 1 byte action type
    for (const auto& move : moves) {
        compressed.push_back(static_cast<uint8_t>(std::clamp(move.tileNumber, 0, 255)));
        compressed.push_back(static_cast<uint8_t>(std::clamp(move.value, 0, 255)));

        float timeFloat = static_cast<float>(move.timestamp);
        compressed.insert(compressed.end(),
                          reinterpret_cast<uint8_t*>(&timeFloat),
                          reinterpret_cast<uint8_t*>(&timeFloat) + 4);

        uint16_t notes = move.notesMask;
        compressed.insert(compressed.end(),
                          reinterpret_cast<uint8_t*>(&notes),
                          reinterpret_cast<uint8_t*>(&notes) + 2);

        uint16_t removed = move.removedMask;
        compressed.insert(compressed.end(),
                          reinterpret_cast<uint8_t*>(&removed),
                          reinterpret_cast<uint8_t*>(&removed) + 2);

        compressed.push_back(move.autoCandidatesEnabled ? 1u : 0u);
        compressed.push_back(static_cast<uint8_t>(move.action));
    }

    return compressed;
}

std::vector<MoveRecord> LeaderboardManager::DecompressMoves(const uint8_t* data, size_t length) {
    std::vector<MoveRecord> moves;

    if (length < 1) {
        return moves;
    }

    // New format starts with 0xFF marker followed by version byte
    if (data[0] == 0xFF) {
        if (length < 2) {
            return moves;
        }

        uint8_t version = data[1];
        size_t offset = 2;

        if (version == 0x01) {
            const size_t stride = 12;
            while (offset + stride <= length) {
                int tile = data[offset];
                int value = data[offset + 1];

                float timeFloat;
                std::memcpy(&timeFloat, data + offset + 2, 4);
                double timestamp = static_cast<double>(timeFloat);

                uint16_t notes;
                std::memcpy(&notes, data + offset + 6, 2);

                uint16_t removed;
                std::memcpy(&removed, data + offset + 8, 2);

                bool autoMode = data[offset + 10] != 0;
                MoveAction action = static_cast<MoveAction>(data[offset + 11]);

                moves.emplace_back(tile, value, timestamp, notes, removed, autoMode, action);
                offset += stride;
            }
        }

        return moves;
    }

    // Legacy format fallback: 6 bytes per move as originally stored
    for (size_t i = 0; i + 5 < length; i += 6) {
        int tile = data[i];
        int value = data[i + 1];

        float timeFloat;
        std::memcpy(&timeFloat, data + i + 2, 4);
        double timestamp = static_cast<double>(timeFloat);

        moves.emplace_back(tile, value, timestamp, 0, 0, false, MoveAction::Value);
    }

    return moves;
}
