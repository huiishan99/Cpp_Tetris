#pragma once

#include <string>
#include <vector>

constexpr int LeaderboardMaxEntries = 5;
constexpr const char *DefaultLeaderboardName = "PLAYER";

struct LeaderboardEntry
{
    std::string name;
    int score;
};

std::vector<LeaderboardEntry> NormalizeLeaderboard(const std::vector<LeaderboardEntry> &entries,
                                                   int maxEntries = LeaderboardMaxEntries);
std::vector<LeaderboardEntry> AddLeaderboardScore(const std::vector<LeaderboardEntry> &entries,
                                                  int score,
                                                  const std::string &name = DefaultLeaderboardName,
                                                  int maxEntries = LeaderboardMaxEntries);
int GetBestLeaderboardScore(const std::vector<LeaderboardEntry> &entries);
std::vector<LeaderboardEntry> LoadLeaderboard(const std::string &path);
bool SaveLeaderboard(const std::string &path, const std::vector<LeaderboardEntry> &entries);
