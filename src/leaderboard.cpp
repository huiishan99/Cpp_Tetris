#include "leaderboard.h"

#include <algorithm>
#include <fstream>
#include <sstream>

namespace
{
bool TryParseInt(const std::string &text, int &value)
{
    std::istringstream stream(text);
    int parsed = 0;
    if (!(stream >> parsed))
    {
        return false;
    }
    value = parsed;
    return true;
}
}

std::string NormalizeLeaderboardName(const std::string &name)
{
    if (name.empty())
    {
        return DefaultLeaderboardName;
    }
    return name.substr(0, LeaderboardMaxNameLength);
}

std::vector<LeaderboardEntry> NormalizeLeaderboard(const std::vector<LeaderboardEntry> &entries,
                                                   int maxEntries)
{
    if (maxEntries < 1)
    {
        maxEntries = 1;
    }

    std::vector<LeaderboardEntry> normalized;
    for (const LeaderboardEntry &entry : entries)
    {
        if (entry.score < 0)
        {
            continue;
        }
        normalized.push_back(LeaderboardEntry{NormalizeLeaderboardName(entry.name), entry.score});
    }

    std::sort(normalized.begin(), normalized.end(),
              [](const LeaderboardEntry &left, const LeaderboardEntry &right) {
                  return left.score > right.score;
              });

    if (static_cast<int>(normalized.size()) > maxEntries)
    {
        normalized.resize(maxEntries);
    }
    return normalized;
}

bool DoesScoreQualifyForLeaderboard(const std::vector<LeaderboardEntry> &entries,
                                    int score,
                                    int maxEntries)
{
    if (score <= 0)
    {
        return false;
    }
    if (maxEntries < 1)
    {
        maxEntries = 1;
    }

    std::vector<LeaderboardEntry> normalized = NormalizeLeaderboard(entries, maxEntries);
    if (static_cast<int>(normalized.size()) < maxEntries)
    {
        return true;
    }

    return score >= normalized.back().score;
}

std::vector<LeaderboardEntry> AddLeaderboardScore(const std::vector<LeaderboardEntry> &entries,
                                                  int score,
                                                  const std::string &name,
                                                  int maxEntries)
{
    std::vector<LeaderboardEntry> updated = entries;
    updated.push_back(LeaderboardEntry{NormalizeLeaderboardName(name), score});
    return NormalizeLeaderboard(updated, maxEntries);
}

int GetBestLeaderboardScore(const std::vector<LeaderboardEntry> &entries)
{
    std::vector<LeaderboardEntry> normalized = NormalizeLeaderboard(entries);
    if (normalized.empty())
    {
        return 0;
    }
    return normalized.front().score;
}

std::vector<LeaderboardEntry> LoadLeaderboard(const std::string &path)
{
    std::ifstream file(path);
    if (!file)
    {
        return {};
    }

    std::vector<LeaderboardEntry> entries;
    std::string line;
    while (std::getline(file, line))
    {
        std::string::size_type separator = line.find('|');
        if (separator == std::string::npos)
        {
            int legacyScore = 0;
            if (TryParseInt(line, legacyScore))
            {
                entries.push_back(LeaderboardEntry{DefaultLeaderboardName, legacyScore});
            }
            continue;
        }

        std::string name = line.substr(0, separator);
        std::string scoreText = line.substr(separator + 1);
        int score = 0;
        if (TryParseInt(scoreText, score))
        {
            entries.push_back(LeaderboardEntry{name, score});
        }
    }

    return NormalizeLeaderboard(entries);
}

bool SaveLeaderboard(const std::string &path, const std::vector<LeaderboardEntry> &entries)
{
    std::vector<LeaderboardEntry> normalized = NormalizeLeaderboard(entries);
    std::ofstream file(path, std::ios::trunc);
    if (!file)
    {
        return false;
    }

    for (const LeaderboardEntry &entry : normalized)
    {
        file << entry.name << '|' << entry.score << '\n';
    }
    return static_cast<bool>(file);
}
