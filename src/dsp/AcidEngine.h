#pragma once

#include <array>
#include <cmath>
#include <cstdint>
#include <vector>

#include "../model/PatternBank.h"

namespace acidlab
{
struct Parameters
{
    float waveform = 0.0f;
    float tune = 0.0f;
    float pulseWidth = 0.5f;
    float cutoff = 800.0f;
    float resonance = 0.35f;
    float envMod = 0.55f;
    float decay = 0.25f;
    float accentAmount = 0.55f;
    float slideTime = 0.08f;
    int filterModel = 1;
    int oversampling = 1;
    float filterTracking = 0.25f;
    float bassBoost = 0.25f;
    float accentAttack = 0.35f;
    float clipLevel = 0.85f;
    int distMode = 0;
    float drive = 0.35f;
    float distTone = 0.55f;
    float distMix = 0.45f;
    bool fxDelayOn = false;
    float fxDelayMix = 0.0f;
    float fxDelayTime = 0.25f;
    bool fxModOn = false;
    float fxModDepth = 0.0f;
    bool fxReverbOn = false;
    float fxReverbMix = 0.0f;
    float fxTone = 0.5f;
    float subLevel = 0.25f;
    float noise = 0.015f;
    float drift = 0.08f;
    float volume = 0.75f;
    int patternLength = 16;
    int seqRate = 1;
    bool sequencerEnabled = true;
    double bpm = 120.0;
};

class AcidEngine
{
public:
    void prepare (double newSampleRate);
    void reset();

    void noteOn (int midiNote, float velocity, bool slide, bool accent);
    void noteOff();

    void render (float* left,
                 float* right,
                 int numSamples,
                 const Parameters& parameters,
                 const std::array<Step, maxSteps>& pattern,
                 bool hostPlaying);

    int getCurrentStep() const noexcept { return currentStep; }
    float getOutputLevel() const noexcept { return outputLevel; }
    float getAccentLevel() const noexcept { return accentMeter; }

private:
    struct OnePole
    {
        void reset() noexcept { z = 0.0f; }
        float process (float x, float coefficient) noexcept
        {
            z += coefficient * (x - z);
            return z;
        }

        float z = 0.0f;
    };

    class TptLowpass
    {
    public:
        void prepare (double newSampleRate);
        void reset();
        float process (float input, float cutoffHz, float resonance, float nonlinearDrive);

    private:
        double sampleRate = 44100.0;
        float ic1eq = 0.0f;
        float ic2eq = 0.0f;
    };

    class AcidLadder
    {
    public:
        void reset();
        float process (float input, float cutoffHz, float resonance, float drive, double effectiveSampleRate);

    private:
        std::array<float, 4> stage {};
    };

    class DelayLine
    {
    public:
        void prepare (double newSampleRate);
        void reset();
        float process (float input, float timeSeconds, float feedback, float tone);

    private:
        std::vector<float> buffer;
        double sampleRate = 44100.0;
        int writeIndex = 0;
        OnePole toneFilter;
    };

    static float midiNoteToHz (float midiNote) noexcept;
    static float polyBlep (float phase, float phaseInc) noexcept;
    static float softClip (float x) noexcept;
    static float applyDistortion (float x, int mode) noexcept;
    static float driveCompensation (int mode) noexcept;
    static float fold (float x) noexcept;
    static float clamp (float value, float low, float high) noexcept;

    float nextRandomBipolar() noexcept;
    void triggerStep (const Step& step, const Parameters& parameters);
    float renderSample (const Parameters& parameters);
    float renderVoiceSubSample (const Parameters& parameters, double effectiveSampleRate);
    float applyEffects (float input, const Parameters& parameters);
    int getOversamplingFactor (int choice) const noexcept;
    double getSamplesPerStep (const Parameters& parameters) const noexcept;

    double sampleRate = 44100.0;
    double samplesUntilNextStep = 0.0;
    bool wasHostPlaying = false;
    int currentStep = -1;

    float phase = 0.0f;
    float subPhase = 0.0f;
    float currentFrequency = 110.0f;
    float targetFrequency = 110.0f;
    float activeVelocity = 0.0f;
    float env = 0.0f;
    float amp = 0.0f;
    float accent = 0.0f;
    float accentPulse = 0.0f;
    float outputLevel = 0.0f;
    float accentMeter = 0.0f;
    bool gateOpen = false;
    bool slideActive = false;
    bool externalHeld = false;

    uint32_t randomState = 0x12345678u;
    OnePole toneFilter;
    OnePole bassFilter;
    OnePole fxToneFilter;
    OnePole modFilter;
    TptLowpass filter;
    AcidLadder acidFilter;
    DelayLine delayLine;
    DelayLine reverbLineA;
    DelayLine reverbLineB;
};
} // namespace acidlab
