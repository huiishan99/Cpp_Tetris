#include "sound.h"

#include <atomic>
#include <cstddef>
#include <deque>
#include <initializer_list>
#include <utility>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#endif

namespace
{
struct Tone
{
    int frequency;
    int durationMs;
};

std::atomic_bool soundEnabled{true};
std::atomic_int soundVolumePercent{100};

#ifdef _WIN32
constexpr std::size_t MaxQueuedCues = 4;

CRITICAL_SECTION soundLock;
bool soundLockInitialized = false;
HANDLE soundEvent = nullptr;
HANDLE soundThread = nullptr;
bool soundShutdownRequested = false;
std::deque<std::vector<Tone>> soundQueue;
#endif

int ClampVolume(int value)
{
    if (value < 0)
    {
        return 0;
    }
    if (value > 100)
    {
        return 100;
    }
    return value;
}

#ifdef _WIN32
int GetScaledDurationMs(int durationMs)
{
    int volumePercent = soundVolumePercent.load();
    if (!soundEnabled.load() || volumePercent <= 0)
    {
        return 0;
    }

    int scaledDurationMs = durationMs * volumePercent / 100;
    if (scaledDurationMs <= 0)
    {
        scaledDurationMs = 1;
    }
    return scaledDurationMs;
}

void PlayToneBlocking(const Tone &tone)
{
    int scaledDurationMs = GetScaledDurationMs(tone.durationMs);
    if (scaledDurationMs <= 0)
    {
        return;
    }

    Beep(tone.frequency, scaledDurationMs);
}

DWORD WINAPI SoundThreadMain(LPVOID)
{
    while (true)
    {
        WaitForSingleObject(soundEvent, INFINITE);

        while (true)
        {
            std::vector<Tone> cue;

            EnterCriticalSection(&soundLock);
            if (soundShutdownRequested)
            {
                LeaveCriticalSection(&soundLock);
                return 0;
            }
            if (soundQueue.empty())
            {
                LeaveCriticalSection(&soundLock);
                break;
            }
            cue = std::move(soundQueue.front());
            soundQueue.pop_front();
            LeaveCriticalSection(&soundLock);

            for (const Tone &tone : cue)
            {
                EnterCriticalSection(&soundLock);
                bool shouldStop = soundShutdownRequested;
                LeaveCriticalSection(&soundLock);
                if (shouldStop)
                {
                    return 0;
                }
                PlayToneBlocking(tone);
            }
        }
    }
}

bool EnsureSoundEngine()
{
    if (!soundLockInitialized)
    {
        InitializeCriticalSection(&soundLock);
        soundLockInitialized = true;
    }

    EnterCriticalSection(&soundLock);
    if (soundEvent == nullptr)
    {
        soundEvent = CreateEventA(nullptr, FALSE, FALSE, nullptr);
    }
    if (soundEvent != nullptr && soundThread == nullptr)
    {
        soundShutdownRequested = false;
        soundThread = CreateThread(nullptr, 0, SoundThreadMain, nullptr, 0, nullptr);
    }
    bool ready = soundEvent != nullptr && soundThread != nullptr;
    LeaveCriticalSection(&soundLock);
    return ready;
}

void ClearQueuedSounds()
{
    if (!soundLockInitialized)
    {
        return;
    }

    EnterCriticalSection(&soundLock);
    soundQueue.clear();
    LeaveCriticalSection(&soundLock);
}

void QueueCue(std::initializer_list<Tone> tones, bool replaceQueued, bool dropIfBusy)
{
    if (!soundEnabled.load() || soundVolumePercent.load() <= 0 || tones.size() == 0)
    {
        return;
    }
    if (!EnsureSoundEngine())
    {
        return;
    }

    EnterCriticalSection(&soundLock);
    if (soundShutdownRequested)
    {
        LeaveCriticalSection(&soundLock);
        return;
    }
    if (replaceQueued)
    {
        soundQueue.clear();
    }
    else if (dropIfBusy && !soundQueue.empty())
    {
        LeaveCriticalSection(&soundLock);
        return;
    }
    else if (soundQueue.size() >= MaxQueuedCues)
    {
        soundQueue.pop_front();
    }

    soundQueue.emplace_back(tones);
    SetEvent(soundEvent);
    LeaveCriticalSection(&soundLock);
}
#else
void ClearQueuedSounds()
{
}
#endif

void PlayCue(std::initializer_list<Tone> tones, bool replaceQueued = false, bool dropIfBusy = false)
{
    if (!soundEnabled.load() || soundVolumePercent.load() <= 0)
    {
        return;
    }

#ifdef _WIN32
    QueueCue(tones, replaceQueued, dropIfBusy);
#else
    (void)tones;
    (void)replaceQueued;
    (void)dropIfBusy;
#endif
}
}

void SetSoundEnabled(bool enabled)
{
    soundEnabled.store(enabled);
    if (!soundEnabled)
    {
        soundVolumePercent.store(0);
        ClearQueuedSounds();
    }
    else if (soundVolumePercent.load() == 0)
    {
        soundVolumePercent.store(100);
    }
}

bool IsSoundEnabled()
{
    return soundEnabled.load();
}

void SetSoundVolumePercent(int volumePercent)
{
    int clampedVolumePercent = ClampVolume(volumePercent);
    soundVolumePercent.store(clampedVolumePercent);
    soundEnabled.store(clampedVolumePercent > 0);
    if (clampedVolumePercent == 0)
    {
        ClearQueuedSounds();
    }
}

int GetSoundVolumePercent()
{
    return soundVolumePercent.load();
}

void PlayMoveSound()
{
    PlayCue({{560, 8}}, false, true);
}

void PlaySoftDropSound()
{
    PlayCue({{370, 8}}, false, true);
}

void PlayHardDropSound()
{
    PlayCue({{220, 18}, {160, 38}}, true);
}

void PlayHoldSound()
{
    PlayCue({{520, 18}, {660, 22}}, false, true);
}

void PlayRotateSound()
{
    PlayCue({{780, 14}, {980, 12}}, false, true);
}

void PlayLineClearSound(int completedLines, bool spinClear)
{
    if (spinClear)
    {
        PlayCue({{880, 28}, {1175, 36}, {1480, 48}}, true);
        return;
    }

    switch (completedLines)
    {
    case 1:
        PlayCue({{660, 35}}, true);
        break;
    case 2:
        PlayCue({{660, 28}, {784, 38}}, true);
        break;
    case 3:
        PlayCue({{660, 24}, {784, 28}, {988, 44}}, true);
        break;
    case 4:
        PlayCue({{523, 26}, {659, 26}, {784, 32}, {1046, 58}}, true);
        break;
    default:
        PlayCue({{660, 35}}, true);
        break;
    }
}

void PlayBackToBackSound()
{
    PlayCue({{988, 18}, {1175, 22}, {1319, 34}});
}

void PlayPerfectClearSound()
{
    PlayCue({{784, 24}, {988, 24}, {1175, 28}, {1568, 62}});
}

void PlayLevelUpSound()
{
    PlayCue({{740, 34}, {932, 34}, {1175, 70}});
}

void PlayPauseSound()
{
    PlayCue({{500, 18}, {420, 20}}, true);
}

void PlayGameOverSound()
{
    PlayCue({{330, 42}, {247, 60}, {165, 78}}, true);
}

void ShutdownSound()
{
#ifdef _WIN32
    if (!soundLockInitialized)
    {
        return;
    }

    HANDLE threadToWait = nullptr;

    EnterCriticalSection(&soundLock);
    soundShutdownRequested = true;
    soundQueue.clear();
    threadToWait = soundThread;
    if (soundEvent != nullptr)
    {
        SetEvent(soundEvent);
    }
    LeaveCriticalSection(&soundLock);

    if (threadToWait != nullptr)
    {
        WaitForSingleObject(threadToWait, INFINITE);
    }

    EnterCriticalSection(&soundLock);
    if (soundThread != nullptr)
    {
        CloseHandle(soundThread);
        soundThread = nullptr;
    }
    if (soundEvent != nullptr)
    {
        CloseHandle(soundEvent);
        soundEvent = nullptr;
    }
    soundShutdownRequested = false;
    LeaveCriticalSection(&soundLock);

    DeleteCriticalSection(&soundLock);
    soundLockInitialized = false;
#endif
}
