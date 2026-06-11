#pragma once

#include <juce_core/juce_core.h>

#include <array>

namespace acidlab
{
constexpr int maxSteps = 64;
constexpr int patternSlots = 8;

struct Step
{
    int note = 0;
    int octave = 0;
    bool accent = false;
    bool slide = false;
    bool gate = true;
    bool rest = false;
};

struct PatternBankMetadata
{
    juce::String name = "AcidLab303 Pattern Bank";
    juce::String author = "Zosia Audio";
    juce::String description;
    int scale = 4;
    int lengthChoice = 0;
    int rateChoice = 1;
};

class PatternBank
{
public:
    using Pattern = std::array<Step, maxSteps>;
    using SlotArray = std::array<Pattern, patternSlots>;

    void initializeFactory();
    Step getStep (int slot, int index) const;
    void setStep (int slot, int index, Step step);
    Pattern getPattern (int slot) const;
    void setPattern (int slot, const Pattern& pattern);

    void randomize (int slot, int scale);
    void mutate (int slot, int scale, float amount);
    void clear (int slot);
    void copy (int slot);
    void paste (int slot);

    juce::var toVar() const;
    void fromVar (const juce::var& slots);
    juce::var toPatternBankVar (const PatternBankMetadata& metadata) const;
    bool fromPatternBankVar (const juce::var& root);

    bool saveToFile (const juce::File& file, const PatternBankMetadata& metadata) const;
    bool loadFromFile (const juce::File& file);

    static int constrainScaleNote (int note, int scale);

private:
    static int safeSlot (int slot);
    static int safeStep (int step);

    SlotArray patterns {};
    Pattern clipboard {};
    bool clipboardValid = false;
};
} // namespace acidlab
