#pragma once
#include "../helpers/misc.h"
#include <juce_gui_basics/juce_gui_basics.h>

namespace melatonin
{
    class InspectorImageButton : public juce::Component
    {
    public:
        std::function<void()> onClick;
        std::function<void()> onDoubleClick;

        explicit InspectorImageButton (const juce::String& filename, juce::Point<int> o = { 0, 0 }) : offset (o)
        {
            image = getIcon (filename);
        }

        void paint (juce::Graphics& g) override
        {
            // Assumes exported at 2x
            g.drawImageTransformed (image, juce::AffineTransform::scale (0.5f).translated (offset).translated ((float) offset.getX(), (float) offset.getY()));
        }

        void mouseDown (const juce::MouseEvent& /*event*/) override
        {
            if (onClick != nullptr)
                onClick();
        }

        void mouseDoubleClick (const juce::MouseEvent& /*event*/) override
        {
            if (onDoubleClick != nullptr)
                onDoubleClick();
        }

    private:
        juce::Image image;
        juce::Point<int> offset;
    };
}
