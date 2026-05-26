#include "sound.h"

#include <algorithm>
#include <atomic>

#ifdef _WIN32
#include <array>
#include <cmath>
#include <cstdint>
#include <windows.h>
#include <mmsystem.h>
#include <vector>

#ifdef _MSC_VER
#pragma comment(lib, "winmm.lib")
#endif
#endif

namespace
{
std::atomic_bool soundEnabled{true};
std::atomic_int soundVolumePercent{100};

enum class SoundCue
{
    Move,
    SoftDrop,
    HardDrop,
    Hold,
    Rotate,
    ClearSingle,
    ClearDouble,
    ClearTriple,
    ClearTetris,
    SpinClear,
    BackToBack,
    PerfectClear,
    LevelUp,
    Pause,
    GameOver,
    Silent,
    Count
};

int ClampVolume(int value)
{
    return std::max(0, std::min(100, value));
}

#ifdef _WIN32
constexpr int SampleRate = 44100;
constexpr double Pi = 3.14159265358979323846;
constexpr double TwoPi = Pi * 2.0;
constexpr double MasterOutputGain = 0.38;
constexpr int CueCount = static_cast<int>(SoundCue::Count);

std::array<std::vector<char>, CueCount> soundCache;
bool soundCacheReady = false;
int cachedSoundVolumePercent = -1;

int CueIndex(SoundCue cue)
{
    return static_cast<int>(cue);
}

int FrameFromMs(double milliseconds)
{
    return static_cast<int>(std::round(milliseconds * SampleRate / 1000.0));
}

double SmoothStep(double value)
{
    value = std::max(0.0, std::min(1.0, value));
    return value * value * (3.0 - 2.0 * value);
}

double GetEnvelope(double localMs, double durationMs, double attackMs, double releaseMs)
{
    double envelope = 1.0;
    if (attackMs > 0.0)
    {
        envelope = std::min(envelope, localMs / attackMs);
    }
    if (releaseMs > 0.0)
    {
        envelope = std::min(envelope, (durationMs - localMs) / releaseMs);
    }
    return SmoothStep(envelope);
}

void AddTone(std::vector<double> &mix,
             double startMs,
             double durationMs,
             double startFrequency,
             double endFrequency,
             double gain,
             double attackMs = 1.0,
             double releaseMs = 12.0)
{
    int startFrame = std::max(0, FrameFromMs(startMs));
    int endFrame = std::min(static_cast<int>(mix.size()), FrameFromMs(startMs + durationMs));
    double phase = 0.0;

    for (int frame = startFrame; frame < endFrame; frame++)
    {
        double localMs = (frame - startFrame) * 1000.0 / SampleRate;
        double progress = durationMs <= 0.0 ? 1.0 : localMs / durationMs;
        double frequency = startFrequency + (endFrequency - startFrequency) * progress;
        phase += TwoPi * frequency / SampleRate;
        mix[frame] += std::sin(phase) * gain * GetEnvelope(localMs, durationMs, attackMs, releaseMs);
    }
}

void AddNoise(std::vector<double> &mix,
              double startMs,
              double durationMs,
              double gain,
              double decayMs,
              unsigned int seed)
{
    int startFrame = std::max(0, FrameFromMs(startMs));
    int endFrame = std::min(static_cast<int>(mix.size()), FrameFromMs(startMs + durationMs));
    unsigned int state = seed;

    for (int frame = startFrame; frame < endFrame; frame++)
    {
        double localMs = (frame - startFrame) * 1000.0 / SampleRate;
        state = state * 1664525u + 1013904223u;
        double noise = (static_cast<double>((state >> 8) & 0xffffu) / 32767.5) - 1.0;
        double envelope = std::exp(-localMs / std::max(1.0, decayMs));
        envelope *= GetEnvelope(localMs, durationMs, 0.2, 4.0);
        mix[frame] += noise * gain * envelope;
    }
}

void AddNote(std::vector<double> &mix, double startMs, double durationMs, double frequency, double gain)
{
    AddTone(mix, startMs, durationMs, frequency, frequency * 1.015, gain, 1.0, durationMs * 0.55);
    AddTone(mix, startMs, durationMs * 0.8, frequency * 2.0, frequency * 2.02, gain * 0.32, 0.6, durationMs * 0.45);
}

std::vector<double> MakeMix(double durationMs)
{
    return std::vector<double>(std::max(1, FrameFromMs(durationMs)), 0.0);
}

std::vector<short> ConvertToPcm(const std::vector<double> &mix, int volumePercent)
{
    std::vector<short> samples;
    samples.reserve(mix.size());
    double normalizedVolume = static_cast<double>(ClampVolume(volumePercent)) / 100.0;
    double volume = MasterOutputGain * std::pow(normalizedVolume, 1.45);

    for (double sample : mix)
    {
        sample *= volume;
        sample = std::max(-1.0, std::min(1.0, sample));
        samples.push_back(static_cast<short>(std::round(sample * 30000.0)));
    }
    return samples;
}

std::vector<short> BuildCueSamples(SoundCue cue, int volumePercent)
{
    std::vector<double> mix;

    switch (cue)
    {
    case SoundCue::Move:
        mix = MakeMix(24.0);
        AddNoise(mix, 0.0, 7.0, 0.04, 2.0, 11u);
        AddTone(mix, 0.0, 22.0, 620.0, 760.0, 0.13, 0.4, 9.0);
        break;
    case SoundCue::SoftDrop:
        mix = MakeMix(20.0);
        AddNoise(mix, 0.0, 6.0, 0.035, 2.0, 17u);
        AddTone(mix, 0.0, 18.0, 980.0, 760.0, 0.12, 0.35, 8.0);
        break;
    case SoundCue::HardDrop:
        mix = MakeMix(72.0);
        AddNoise(mix, 0.0, 13.0, 0.20, 4.5, 23u);
        AddTone(mix, 0.0, 34.0, 185.0, 118.0, 0.13, 0.5, 22.0);
        AddTone(mix, 0.0, 30.0, 1350.0, 2200.0, 0.20, 0.25, 11.0);
        AddTone(mix, 14.0, 32.0, 520.0, 390.0, 0.07, 0.5, 18.0);
        break;
    case SoundCue::Hold:
        mix = MakeMix(62.0);
        AddNoise(mix, 0.0, 8.0, 0.05, 3.0, 29u);
        AddTone(mix, 0.0, 38.0, 620.0, 880.0, 0.12, 0.6, 16.0);
        AddTone(mix, 18.0, 34.0, 880.0, 1180.0, 0.10, 0.6, 14.0);
        break;
    case SoundCue::Rotate:
        mix = MakeMix(28.0);
        AddNoise(mix, 0.0, 5.0, 0.03, 2.0, 31u);
        AddTone(mix, 0.0, 24.0, 980.0, 1320.0, 0.13, 0.35, 9.0);
        break;
    case SoundCue::ClearSingle:
        mix = MakeMix(115.0);
        AddNoise(mix, 0.0, 20.0, 0.09, 7.0, 37u);
        AddNote(mix, 0.0, 62.0, 660.0, 0.14);
        AddNote(mix, 38.0, 62.0, 990.0, 0.12);
        break;
    case SoundCue::ClearDouble:
        mix = MakeMix(145.0);
        AddNoise(mix, 0.0, 24.0, 0.11, 8.0, 41u);
        AddNote(mix, 0.0, 64.0, 620.0, 0.13);
        AddNote(mix, 36.0, 70.0, 784.0, 0.13);
        AddNote(mix, 78.0, 58.0, 1046.0, 0.12);
        break;
    case SoundCue::ClearTriple:
        mix = MakeMix(178.0);
        AddNoise(mix, 0.0, 30.0, 0.12, 9.0, 43u);
        AddNote(mix, 0.0, 68.0, 620.0, 0.13);
        AddNote(mix, 36.0, 72.0, 784.0, 0.13);
        AddNote(mix, 76.0, 72.0, 988.0, 0.12);
        AddNote(mix, 116.0, 54.0, 1175.0, 0.11);
        break;
    case SoundCue::ClearTetris:
        mix = MakeMix(255.0);
        AddNoise(mix, 0.0, 16.0, 0.18, 4.0, 47u);
        AddNoise(mix, 24.0, 150.0, 0.055, 38.0, 53u);
        AddTone(mix, 0.0, 46.0, 155.0, 92.0, 0.12, 0.4, 30.0);
        AddNote(mix, 0.0, 72.0, 523.0, 0.13);
        AddNote(mix, 42.0, 76.0, 659.0, 0.14);
        AddNote(mix, 84.0, 80.0, 784.0, 0.14);
        AddNote(mix, 126.0, 88.0, 1046.0, 0.15);
        AddNote(mix, 178.0, 60.0, 1318.0, 0.12);
        break;
    case SoundCue::SpinClear:
        mix = MakeMix(200.0);
        AddNoise(mix, 0.0, 24.0, 0.13, 7.0, 59u);
        AddTone(mix, 0.0, 125.0, 780.0, 1568.0, 0.15, 0.6, 52.0);
        AddTone(mix, 22.0, 110.0, 1175.0, 1760.0, 0.10, 0.8, 40.0);
        AddNote(mix, 112.0, 70.0, 1480.0, 0.11);
        break;
    case SoundCue::BackToBack:
        mix = MakeMix(205.0);
        AddNoise(mix, 0.0, 14.0, 0.16, 4.0, 61u);
        AddTone(mix, 0.0, 38.0, 190.0, 130.0, 0.10, 0.4, 24.0);
        AddNote(mix, 0.0, 72.0, 988.0, 0.13);
        AddNote(mix, 48.0, 76.0, 1175.0, 0.14);
        AddNote(mix, 98.0, 84.0, 1568.0, 0.14);
        break;
    case SoundCue::PerfectClear:
        mix = MakeMix(330.0);
        AddNoise(mix, 0.0, 18.0, 0.17, 5.0, 67u);
        AddNoise(mix, 36.0, 210.0, 0.055, 55.0, 71u);
        AddTone(mix, 0.0, 42.0, 175.0, 120.0, 0.10, 0.4, 26.0);
        AddNote(mix, 0.0, 78.0, 784.0, 0.13);
        AddNote(mix, 44.0, 82.0, 988.0, 0.13);
        AddNote(mix, 88.0, 86.0, 1175.0, 0.13);
        AddNote(mix, 132.0, 92.0, 1568.0, 0.14);
        AddNote(mix, 188.0, 90.0, 1975.0, 0.11);
        AddNote(mix, 236.0, 74.0, 2349.0, 0.09);
        break;
    case SoundCue::LevelUp:
        mix = MakeMix(250.0);
        AddNoise(mix, 0.0, 14.0, 0.11, 5.0, 73u);
        AddNote(mix, 0.0, 80.0, 740.0, 0.12);
        AddNote(mix, 54.0, 86.0, 932.0, 0.13);
        AddNote(mix, 112.0, 112.0, 1175.0, 0.14);
        break;
    case SoundCue::Pause:
        mix = MakeMix(82.0);
        AddTone(mix, 0.0, 42.0, 560.0, 460.0, 0.10, 0.8, 16.0);
        AddTone(mix, 32.0, 38.0, 420.0, 360.0, 0.08, 0.8, 16.0);
        break;
    case SoundCue::GameOver:
        mix = MakeMix(420.0);
        AddTone(mix, 0.0, 110.0, 330.0, 315.0, 0.11, 3.0, 45.0);
        AddTone(mix, 96.0, 120.0, 247.0, 238.0, 0.11, 3.0, 52.0);
        AddTone(mix, 202.0, 150.0, 196.0, 178.0, 0.10, 3.0, 72.0);
        AddTone(mix, 310.0, 90.0, 165.0, 150.0, 0.08, 3.0, 58.0);
        break;
    case SoundCue::Silent:
        mix = MakeMix(3.0);
        break;
    case SoundCue::Count:
        mix = MakeMix(1.0);
        break;
    }

    return ConvertToPcm(mix, volumePercent);
}

void AppendFourCc(std::vector<char> &bytes, const char *text)
{
    for (int index = 0; index < 4; index++)
    {
        bytes.push_back(text[index]);
    }
}

void AppendUInt16(std::vector<char> &bytes, unsigned int value)
{
    bytes.push_back(static_cast<char>(value & 0xffu));
    bytes.push_back(static_cast<char>((value >> 8) & 0xffu));
}

void AppendUInt32(std::vector<char> &bytes, unsigned int value)
{
    bytes.push_back(static_cast<char>(value & 0xffu));
    bytes.push_back(static_cast<char>((value >> 8) & 0xffu));
    bytes.push_back(static_cast<char>((value >> 16) & 0xffu));
    bytes.push_back(static_cast<char>((value >> 24) & 0xffu));
}

std::vector<char> BuildWavBytes(const std::vector<short> &samples)
{
    constexpr int Channels = 1;
    constexpr int BitsPerSample = 16;
    constexpr int BlockAlign = Channels * BitsPerSample / 8;
    constexpr int ByteRate = SampleRate * BlockAlign;

    unsigned int dataSize = static_cast<unsigned int>(samples.size() * sizeof(short));
    std::vector<char> bytes;
    bytes.reserve(44 + dataSize);

    AppendFourCc(bytes, "RIFF");
    AppendUInt32(bytes, 36u + dataSize);
    AppendFourCc(bytes, "WAVE");
    AppendFourCc(bytes, "fmt ");
    AppendUInt32(bytes, 16u);
    AppendUInt16(bytes, 1u);
    AppendUInt16(bytes, Channels);
    AppendUInt32(bytes, SampleRate);
    AppendUInt32(bytes, ByteRate);
    AppendUInt16(bytes, BlockAlign);
    AppendUInt16(bytes, BitsPerSample);
    AppendFourCc(bytes, "data");
    AppendUInt32(bytes, dataSize);

    for (short sample : samples)
    {
        unsigned short value = static_cast<unsigned short>(sample);
        AppendUInt16(bytes, value);
    }
    return bytes;
}

void StopSoundPlayback()
{
    PlaySoundA(nullptr, nullptr, 0);
}

void MarkSoundCacheDirty()
{
    soundCacheReady = false;
    cachedSoundVolumePercent = -1;
}

bool EnsureSoundLibrary()
{
    int volumePercent = soundVolumePercent.load();
    if (!soundEnabled.load() || volumePercent <= 0)
    {
        return false;
    }
    if (soundCacheReady && cachedSoundVolumePercent == volumePercent)
    {
        return true;
    }

    StopSoundPlayback();
    for (int index = 0; index < CueCount; index++)
    {
        SoundCue cue = static_cast<SoundCue>(index);
        soundCache[index] = BuildWavBytes(BuildCueSamples(cue, volumePercent));
    }
    soundCacheReady = true;
    cachedSoundVolumePercent = volumePercent;
    return true;
}
#else
void StopSoundPlayback()
{
}

void MarkSoundCacheDirty()
{
}
#endif

void PlayCue(SoundCue cue, bool interruptCurrent)
{
    if (!soundEnabled.load() || soundVolumePercent.load() <= 0)
    {
        return;
    }

#ifdef _WIN32
    if (!EnsureSoundLibrary())
    {
        return;
    }

    const std::vector<char> &bytes = soundCache[CueIndex(cue)];
    DWORD flags = SND_MEMORY | SND_ASYNC | SND_NODEFAULT;
    if (!interruptCurrent)
    {
        flags |= SND_NOSTOP;
    }
    PlaySoundA(bytes.data(), nullptr, flags);
#else
    (void)cue;
    (void)interruptCurrent;
#endif
}

SoundCue GetClearCue(int completedLines, bool spinClear, bool backToBack, bool perfectClear)
{
    if (perfectClear)
    {
        return SoundCue::PerfectClear;
    }
    if (backToBack)
    {
        return SoundCue::BackToBack;
    }
    if (spinClear)
    {
        return SoundCue::SpinClear;
    }

    switch (completedLines)
    {
    case 1:
        return SoundCue::ClearSingle;
    case 2:
        return SoundCue::ClearDouble;
    case 3:
        return SoundCue::ClearTriple;
    case 4:
        return SoundCue::ClearTetris;
    default:
        return SoundCue::ClearSingle;
    }
}
}

void WarmUpSound()
{
#ifdef _WIN32
    if (EnsureSoundLibrary())
    {
        PlayCue(SoundCue::Silent, true);
        StopSoundPlayback();
    }
#endif
}

void SetSoundEnabled(bool enabled)
{
    soundEnabled.store(enabled);
    if (!enabled)
    {
        soundVolumePercent.store(0);
        StopSoundPlayback();
        return;
    }
    if (soundVolumePercent.load() == 0)
    {
        soundVolumePercent.store(100);
        MarkSoundCacheDirty();
    }
}

bool IsSoundEnabled()
{
    return soundEnabled.load();
}

void SetSoundVolumePercent(int volumePercent)
{
    int clampedVolumePercent = ClampVolume(volumePercent);
    if (soundVolumePercent.load() != clampedVolumePercent)
    {
        StopSoundPlayback();
        MarkSoundCacheDirty();
    }
    soundVolumePercent.store(clampedVolumePercent);
    soundEnabled.store(clampedVolumePercent > 0);
}

int GetSoundVolumePercent()
{
    return soundVolumePercent.load();
}

void PlayMoveSound()
{
    PlayCue(SoundCue::Move, false);
}

void PlaySoftDropSound()
{
    PlayCue(SoundCue::SoftDrop, false);
}

void PlayHardDropSound()
{
    PlayCue(SoundCue::HardDrop, true);
}

void PlayHoldSound()
{
    PlayCue(SoundCue::Hold, false);
}

void PlayRotateSound()
{
    PlayCue(SoundCue::Rotate, false);
}

void PlayLineClearSound(int completedLines, bool spinClear)
{
    PlayCue(GetClearCue(completedLines, spinClear, false, false), true);
}

void PlayClearResultSound(int completedLines, bool spinClear, bool backToBack, bool perfectClear)
{
    PlayCue(GetClearCue(completedLines, spinClear, backToBack, perfectClear), true);
}

void PlayBackToBackSound()
{
    PlayCue(SoundCue::BackToBack, true);
}

void PlayPerfectClearSound()
{
    PlayCue(SoundCue::PerfectClear, true);
}

void PlayLevelUpSound()
{
    PlayCue(SoundCue::LevelUp, false);
}

void PlayPauseSound()
{
    PlayCue(SoundCue::Pause, true);
}

void PlayGameOverSound()
{
    PlayCue(SoundCue::GameOver, true);
}

void ShutdownSound()
{
    StopSoundPlayback();
}
