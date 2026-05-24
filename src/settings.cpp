#include "settings.h"

#include "leaderboard.h"

#include <fstream>
#include <sstream>

namespace
{
int Clamp(int value, int minimum, int maximum)
{
    if (value < minimum)
    {
        return minimum;
    }
    if (value > maximum)
    {
        return maximum;
    }
    return value;
}

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

GameSettings GetDefaultSettings()
{
    return GameSettings{
        SettingsDefaultDasDelayMs,
        SettingsDefaultArrIntervalMs,
        SettingsDefaultClearFlashDurationMs,
        SettingsDefaultTuningPreset,
        SettingsDefaultControlScheme,
        SettingsDefaultWindowScalePercent,
        true,
        SettingsDefaultSoundVolumePercent,
        DefaultLeaderboardName,
    };
}

GameSettings SanitizeSettings(const GameSettings &settings)
{
    GameSettings sanitized = settings;
    sanitized.dasDelayMs = Clamp(sanitized.dasDelayMs, SettingsMinDasDelayMs, SettingsMaxDasDelayMs);
    sanitized.arrIntervalMs = Clamp(sanitized.arrIntervalMs, SettingsMinArrIntervalMs, SettingsMaxArrIntervalMs);
    sanitized.clearFlashDurationMs = Clamp(sanitized.clearFlashDurationMs,
                                           SettingsMinClearFlashDurationMs,
                                           SettingsMaxClearFlashDurationMs);
    sanitized.tuningPreset = Clamp(sanitized.tuningPreset, SettingsMinTuningPreset, SettingsMaxTuningPreset);
    sanitized.controlScheme = Clamp(sanitized.controlScheme, SettingsMinControlScheme, SettingsMaxControlScheme);
    sanitized.windowScalePercent = Clamp(sanitized.windowScalePercent,
                                         SettingsMinWindowScalePercent,
                                         SettingsMaxWindowScalePercent);
    sanitized.soundVolumePercent = Clamp(sanitized.soundVolumePercent,
                                         SettingsMinSoundVolumePercent,
                                         SettingsMaxSoundVolumePercent);
    if (!sanitized.soundEnabled)
    {
        sanitized.soundVolumePercent = 0;
    }
    else if (sanitized.soundVolumePercent == 0)
    {
        sanitized.soundEnabled = false;
    }
    sanitized.playerName = NormalizeLeaderboardName(sanitized.playerName);
    return sanitized;
}

GameSettings LoadSettings(const std::string &path)
{
    GameSettings settings = GetDefaultSettings();
    std::ifstream file(path);
    if (!file)
    {
        return settings;
    }

    std::string line;
    while (std::getline(file, line))
    {
        std::string::size_type separator = line.find('=');
        if (separator == std::string::npos)
        {
            continue;
        }

        std::string key = line.substr(0, separator);
        std::string valueText = line.substr(separator + 1);
        if (key == "player_name")
        {
            settings.playerName = valueText;
            continue;
        }

        int value = 0;
        if (!TryParseInt(valueText, value))
        {
            continue;
        }

        if (key == "das_delay_ms")
        {
            settings.dasDelayMs = value;
        }
        else if (key == "arr_interval_ms")
        {
            settings.arrIntervalMs = value;
        }
        else if (key == "clear_flash_ms")
        {
            settings.clearFlashDurationMs = value;
        }
        else if (key == "tuning_preset")
        {
            settings.tuningPreset = value;
        }
        else if (key == "control_scheme")
        {
            settings.controlScheme = value;
        }
        else if (key == "window_scale_percent")
        {
            settings.windowScalePercent = value;
        }
        else if (key == "sound_enabled")
        {
            settings.soundEnabled = value != 0;
        }
        else if (key == "sound_volume_percent")
        {
            settings.soundVolumePercent = value;
        }
    }

    return SanitizeSettings(settings);
}

bool SaveSettings(const std::string &path, const GameSettings &settings)
{
    GameSettings sanitized = SanitizeSettings(settings);
    std::ofstream file(path, std::ios::trunc);
    if (!file)
    {
        return false;
    }

    file << "das_delay_ms=" << sanitized.dasDelayMs << '\n';
    file << "arr_interval_ms=" << sanitized.arrIntervalMs << '\n';
    file << "clear_flash_ms=" << sanitized.clearFlashDurationMs << '\n';
    file << "tuning_preset=" << sanitized.tuningPreset << '\n';
    file << "control_scheme=" << sanitized.controlScheme << '\n';
    file << "window_scale_percent=" << sanitized.windowScalePercent << '\n';
    file << "sound_enabled=" << (sanitized.soundEnabled ? 1 : 0) << '\n';
    file << "sound_volume_percent=" << sanitized.soundVolumePercent << '\n';
    file << "player_name=" << sanitized.playerName << '\n';
    return static_cast<bool>(file);
}
