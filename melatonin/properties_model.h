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
            lnfComponentLabel.setText("LookAndFeel: ", juce::dontSendNotification);
            lnfComponentLabel.setJustificationType (juce::Justification::centredLeft);

            addAndMakeVisible (lnfValLabel);
            lnfValLabel.setColour (juce::Label::textColourId, color::white);
            lnfValLabel.setJustificationType (juce::Justification::centredLeft);

            for(auto* c: getComps())
                addAndMakeVisible(c);

            reset();
        }

        void paint (juce::Graphics& g) override
        {
            //just super simple separator line
            g.setColour (color::greyLineColor);
            g.drawHorizontalLine (padding, padding, getRight() - padding);
        }

        void resized() override
        {
            auto center = getLocalBounds().getCentre();
            auto labelHeight = 30;

            auto lnfLabelWidth = 100;
            lnfComponentLabel.setBounds (lnfRectangle().getX(), lnfRectangle().getY() - labelHeight + 5, lnfLabelWidth, labelHeight);
            lnfValLabel.setBounds (lnfLabelWidth + lnfRectangle().getX(), lnfRectangle().getY() - labelHeight + 5, lnfRectangle().getWidth() - lnfLabelWidth, labelHeight);

            /*xLabel.setBounds (lnfRectangle().getX(), lnfRectangle().getY() - labelHeight + 5, lnfRectangle().getWidth(), labelHeight);
            xValLabel.setBounds (lnfRectangle().getX(), lnfRectangle().getY() - labelHeight + 5, lnfRectangle().getWidth(), labelHeight);

            lnfComponentLabel.setBounds (lnfRectangle().getX(), lnfRectangle().getY() - labelHeight + 5, lnfRectangle().getWidth(), labelHeight);
            lnfComponentLabel.setBounds (lnfRectangle().getX(), lnfRectangle().getY() - labelHeight + 5, lnfRectangle().getWidth(), labelHeight);

            lnfComponentLabel.setBounds (lnfRectangle().getX(), lnfRectangle().getY() - labelHeight + 5, lnfRectangle().getWidth(), labelHeight);
            lnfComponentLabel.setBounds (lnfRectangle().getX(), lnfRectangle().getY() - labelHeight + 5, lnfRectangle().getWidth(), labelHeight);

            lnfComponentLabel.setBounds (lnfRectangle().getX(), lnfRectangle().getY() - labelHeight + 5, lnfRectangle().getWidth(), labelHeight);
            lnfComponentLabel.setBounds (lnfRectangle().getX(), lnfRectangle().getY() - labelHeight + 5, lnfRectangle().getWidth(), labelHeight);

            lnfComponentLabel.setBounds (lnfRectangle().getX(), lnfRectangle().getY() - labelHeight + 5, lnfRectangle().getWidth(), labelHeight);
            lnfComponentLabel.setBounds (lnfRectangle().getX(), lnfRectangle().getY() - labelHeight + 5, lnfRectangle().getWidth(), labelHeight);
            lnfComponentLabel.setBounds (lnfRectangle().getX(), lnfRectangle().getY() - labelHeight + 5, lnfRectangle().getWidth(), labelHeight);
        */}

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
            lnfComponentLabel.setText ("", juce::dontSendNotification);
            lnfValLabel.setText ("", juce::dontSendNotification);
            xValLabel.setText ("", juce::dontSendNotification);
            yValLabel.setText ("", juce::dontSendNotification);
            widthValNameLabel.setText ("", juce::dontSendNotification);
            heightValLabel.setText ("", juce::dontSendNotification);
            focusStateValLabel.setText ("", juce::dontSendNotification);
            isOpaqueToggle.setToggleState(false, juce::dontSendNotification);
            isOpaqueToggle.setEnabled(false);

            alphaValLabel.setText ("", juce::dontSendNotification);
        }

    private:
        Component::SafePointer<Component> displayedComponent;

        juce::Label lnfComponentLabel,
                lnfValLabel;

        juce::Label xLabel,
            xValLabel;

        juce::Label yLabel;
        juce::Label yValLabel;

        juce::Label widthNameLabel;
        juce::Label widthValNameLabel;

        juce::Label heightLabel;
        juce::Label heightValLabel;

        juce::Label focusStateLabel;
        juce::Label focusStateValLabel;

        juce::Label opaqueStateLabel;
        juce::ToggleButton isOpaqueToggle{"opaque state"};

        juce::Label alphaLabel;
        juce::Label alphaValLabel;

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
        juce::Rectangle<int> xyRectangle()
        {
            return parentComponentRectangle().reduced (0, paddingToParent).withTrimmedTop (25);
        }

        void updateLabels()
        {
            lnfComponentLabel.setText("LookAndFeel: ", juce::dontSendNotification);
            lnfValLabel.setText (lnfString (displayedComponent), juce::dontSendNotification);
            repaint();
        }

        std::vector<juce::Component*> getComps(){
            return {
                &lnfComponentLabel, &lnfValLabel,
                &xLabel, &xValLabel,
                &yLabel, &yValLabel,
                &widthNameLabel, &widthValNameLabel,
                &heightLabel, &heightValLabel,
                &focusStateLabel, &focusStateValLabel,
                &opaqueStateLabel, &isOpaqueToggle,
                &alphaLabel, &alphaValLabel
            };
        }
    };
}
