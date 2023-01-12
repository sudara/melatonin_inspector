#pragma once
#include "juce_graphics/juce_graphics.h"

namespace colors
{
    const juce::Colour white = juce::Colours::white; // consistency
    const juce::Colour background = juce::Colour::fromString ("FF2B1D32");
    const juce::Colour blueLineColor = juce::Colour::fromString ("FF429DE2");
    const juce::Colour blueLabelBackgroundColor = juce::Colour::fromString ("FF149DF9");
    const juce::Colour blueLabelTextColor = juce::Colour::fromString ("FFF4FDFF");
    const juce::Colour redLineColor = juce::Colour::fromString ("FFD4563F");
    const juce::Colour redLabelBackgroundColor = juce::Colour::fromString ("FFF54514");
    const juce::Colour greyLineColor = juce::Colours::lightgrey;

    const juce::Colour panelLineSeparatorColor = juce::Colour::fromString("FF0C0713");
    const juce::Colour titleTextColor = juce::Colour::fromString("FF9ABEFF");
    const juce::Colour yellowColor = juce::Colour::fromString("FFFFE58A");
    const juce::Colour backgroundDarkerColor = juce::Colours::black.withAlpha(0.38f);
    const juce::Colour blackColor = juce::Colour::fromString("FF000000");

    const juce::Colour bluePropsScrollbarColor = juce::Colour::fromString("FF9ABEFF");
    const juce::Colour treeViewMinusPlusColor = juce::Colour::fromString("FF776F81");
    const juce::Colour blueTextLabelColor = juce::Colour::fromString("FF8392AF");
    const juce::Colour searchTextLabelColor = juce::Colour::fromString("FF5F526F");
}
