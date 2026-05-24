#pragma once

struct TuningPresetValues
{
    int dasDelayMs;
    int arrIntervalMs;
    int clearFlashDurationMs;
};

TuningPresetValues GetTuningPresetValues(int tuningPreset);
bool IsTuningPresetCustom(int tuningPreset);
const char *GetTuningPresetName(int tuningPreset);
const char *GetControlSchemeName(int controlScheme);
bool ControlSchemeAllowsArrows(int controlScheme);
bool ControlSchemeAllowsWasd(int controlScheme);
