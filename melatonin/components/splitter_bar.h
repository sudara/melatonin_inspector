#pragma once

#include "../helpers/colors.h"
#include <juce_gui_basics/juce_gui_basics.h>

namespace melatonin
{
    // A thin vertical handle that resizes the tree/main-column split.
    // The actual layout logic stays in InspectorComponent via these callbacks.
    class SplitterBar : public juce::Component
    {
    public:
        SplitterBar()
        {
            setOpaque (true);
            setMouseCursor (juce::MouseCursor::LeftRightResizeCursor);
            setRepaintsOnMouseActivity (true);
        }

        // Returns the current width of the component being resized.
        std::function<int()> getTargetWidth;

        // Called with the requested new width while dragging.
        std::function<void (int)> setTargetWidth;

        void paint (juce::Graphics& g) override
        {
            // Blend the tree and main-column backgrounds so the handle reads as
            // a divider rather than a gap in the inspector.
            g.setGradientFill (juce::ColourGradient (colors::treeBackgroundDarker, 0.0f, 0.0f, colors::panelBackgroundDarker, (float) getWidth(), 0.0f, false));
            g.fillAll();

            g.setColour (isMouseOverOrDragging() ? colors::disclosure : colors::panelLineSeparator);
            g.fillRect (getLocalBounds().withWidth (1).withX (getWidth() / 2));
        }

        void mouseDown (const juce::MouseEvent& e) override
        {
            dragStartScreenX = e.getScreenX();
            dragStartWidth = getTargetWidth != nullptr ? getTargetWidth() : 0;
        }

        void mouseDrag (const juce::MouseEvent& e) override
        {
            if (setTargetWidth != nullptr)
                setTargetWidth (dragStartWidth - (e.getScreenX() - dragStartScreenX));
        }

    private:
        int dragStartScreenX = 0;
        int dragStartWidth = 0;
    };
}
