#include "sound.h"

#ifdef _WIN32
#include <windows.h>
#endif

namespace
{
bool soundEnabled = true;
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
#ifdef _WIN32
    Beep(440, 12);
#endif
}

void PlaySoftDropSound()
{
    if (!soundEnabled)
    {
        return;
    }
#ifdef _WIN32
    Beep(330, 10);
#endif
}

void PlayHardDropSound()
{
    if (!soundEnabled)
    {
        return;
    }
#ifdef _WIN32
    Beep(220, 35);
#endif
}

void PlayHoldSound()
{
    if (!soundEnabled)
    {
        return;
    }
#ifdef _WIN32
    Beep(520, 25);
#endif
}

void PlayRotateSound()
{
    if (!soundEnabled)
    {
        return;
    }
#ifdef _WIN32
    Beep(880, 25);
#endif
}

void PlayLineClearSound()
{
    if (!soundEnabled)
    {
        return;
    }
#ifdef _WIN32
    Beep(660, 60);
#endif
}

void PlayLevelUpSound()
{
    if (!soundEnabled)
    {
        return;
    }
#ifdef _WIN32
    Beep(740, 45);
    Beep(980, 65);
#endif
}

void PlayPauseSound()
{
    if (!soundEnabled)
    {
        return;
    }
#ifdef _WIN32
    Beep(500, 30);
#endif
}

void PlayGameOverSound()
{
    if (!soundEnabled)
    {
        return;
    }
#ifdef _WIN32
    Beep(220, 70);
    Beep(165, 110);
#endif
}
