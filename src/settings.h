#pragma once

#include <string>

constexpr int SettingsDefaultDasDelayMs = 145;
constexpr int SettingsDefaultArrIntervalMs = 35;
constexpr int SettingsDefaultClearFlashDurationMs = 170;
constexpr int SettingsMinDasDelayMs = 80;
constexpr int SettingsMaxDasDelayMs = 250;
constexpr int SettingsMinArrIntervalMs = 15;
constexpr int SettingsMaxArrIntervalMs = 80;
constexpr int SettingsMinClearFlashDurationMs = 90;
constexpr int SettingsMaxClearFlashDurationMs = 240;

struct GameSettings
{
    int dasDelayMs;
    int arrIntervalMs;
    int clearFlashDurationMs;
    bool soundEnabled;
};

GameSettings GetDefaultSettings();
GameSettings SanitizeSettings(const GameSettings &settings);
GameSettings LoadSettings(const std::string &path);
bool SaveSettings(const std::string &path, const GameSettings &settings);
