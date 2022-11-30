#pragma once
#include "helpers.h"

namespace melatonin
{
    class BoxModel : public juce::Component, public juce::Label::Listener, public juce::ComponentListener
    {
    public:
        BoxModel()
        {
            addAndMakeVisible (componentLabel);
            componentLabel.setColour (juce::Label::textColourId, color::blueLabelBackgroundColor);
            componentLabel.setJustificationType (juce::Justification::centredLeft);

            addAndMakeVisible (parentComponentLabel);
            parentComponentLabel.setColour (juce::Label::textColourId, color::redLineColor);
            parentComponentLabel.setJustificationType (juce::Justification::centredLeft);

            addAndMakeVisible (widthLabel);
            widthLabel.setEditable (true);
            widthLabel.addListener (this);
            widthLabel.setFont (20.0f);
            widthLabel.setJustificationType (juce::Justification::centredRight);

            addAndMakeVisible (byLabel);
            byLabel.setText (L" × ", juce::dontSendNotification);
            byLabel.setFont (20.f);
            byLabel.setJustificationType (juce::Justification::centred);

            addAndMakeVisible (heightLabel);
            heightLabel.setEditable (true);
            heightLabel.addListener (this);
            heightLabel.setFont (20.0f);
            heightLabel.setJustificationType (juce::Justification::centredLeft);

            juce::Label* parentLabels[4] = { &topToParentLabel, &rightToParentLabel, &bottomToParentLabel, &leftToParentLabel };
            juce::Label* paddingLabels[4] = { &paddingTopLabel, &paddingRightLabel, &paddingLeftLabel, &paddingBottomLabel };

            for (auto parentLabel : parentLabels)
            {
                addAndMakeVisible (parentLabel);
                parentLabel->setText ("-", juce::dontSendNotification);
                parentLabel->setJustificationType (juce::Justification::centred);
                parentLabel->setColour (juce::Label::textColourId, color::redLineColor);
            }

            for (auto l : paddingLabels)
            {
                addChildComponent(l);
                l->setText ("-", juce::dontSendNotification);
                l->setJustificationType (juce::Justification::centred);
                l->setColour (juce::Label::textColourId, color::white);
                l->setColour (juce::Label::backgroundColourId, color::blueLineColor);
                l->setColour (juce::TextEditor::ColourIds::highlightColourId, color::blueLineColor.darker());

                l->onEditorShow = [l] {
                    if (auto editor = l->getCurrentTextEditor())
                    {
                        auto labelJustification = l->getJustificationType();
                        if (editor->getJustificationType() != labelJustification)
                        {
                            editor->setJustification (l->getJustificationType());
                        }
                    }
                };

                l->onEditorHide = [l] {
                    auto text = l->getText (true);
                    if (text.getIntValue() == 0)
                        l->setText ("0", juce::dontSendNotification);
                };
            }
        }

        ~BoxModel() override{
            if(displayedComponent)
                displayedComponent->getTopLevelComponent()->removeComponentListener(this);
        }

        void paint (juce::Graphics& g) override
        {
            // dashed line rectangles be hard, yo!
            g.setColour (color::redLineColor);
            float dashLengths[2] = { 3.f, 3.f };
            parentRectanglePath.clear();
            parentRectanglePath.addRectangle (parentComponentRectangle());
            auto parentStroke = juce::PathStrokeType (0.5);
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

            auto area1 = componentRectangle().removeFromTop (labelHeight);
            paddingTopLabel.setBounds (area1);

            auto area2 = componentRectangle().removeFromBottom (labelHeight);
            paddingBottomLabel.setBounds (area2);

            auto area3 = componentRectangle().removeFromLeft (labelHeight).withTrimmedTop (labelHeight).withTrimmedBottom (labelHeight);
            paddingLeftLabel.setBounds (area3);

            auto area4 = componentRectangle().removeFromRight (labelHeight).withTrimmedTop (labelHeight).withTrimmedBottom (labelHeight);
            paddingRightLabel.setBounds (area4);
        }

        void displayComponent (Component* componentToDisplay)
        {
            if (displayedComponent)
            {
                displayedComponent->removeComponentListener (this);
            }

            displayedComponent = componentToDisplay;
            displayedComponent->addComponentListener (this);

            displayedComponent->getTopLevelComponent()->addComponentListener(this);

            updateLabels();
            updatePaddingLabelsIfNeeded();
        }

        void labelTextChanged (juce::Label* changedLabel) override
        {
            if (changedLabel == &widthLabel || changedLabel == &heightLabel)
            {
                displayedComponent->setSize (widthLabel.getText().getIntValue(), heightLabel.getText().getIntValue());
            }
            if (changedLabel == &paddingRightLabel || changedLabel == &paddingLeftLabel
                || changedLabel == &paddingTopLabel || changedLabel == &paddingBottomLabel)
            {
                updateDispayedCompPaddingProperties (paddingRightLabel.getText().getIntValue(), paddingLeftLabel.getText().getIntValue(), paddingTopLabel.getText().getIntValue(), paddingBottomLabel.getText().getIntValue());
            }
        }

        // A selected component has been dragged or resized and this is our callback
        void componentMovedOrResized (Component& /*component*/, bool /*wasMoved*/, bool wasResized) override
        {
            if (wasResized)
            {
                updateLabels();
                updatePaddingLabelsIfNeeded();
            }
        }

        void reset()
        {
            juce::Label* labels[6] = { &widthLabel, &heightLabel, &topToParentLabel, &rightToParentLabel, &bottomToParentLabel, &leftToParentLabel };

            for (auto label : labels)
            {
                label->setText ("-", juce::dontSendNotification);
            }

            juce::Label* paddingLabels[4] = { &paddingTopLabel, &paddingRightLabel, &paddingLeftLabel, &paddingBottomLabel };
            for (auto label : paddingLabels)
            {
                label->setVisible(false);
            }

            componentLabel.setText ("", juce::dontSendNotification);
            parentComponentLabel.setText ("", juce::dontSendNotification);
        }

    private:
        Component::SafePointer<Component> displayedComponent;

        juce::Label componentLabel;
        juce::Label parentComponentLabel;

        juce::Label widthLabel;
        juce::Label byLabel;
        juce::Label heightLabel;

        juce::Label topToParentLabel;
        juce::Label rightToParentLabel;
        juce::Label bottomToParentLabel;
        juce::Label leftToParentLabel;

        juce::Label paddingTopLabel,
            paddingRightLabel,
            paddingBottomLabel,
            paddingLeftLabel;

        int padding = 30;
        int paddingToParent = 44;
        juce::Path parentRectanglePath; // complicated b/c it's dashed

        juce::Rectangle<int> parentComponentRectangle()
        {
            return getLocalBounds().reduced (padding);
        }

        juce::Rectangle<int> componentRectangle()
        {
            return parentComponentRectangle().reduced (paddingToParent).withTrimmedTop (5);
        }

        void updateLabels()
        {
            auto boundsInParent = displayedComponent->getBoundsInParent();

            parentComponentLabel.setText (componentString (displayedComponent->getParentComponent()), juce::dontSendNotification);
            componentLabel.setText (componentString (displayedComponent), juce::dontSendNotification);

            widthLabel.setText (juce::String (displayedComponent->getWidth()), juce::dontSendNotification);
            heightLabel.setText (juce::String (displayedComponent->getHeight()), juce::dontSendNotification);

            topToParentLabel.setText (juce::String (boundsInParent.getY()), juce::dontSendNotification);
            rightToParentLabel.setText (juce::String (displayedComponent->getParentWidth() - displayedComponent->getWidth() - boundsInParent.getX()), juce::dontSendNotification);
            bottomToParentLabel.setText (juce::String (displayedComponent->getParentHeight() - displayedComponent->getHeight() - boundsInParent.getY()), juce::dontSendNotification);
            leftToParentLabel.setText (juce::String (boundsInParent.getX()), juce::dontSendNotification);
            repaint();
        }

        // See Melatonin's PaddedComponent or store this info in your component's getProperties
        void updatePaddingLabelsIfNeeded()
        {
            if (!displayedComponent)
            {
                //if displayedComponent is null, getting props will fail
                juce::Label* paddingLabels[4] = { &paddingTopLabel, &paddingRightLabel, &paddingLeftLabel, &paddingBottomLabel };

                for (auto pl : paddingLabels)
                {
                    pl->setText ("-", juce::dontSendNotification);
                    pl->removeListener (this);
                }

                return;
            }
            auto component = displayedComponent.getComponent();
            auto props = component->getProperties();
            auto hasTopPadding = props.contains ("paddingTop");
            auto hasBottomPadding = props.contains ("paddingBottom");
            auto hasLeftPadding = props.contains ("paddingLeft");
            auto hasRightPadding = props.contains ("paddingRight");

            int paddingTop = props["paddingTop"];
            int paddingBottom = props["paddingBottom"];
            int paddingLeft = props["paddingLeft"];
            int paddingRight = props["paddingRight"];

            bool isPaddingComponent = hasBottomPadding || hasTopPadding || hasLeftPadding || hasRightPadding;
            paddingTopLabel.setVisible(isPaddingComponent);
            paddingBottomLabel.setVisible(isPaddingComponent);
            paddingLeftLabel.setVisible(isPaddingComponent);
            paddingRightLabel.setVisible(isPaddingComponent);

            paddingTopLabel.setText (hasTopPadding ? juce::String (paddingTop) : "-", juce::dontSendNotification);
            paddingTopLabel.setEditable (hasTopPadding);
            paddingTopLabel.addListener (this);

            paddingBottomLabel.setText (hasBottomPadding ? juce::String (paddingBottom) : "-", juce::dontSendNotification);
            paddingBottomLabel.setEditable (hasBottomPadding);
            paddingBottomLabel.addListener (this);

            paddingLeftLabel.setText (hasLeftPadding ? juce::String (paddingLeft) : "-", juce::dontSendNotification);
            paddingLeftLabel.setEditable (hasLeftPadding);
            paddingLeftLabel.addListener (this);

            paddingRightLabel.setText (hasRightPadding ? juce::String (paddingRight) : "-", juce::dontSendNotification);
            paddingRightLabel.setEditable (hasRightPadding);
            paddingRightLabel.addListener (this);
        }

        void updateDispayedCompPaddingProperties (double paddingRight, double paddingLeft, double paddingTop, double paddingBottom)
        {
            if (displayedComponent)
            {
                auto& props = displayedComponent->getProperties();
                props.set ("paddingLeft", paddingLeft);
                props.set ("paddingTop", paddingTop);
                props.set ("paddingRight", paddingRight);
                props.set ("paddingBottom", paddingBottom);
                displayedComponent->resized();
                displayedComponent->repaint();
            }
        }
    };
}
