#include "settings_options.h"

#include "settings.h"

TuningPresetValues GetTuningPresetValues(int tuningPreset)
{
    if (tuningPreset == 0)
    {
        return TuningPresetValues{185, 55, 190};
    }
    if (tuningPreset == 2)
    {
        return TuningPresetValues{105, 20, 110};
    }
    return TuningPresetValues{
        SettingsDefaultDasDelayMs,
        SettingsDefaultArrIntervalMs,
        SettingsDefaultClearFlashDurationMs,
    };
}

bool IsTuningPresetCustom(int tuningPreset)
{
    return tuningPreset == SettingsMaxTuningPreset;
}

const char *GetTuningPresetName(int tuningPreset)
{
    if (tuningPreset == 0)
    {
        return "BEGINNER";
    }
    if (tuningPreset == 1)
    {
        return "BALANCED";
    }
    if (tuningPreset == 2)
    {
        return "FAST";
    }
    return "CUSTOM";
}

const char *GetControlSchemeName(int controlScheme)
{
    if (controlScheme == 1)
    {
        return "ARROWS";
    }
    if (controlScheme == 2)
    {
        return "WASD";
    }
    return "HYBRID";
}

bool ControlSchemeAllowsArrows(int controlScheme)
{
    return controlScheme != 2;
}

bool ControlSchemeAllowsWasd(int controlScheme)
{
    return controlScheme != 1;
}
