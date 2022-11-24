#pragma once
#include "helpers.h"

namespace melatonin
{
    class PropertiesModel : public juce::Component, public juce::Label::Listener, public juce::ComponentListener
    {
    public:
        PropertiesModel()
        {
            addAndMakeVisible (lnfComponentLabel);
            lnfComponentLabel.setColour (juce::Label::textColourId, color::white);
            lnfComponentLabel.setJustificationType (juce::Justification::centredLeft);

            reset();
        }

        void paint (juce::Graphics& g) override
        {
            //just super simple separator line
            g.setColour (color::greyLineColor);
            g.drawHorizontalLine(padding, padding, getRight() - padding);
        }

        void resized() override
        {
            auto center = getLocalBounds().getCentre();
            auto labelHeight = 30;

            lnfComponentLabel.setBounds (lnfRectangle().getX(), lnfRectangle().getY() - labelHeight + 5, lnfRectangle().getWidth(), labelHeight);
        }

        void displayComponent (Component* componentToDisplay)
        {
            if (displayedComponent)
            {
                displayedComponent->removeComponentListener (this);
            }

            displayedComponent = componentToDisplay;
            displayedComponent->addComponentListener (this);

            updateLabels();
        }

        void labelTextChanged (juce::Label* changedLabel) override
        {

        }

        // A selected component has been dragged or resized and this is our callback
        void componentMovedOrResized (Component& /*component*/, bool /*wasMoved*/, bool wasResized) override
        {
            if (wasResized)
            {
                updateLabels();
            }
        }

        void reset()
        {
            lnfComponentLabel.setText ("LookAndFeel: ", juce::dontSendNotification);
        }

    private:
        Component::SafePointer<Component> displayedComponent;

        juce::Label lnfComponentLabel;

        int padding = 30;
        int paddingToParent = 24;

        juce::Rectangle<int> parentComponentRectangle()
        {
            return getLocalBounds().reduced (padding);
        }

        juce::Rectangle<int> lnfRectangle()
        {
            return parentComponentRectangle().reduced (0, paddingToParent).withTrimmedTop (5);
        }

        void updateLabels()
        {
            lnfComponentLabel.setText ("LookAndFeel: " + lnfString(displayedComponent), juce::dontSendNotification);
            repaint();
        }
    };
}
