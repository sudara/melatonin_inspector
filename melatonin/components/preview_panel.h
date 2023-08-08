#pragma once

#include "melatonin_inspector/melatonin/component_model.h"

namespace melatonin
{

    class Preview : public juce::Component, public ComponentModel::Listener
    {
    public:
        explicit Preview (ComponentModel& _model) : model (_model)
        {
            addAndMakeVisible (previewImage);

            model.addListener (*this);
        }

        ~Preview() override
        {
            model.removeListener (*this);
        }

        void paint (juce::Graphics& g) override
        {
            g.fillAll (colors::blackColor);
            g.setFont (g.getCurrentFont().withHeight (15.0f));
            if (hasPerformanceTiming())
            {
                double exclusiveSum = (double) model.timing1.getValue() + (double) model.timing2.getValue() + (double) model.timing3.getValue();
                bool hasExclusive = exclusiveSum * 1000 * 1000 > 1; // at least 1 microsecond
                bool hasChildren = model.hasChildren.getValue();

                auto exclusiveBounds = performanceBounds.withHeight (25);

                g.setColour (hasExclusive ? colors::propertyName : colors::propertyValueDisabled);
                g.drawText ("Exclusive", exclusiveBounds.removeFromLeft (100), juce::Justification::topLeft);
                drawTimingText (g, exclusiveBounds.removeFromLeft (60), model.timing1.getValue(), !hasExclusive);
                drawTimingText (g, exclusiveBounds.removeFromLeft (60), model.timing2.getValue(), !hasExclusive);
                drawTimingText (g, exclusiveBounds.removeFromLeft (60), model.timing3.getValue(), !hasExclusive);

                auto withChildrenBounds = performanceBounds.withTop (125);

                g.setColour (hasChildren ? colors::propertyName : colors::propertyValueDisabled);
                g.drawText ("With Children", withChildrenBounds.removeFromLeft (100), juce::Justification::topLeft);
                drawTimingText (g, withChildrenBounds.removeFromLeft (60), model.timingWithChildren1, !hasChildren);
                drawTimingText (g, withChildrenBounds.removeFromLeft (60), model.timingWithChildren2, !hasChildren);
                drawTimingText (g, withChildrenBounds.removeFromLeft (60), model.timingWithChildren3, !hasChildren);
            }
        }

        [[nodiscard]] bool hasPerformanceTiming() const
        {
            return model.timing1.getValue().isDouble();
        }

        static void drawTimingText (juce::Graphics& g, juce::Rectangle<int> bounds, double value, bool disabled = false)
        {
            auto text = timingWithUnits (disabled ? 0 : value);

            auto ms = value * 1000;
            if (disabled || ms * 1000 < 1)
                g.setColour (colors::propertyValueDisabled);
            else if (ms > 3)
                g.setColour (colors::propertyValueWarn);
            else if (ms > 8)
                g.setColour (colors::propertyValueError);
            else
                g.setColour (colors::propertyValue);

            g.drawText (text, bounds, juce::Justification::topLeft);
        }

        static juce::String timingWithUnits (double value)
        {
            double ms = value * 1000;
            if (ms * 1000 < 1)
                return "-";
            else if (ms < 1)
                return juce::String (ms * 1000, 1).dropLastCharacters (2) + juce::String (juce::CharPointer_UTF8 ("\xc2\xb5")) + "s"; // µs
            else
                return juce::String (ms, 1) + "ms";
        }

        void resized() override
        {
            auto area = getLocalBounds();
            if (hasPerformanceTiming())
                performanceBounds = area.removeFromBottom (50).withLeft (32);
            else
                performanceBounds = juce::Rectangle<int>();

            previewImage.setBounds (area.reduced (32, 16));
        }

        ComponentModel& model;

    private:
        juce::ImageComponent previewImage;
        juce::Path parentRectanglePath;
        juce::Rectangle<int> performanceBounds;

        void componentModelChanged (ComponentModel&) override
        {
            if (auto component = model.getSelectedComponent())
                previewImage.setImage (component->createComponentSnapshot ({ component->getWidth(), component->getHeight() }, false, 2.0f));
            else
                previewImage.setImage (juce::Image());
        }

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Preview)
    };
}
