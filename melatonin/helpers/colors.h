#pragma once
#include "juce_graphics/juce_graphics.h"

namespace colors
{

    const juce::Colour headerBackground = juce::Colour::fromRGB (28, 14, 34);

    const juce::Colour panelBackgroundLighter = juce::Colour::fromRGB (45, 22, 55);
    const juce::Colour panelBackgroundDarker = juce::Colour::fromRGB (38, 18, 52);

    const juce::Colour searchTextLabelColor = juce::Colour::fromString ("FF5F526F");

    const juce::Colour treeBackgroundLighter = juce::Colour::fromRGB (34, 17, 46);
    const juce::Colour treeBackgroundDarker = juce::Colour::fromRGB (24, 10, 35);
    const juce::Colour treeItemText = juce::Colour::fromRGB (118, 143, 190);
    const juce::Colour treeItemTextSelected = juce::Colour::fromRGB (206, 234, 255);
    const juce::Colour treeItemTextDisabled = juce::Colour::fromRGBA (119, 111, 129, 165);

    const juce::Colour treeViewMinusPlusColor = juce::Colour::fromString ("FF776F81");

    const juce::Colour white = juce::Colours::white; // consistency
    const juce::Colour background = juce::Colour::fromString ("FF2B1D32");
    const juce::Colour blueLineColor = juce::Colour::fromString ("FF429DE2");
    const juce::Colour blueLabelBackgroundColor = juce::Colour::fromString ("FF149DF9");
    const juce::Colour blueLabelTextColor = juce::Colour::fromString ("FFF4FDFF");
    const juce::Colour redLineColor = juce::Colour::fromString ("FFD4563F");
    const juce::Colour redLabelBackgroundColor = juce::Colour::fromString ("FFF54514");
    const juce::Colour greyLineColor = juce::Colours::lightgrey;

    const juce::Colour panelLineSeparatorColor = juce::Colour::fromString ("FF0C0713");
    const juce::Colour titleTextColor = juce::Colour::fromString ("FF9ABEFF");
    const juce::Colour yellowColor = juce::Colour::fromString ("FFFFE58A");
    const juce::Colour backgroundDarkerColor = juce::Colours::black.withAlpha (0.38f);
    const juce::Colour blackColor = juce::Colour::fromString ("FF000000");

    const juce::Colour bluePropsScrollbarColor = juce::Colour::fromString ("FF9ABEFF");
    const juce::Colour blueTextLabelColor = juce::Colour::fromString ("FF8392AF");
}
