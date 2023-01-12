#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

static inline juce::String dimensionsString (juce::Rectangle<int> bounds)
{
    // account for outline border drawing (1 on each side)
    return juce::String (bounds.getWidth() - 2) + L" × " + juce::String (bounds.getHeight() - 2);
}

static inline juce::String distanceString (juce::Line<float> line)
{
    // account for outline border drawing (1 on each side)
    auto v = line.getLength() + 3;
    return juce::String (v);
}
