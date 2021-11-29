#pragma once
#include "helpers.h"

namespace melatonin
{
    class BoxModel : public Component, public Label::Listener, public ComponentListener
    {
    public:
        BoxModel()
        {
            addAndMakeVisible (componentLabel);
            componentLabel.setColour (Label::textColourId, color::blueLabelBackgroundColor);
            componentLabel.setJustificationType (Justification::centredLeft);

            addAndMakeVisible (parentComponentLabel);
            parentComponentLabel.setColour (Label::textColourId, color::redLineColor);
            parentComponentLabel.setJustificationType (Justification::centredLeft);

            addAndMakeVisible (widthLabel);
            widthLabel.setEditable (true);
            widthLabel.addListener (this);
            widthLabel.setFont (20.0f);
            widthLabel.setJustificationType (Justification::centredRight);

            addAndMakeVisible (byLabel);
            byLabel.setText (L" × ", dontSendNotification);
            byLabel.setFont (20.f);
            byLabel.setJustificationType (Justification::centred);

            addAndMakeVisible (heightLabel);
            heightLabel.setEditable (true);
            heightLabel.addListener (this);
            heightLabel.setFont (20.0f);
            heightLabel.setJustificationType (Justification::centredLeft);

            Label* parentLabels[4] = { &topToParentLabel, &rightToParentLabel, &bottomToParentLabel, &leftToParentLabel };

            for (auto parentLabel : parentLabels)
            {
                addAndMakeVisible (parentLabel);
                parentLabel->setText ("-", dontSendNotification);
                parentLabel->setJustificationType (Justification::centred);
                parentLabel->setColour (Label::textColourId, color::redLineColor);
            }
        }

        void paint (Graphics& g) override
        {
            // dashed line rectangles be hard, yo!
            g.setColour (color::redLineColor);
            float dashLengths[2] = { 3.f, 3.f };
            parentRectanglePath.clear();
            parentRectanglePath.addRectangle (parentComponentRectangle());
            auto parentStroke = PathStrokeType (0.5);
            parentStroke.createDashedStroke (parentRectanglePath, parentRectanglePath, dashLengths, 2);
            g.strokePath (parentRectanglePath, parentStroke);

            g.setColour (color::blueLineColor);
            g.drawRect (componentRectangle(), 2.0);
        }

        void resized() override
        {
            auto center = getLocalBounds().getCentre();
            auto labelWidth = 60;
            auto labelHeight = 30;

            parentComponentLabel.setBounds (parentComponentRectangle().getX(), parentComponentRectangle().getY() - labelHeight + 5, parentComponentRectangle().getWidth(), labelHeight);
            componentLabel.setBounds (componentRectangle().getX(), componentRectangle().getY() - labelHeight + 5, componentRectangle().getWidth(), labelHeight);

            widthLabel.setBounds (center.getX() - 70, center.getY() - 15, labelWidth, labelHeight);
            byLabel.setBounds (center.getX() - 10, center.getY() - 15, 20, labelHeight);
            heightLabel.setBounds (center.getX() + 10, center.getY() - 15, labelWidth, labelHeight);

            topToParentLabel.setBounds (center.getX() - labelWidth / 2, padding + paddingToParent / 2 - labelHeight / 2, labelWidth, labelHeight);
            rightToParentLabel.setBounds (getWidth() - padding - paddingToParent / 2 - labelWidth / 2, center.getY() - labelHeight / 2, labelWidth, labelHeight);
            bottomToParentLabel.setBounds (center.getX() - labelWidth / 2, getHeight() - padding - paddingToParent / 2 - labelHeight / 2, labelWidth, labelHeight);
            leftToParentLabel.setBounds (padding + paddingToParent / 2 - labelWidth / 2, center.getY() - labelHeight / 2, labelWidth, labelHeight);
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

        void labelTextChanged (Label* changedLabel) override
        {
            if (changedLabel == &widthLabel || changedLabel == &heightLabel)
            {
                displayedComponent->setSize (widthLabel.getText().getIntValue(), heightLabel.getText().getIntValue());
            }
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
            Label* labels[6] = { &widthLabel, &heightLabel, &topToParentLabel, &rightToParentLabel, &bottomToParentLabel, &leftToParentLabel };

            for (auto label : labels)
            {
                label->setText ("-", dontSendNotification);
            }
            componentLabel.setText ("", dontSendNotification);
            parentComponentLabel.setText ("", dontSendNotification);
        }

    private:
        Component::SafePointer<Component> displayedComponent;

        Label componentLabel;
        Label parentComponentLabel;

        Label widthLabel;
        Label byLabel;
        Label heightLabel;

        Label topToParentLabel;
        Label rightToParentLabel;
        Label bottomToParentLabel;
        Label leftToParentLabel;

        int padding = 30;
        int paddingToParent = 50;
        Path parentRectanglePath; // complicated b/c it's dashed

        Rectangle<int> parentComponentRectangle()
        {
            return getLocalBounds().reduced (padding);
        }

        Rectangle<int> componentRectangle()
        {
            return parentComponentRectangle().reduced (paddingToParent).withTrimmedTop (5);
        }

        void updateLabels()
        {
            auto boundsInParent = displayedComponent->getBoundsInParent();

            parentComponentLabel.setText (componentString (displayedComponent->getParentComponent()), dontSendNotification);
            componentLabel.setText (componentString (displayedComponent), dontSendNotification);

            widthLabel.setText (String (displayedComponent->getWidth()), dontSendNotification);
            heightLabel.setText (String (displayedComponent->getHeight()), dontSendNotification);

            topToParentLabel.setText (String (boundsInParent.getY()), dontSendNotification);
            rightToParentLabel.setText (String (displayedComponent->getParentWidth() - displayedComponent->getWidth() - boundsInParent.getX()), dontSendNotification);
            bottomToParentLabel.setText (String (displayedComponent->getParentHeight() - displayedComponent->getHeight() - boundsInParent.getY()), dontSendNotification);
            leftToParentLabel.setText (String (boundsInParent.getX()), dontSendNotification);
            repaint();
        }
    };
}