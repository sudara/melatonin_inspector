#pragma once
#include "juce_graphics/juce_graphics.h"

namespace melatonin
{
    namespace colors
    {
        const juce::Colour headerBackground = juce::Colour::fromRGB (28, 14, 34);

        const juce::Colour panelBackgroundLighter = juce::Colour::fromRGB (45, 22, 55);
        const juce::Colour panelBackgroundDarker = juce::Colour::fromRGB (38, 18, 52);

        // checkbox, active color button, text editor border, etc
        const juce::Colour mainTextColor = juce::Colour::fromRGB (118, 143, 190);
        const juce::Colour highlightColor = juce::Colour::fromRGB (255, 229, 138);

        const juce::Colour searchTextLabelColor = juce::Colour::fromRGB (92, 82, 111);

        const juce::Colour treeBackgroundLighter = juce::Colour::fromRGB (34, 17, 46);
        const juce::Colour treeBackgroundDarker = juce::Colour::fromRGB (24, 10, 35);
        const juce::Colour treeItemSelection = juce::Colours::black;
        const juce::Colour treeItemText = mainTextColor;
        const juce::Colour treeItemTextSelected = juce::Colour::fromRGB (206, 234, 255);
        const juce::Colour treeItemTextDisabled = juce::Colour::fromRGBA (119, 111, 129, 165);
        const juce::Colour treeViewMinusPlusColor = juce::Colour::fromString ("FF776F81");

        const juce::Colour propertyName = mainTextColor;
        const juce::Colour propertyValue = juce::Colour::fromRGB (206, 235, 255);
        const juce::Colour propertyValueWarn = juce::Colour::fromRGB (255, 117, 117);
        const juce::Colour propertyValueError = juce::Colour::fromRGB (255, 51, 51);
        const juce::Colour propertyValueDisabled = treeItemTextDisabled;

        const juce::Colour checkboxCheck = juce::Colour::fromRGB (41, 11, 79);

        const juce::Colour white = juce::Colours::white; // consistency
        const juce::Colour background = juce::Colour::fromString ("FF2B1D32");
        const juce::Colour blueLineColor = juce::Colour::fromString ("FF429DE2");
        const juce::Colour blueLabelBackgroundColor = juce::Colour::fromString ("FF149DF9");
        const juce::Colour redLineColor = juce::Colour::fromString ("FFD4563F");

        const juce::Colour panelLineSeparatorColor = juce::Colour::fromString ("FF0C0713");
        const juce::Colour titleTextColor = juce::Colour::fromString ("FF9ABEFF");
        const juce::Colour blackColor = juce::Colour::fromString ("FF000000");

        const juce::Colour scrollbar = juce::Colour::fromString ("FF9ABEFF");
        const juce::Colour blueTextLabelColor = juce::Colour::fromString ("FF8392AF");
    }
}
