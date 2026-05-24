#include "sound.h"

#ifdef _WIN32
#include <windows.h>
#endif

namespace
{
bool soundEnabled = true;

void PlayTone(int frequency, int durationMs)
{
#ifdef _WIN32
    Beep(frequency, durationMs);
#else
    (void)frequency;
    (void)durationMs;
#endif
}
}

void SetSoundEnabled(bool enabled)
{
    soundEnabled = enabled;
}

bool IsSoundEnabled()
{
    return soundEnabled;
}

void PlayMoveSound()
{
    if (!soundEnabled)
    {
        return;
    }
    PlayTone(560, 8);
}

void PlaySoftDropSound()
{
    if (!soundEnabled)
    {
        return;
    }
    PlayTone(370, 8);
}

void PlayHardDropSound()
{
    if (!soundEnabled)
    {
        return;
    }
    PlayTone(220, 18);
    PlayTone(160, 38);
}

void PlayHoldSound()
{
    if (!soundEnabled)
    {
        return;
    }
    PlayTone(520, 18);
    PlayTone(660, 22);
}

void PlayRotateSound()
{
    if (!soundEnabled)
    {
        return;
    }
    PlayTone(780, 14);
    PlayTone(980, 12);
}

void PlayLineClearSound(int completedLines, bool spinClear)
{
    if (!soundEnabled)
    {
        return;
    }

    if (spinClear)
    {
        PlayTone(880, 28);
        PlayTone(1175, 36);
        PlayTone(1480, 48);
        return;
    }

    switch (completedLines)
    {
    case 1:
        PlayTone(660, 35);
        break;
    case 2:
        PlayTone(660, 28);
        PlayTone(784, 38);
        break;
    case 3:
        PlayTone(660, 24);
        PlayTone(784, 28);
        PlayTone(988, 44);
        break;
    case 4:
        PlayTone(523, 26);
        PlayTone(659, 26);
        PlayTone(784, 32);
        PlayTone(1046, 58);
        break;
    default:
        PlayTone(660, 35);
        break;
    }
}

void PlayLevelUpSound()
{
    if (!soundEnabled)
    {
        return;
    }
    PlayTone(740, 34);
    PlayTone(932, 34);
    PlayTone(1175, 70);
}

void PlayPauseSound()
{
    if (!soundEnabled)
    {
        return;
    }
    PlayTone(500, 18);
    PlayTone(420, 20);
}

void PlayGameOverSound()
{
    if (!soundEnabled)
    {
        return;
    }
    PlayTone(330, 42);
    PlayTone(247, 60);
    PlayTone(165, 78);
}
