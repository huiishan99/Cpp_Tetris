#pragma once

#include <string>
#include <vector>

constexpr int LeaderboardMaxEntries = 5;
constexpr int LeaderboardMinNameLength = 3;
constexpr int LeaderboardMaxNameLength = 12;
constexpr const char *DefaultLeaderboardName = "PLAYER";

struct LeaderboardEntry
{
    std::string name;
    int score;
};

std::string NormalizeLeaderboardName(const std::string &name);
std::vector<LeaderboardEntry> NormalizeLeaderboard(const std::vector<LeaderboardEntry> &entries,
                                                   int maxEntries = LeaderboardMaxEntries);
bool DoesScoreQualifyForLeaderboard(const std::vector<LeaderboardEntry> &entries,
                                    int score,
                                    int maxEntries = LeaderboardMaxEntries);
std::vector<LeaderboardEntry> AddLeaderboardScore(const std::vector<LeaderboardEntry> &entries,
                                                  int score,
                                                  const std::string &name = DefaultLeaderboardName,
                                                  int maxEntries = LeaderboardMaxEntries);
int GetBestLeaderboardScore(const std::vector<LeaderboardEntry> &entries);
std::vector<LeaderboardEntry> LoadLeaderboard(const std::string &path);
bool SaveLeaderboard(const std::string &path, const std::vector<LeaderboardEntry> &entries);
