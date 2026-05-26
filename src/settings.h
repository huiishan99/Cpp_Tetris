#pragma once

#include <string>

constexpr int SettingsDefaultDasDelayMs = 145;
constexpr int SettingsDefaultArrIntervalMs = 35;
constexpr int SettingsDefaultClearFlashDurationMs = 170;
constexpr int SettingsDefaultTuningPreset = 1;
constexpr int SettingsDefaultControlScheme = 0;
constexpr int SettingsDefaultWindowScalePercent = 100;
constexpr int SettingsDefaultSoundVolumePercent = 60;
constexpr int SettingsMinDasDelayMs = 80;
constexpr int SettingsMaxDasDelayMs = 250;
constexpr int SettingsMinArrIntervalMs = 15;
constexpr int SettingsMaxArrIntervalMs = 80;
constexpr int SettingsMinClearFlashDurationMs = 90;
constexpr int SettingsMaxClearFlashDurationMs = 240;
constexpr int SettingsMinTuningPreset = 0;
constexpr int SettingsMaxTuningPreset = 3;
constexpr int SettingsMinControlScheme = 0;
constexpr int SettingsMaxControlScheme = 2;
constexpr int SettingsMinWindowScalePercent = 100;
constexpr int SettingsMaxWindowScalePercent = 150;
constexpr int SettingsMinSoundVolumePercent = 0;
constexpr int SettingsMaxSoundVolumePercent = 100;

struct GameSettings
{
    int dasDelayMs;
    int arrIntervalMs;
    int clearFlashDurationMs;
    int tuningPreset;
    int controlScheme;
    int windowScalePercent;
    bool soundEnabled;
    int soundVolumePercent;
    std::string playerName;
};

GameSettings GetDefaultSettings();
GameSettings SanitizeSettings(const GameSettings &settings);
GameSettings LoadSettings(const std::string &path);
bool SaveSettings(const std::string &path, const GameSettings &settings);
