#pragma once

void PlayMoveSound();
void PlaySoftDropSound();
void PlayHardDropSound();
void PlayHoldSound();
void PlayRotateSound();
void PlayLineClearSound(int completedLines, bool spinClear);
void PlayLevelUpSound();
void PlayPauseSound();
void PlayGameOverSound();
void SetSoundEnabled(bool enabled);
bool IsSoundEnabled();
void SetSoundVolumePercent(int volumePercent);
int GetSoundVolumePercent();
