#include "PatternBank.h"

#include <random>

namespace acidlab
{
int PatternBank::safeSlot (int slot)
{
    return juce::jlimit (0, patternSlots - 1, slot);
}

int PatternBank::safeStep (int step)
{
    return juce::jlimit (0, maxSteps - 1, step);
}

void PatternBank::initializeFactory()
{
    const int notes[] = { 0, 0, 7, 0, 3, 0, 10, 0, 0, 12, 7, 0, 5, 3, 0, 10 };
    for (int slot = 0; slot < patternSlots; ++slot)
    {
        for (int i = 0; i < maxSteps; ++i)
        {
            auto& step = patterns[static_cast<size_t> (slot)][static_cast<size_t> (i)];
            const auto shifted = notes[(i + slot * 2) % 16];
            step.note = shifted % 12;
            step.octave = shifted >= 12 ? 1 : 0;
            step.accent = i % 8 == 0 || (i + slot) % 13 == 0;
            step.slide = (i + slot) % 7 == 3;
            step.gate = true;
            step.rest = (i + slot) % 11 == 6;
        }
    }
}

Step PatternBank::getStep (int slot, int index) const
{
    return patterns[static_cast<size_t> (safeSlot (slot))][static_cast<size_t> (safeStep (index))];
}

void PatternBank::setStep (int slot, int index, Step step)
{
    patterns[static_cast<size_t> (safeSlot (slot))][static_cast<size_t> (safeStep (index))] = step;
}

PatternBank::Pattern PatternBank::getPattern (int slot) const
{
    return patterns[static_cast<size_t> (safeSlot (slot))];
}

void PatternBank::setPattern (int slot, const Pattern& pattern)
{
    patterns[static_cast<size_t> (safeSlot (slot))] = pattern;
}

void PatternBank::randomize (int slot, int scale)
{
    std::mt19937 rng { std::random_device{}() };
    std::uniform_int_distribution<int> noteDistribution { 0, 11 };
    std::uniform_int_distribution<int> octaveDistribution { -1, 1 };
    std::bernoulli_distribution accentDistribution { 0.22 };
    std::bernoulli_distribution slideDistribution { 0.18 };
    std::bernoulli_distribution restDistribution { 0.14 };

    auto& pattern = patterns[static_cast<size_t> (safeSlot (slot))];
    for (auto& step : pattern)
    {
        step.note = constrainScaleNote (noteDistribution (rng), scale);
        step.octave = octaveDistribution (rng);
        step.accent = accentDistribution (rng);
        step.slide = slideDistribution (rng);
        step.rest = restDistribution (rng);
        step.gate = ! step.rest;
    }
}

void PatternBank::mutate (int slot, int scale, float amount)
{
    std::mt19937 rng { std::random_device{}() };
    std::uniform_int_distribution<int> noteDistribution { 0, 11 };
    std::uniform_int_distribution<int> octaveNudge { -1, 1 };
    std::bernoulli_distribution mutate { juce::jlimit (0.02, 0.75, static_cast<double> (amount)) };

    auto& pattern = patterns[static_cast<size_t> (safeSlot (slot))];
    for (auto& step : pattern)
    {
        if (mutate (rng))
            step.note = constrainScaleNote (noteDistribution (rng), scale);
        if (mutate (rng) && amount > 0.45f)
            step.octave = juce::jlimit (-2, 2, step.octave + octaveNudge (rng));
        if (mutate (rng))
            step.accent = ! step.accent;
        if (mutate (rng))
            step.slide = ! step.slide;
        if (mutate (rng) && amount > 0.3f)
        {
            step.rest = ! step.rest;
            step.gate = ! step.rest;
        }
    }
}

void PatternBank::clear (int slot)
{
    auto& pattern = patterns[static_cast<size_t> (safeSlot (slot))];
    for (auto& step : pattern)
    {
        step = {};
        step.gate = false;
        step.rest = true;
    }
}

void PatternBank::copy (int slot)
{
    clipboard = patterns[static_cast<size_t> (safeSlot (slot))];
    clipboardValid = true;
}

void PatternBank::paste (int slot)
{
    if (clipboardValid)
        patterns[static_cast<size_t> (safeSlot (slot))] = clipboard;
}

juce::var PatternBank::toVar() const
{
    juce::Array<juce::var> slots;
    for (int slot = 0; slot < patternSlots; ++slot)
    {
        juce::Array<juce::var> steps;
        for (int i = 0; i < maxSteps; ++i)
        {
            const auto& step = patterns[static_cast<size_t> (slot)][static_cast<size_t> (i)];
            auto object = new juce::DynamicObject();
            object->setProperty ("note", step.note);
            object->setProperty ("octave", step.octave);
            object->setProperty ("accent", step.accent);
            object->setProperty ("slide", step.slide);
            object->setProperty ("gate", step.gate);
            object->setProperty ("rest", step.rest);
            steps.add (juce::var (object));
        }
        slots.add (steps);
    }
    return slots;
}

void PatternBank::fromVar (const juce::var& slots)
{
    if (! slots.isArray())
        return;

    auto* slotArray = slots.getArray();
    for (int slot = 0; slot < juce::jmin (slotArray->size(), patternSlots); ++slot)
    {
        if (! slotArray->getReference (slot).isArray())
            continue;

        auto* stepArray = slotArray->getReference (slot).getArray();
        for (int i = 0; i < juce::jmin (stepArray->size(), maxSteps); ++i)
        {
            auto* object = stepArray->getReference (i).getDynamicObject();
            if (object == nullptr)
                continue;

            auto& step = patterns[static_cast<size_t> (slot)][static_cast<size_t> (i)];
            step.note = juce::jlimit (0, 11, static_cast<int> (object->getProperty ("note")));
            step.octave = juce::jlimit (-2, 2, static_cast<int> (object->getProperty ("octave")));
            step.accent = static_cast<bool> (object->getProperty ("accent"));
            step.slide = static_cast<bool> (object->getProperty ("slide"));
            step.gate = static_cast<bool> (object->getProperty ("gate"));
            step.rest = static_cast<bool> (object->getProperty ("rest"));
        }
    }
}

juce::var PatternBank::toPatternBankVar (const PatternBankMetadata& metadata) const
{
    auto root = new juce::DynamicObject();
    auto meta = new juce::DynamicObject();
    auto settings = new juce::DynamicObject();

    root->setProperty ("format", "AcidLab303PatternBank");
    root->setProperty ("version", 1);
    meta->setProperty ("name", metadata.name);
    meta->setProperty ("author", metadata.author);
    meta->setProperty ("description", metadata.description);
    settings->setProperty ("scale", metadata.scale);
    settings->setProperty ("length", metadata.lengthChoice);
    settings->setProperty ("rate", metadata.rateChoice);
    root->setProperty ("meta", juce::var (meta));
    root->setProperty ("settings", juce::var (settings));
    root->setProperty ("patterns", toVar());
    return juce::var (root);
}

bool PatternBank::fromPatternBankVar (const juce::var& rootVar)
{
    auto* root = rootVar.getDynamicObject();
    if (root == nullptr || root->getProperty ("format").toString() != "AcidLab303PatternBank")
        return false;

    fromVar (root->getProperty ("patterns"));
    return true;
}

bool PatternBank::saveToFile (const juce::File& file, const PatternBankMetadata& metadata) const
{
    file.getParentDirectory().createDirectory();
    return file.replaceWithText (juce::JSON::toString (toPatternBankVar (metadata), true));
}

bool PatternBank::loadFromFile (const juce::File& file)
{
    return fromPatternBankVar (juce::JSON::parse (file));
}

int PatternBank::constrainScaleNote (int note, int scale)
{
    static constexpr int chromatic[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 };
    static constexpr int minor[] = { 0, 2, 3, 5, 7, 8, 10 };
    static constexpr int major[] = { 0, 2, 4, 5, 7, 9, 11 };
    static constexpr int phrygian[] = { 0, 1, 3, 5, 7, 8, 10 };
    static constexpr int acid[] = { 0, 3, 5, 7, 10 };

    const int* notes = chromatic;
    int count = 12;
    switch (scale)
    {
        case 1: notes = minor; count = 7; break;
        case 2: notes = major; count = 7; break;
        case 3: notes = phrygian; count = 7; break;
        case 4: notes = acid; count = 5; break;
        default: break;
    }

    return notes[juce::jlimit (0, count - 1, std::abs (note) % count)];
}
} // namespace acidlab
