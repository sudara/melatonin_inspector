#pragma once
#include "helpers.h"
#include "model.h"

namespace melatonin
{
    class BoxModel : public CollapsablePanel, public juce::Label::Listener, public juce::ComponentListener, public ComponentModel::Listener
    {
    public:
        explicit BoxModel (ComponentModel& componentModel) : CollapsablePanel ("BOX MODEL"), model (componentModel)
        {
            content = std::make_unique<juce::Component>();

            content->addAndMakeVisible (componentLabel);
            componentLabel.setColour (juce::Label::textColourId, color::blueLabelBackgroundColor);
            componentLabel.setJustificationType (juce::Justification::centredLeft);

            content->addAndMakeVisible (parentComponentLabel);
            parentComponentLabel.setColour (juce::Label::textColourId, color::redLineColor);
            parentComponentLabel.setJustificationType (juce::Justification::centredLeft);

            content->addAndMakeVisible (widthLabel);
            widthLabel.addListener (this);
            widthLabel.setFont (20.0f);
            widthLabel.setJustificationType (juce::Justification::centredRight);

            content->addAndMakeVisible (byLabel);
            byLabel.setText (L" × ", juce::dontSendNotification);
            byLabel.setFont (20.f);
            byLabel.setJustificationType (juce::Justification::centred);

            content->addAndMakeVisible (heightLabel);
            heightLabel.addListener (this);
            heightLabel.setFont (20.0f);
            heightLabel.setJustificationType (juce::Justification::centredLeft);

            juce::Label* parentLabels[4] = { &topToParentLabel, &rightToParentLabel, &bottomToParentLabel, &leftToParentLabel };
            juce::Label* paddingLabels[4] = { &paddingTopLabel, &paddingRightLabel, &paddingLeftLabel, &paddingBottomLabel };

            for (auto parentLabel : parentLabels)
            {
                content->addAndMakeVisible (parentLabel);
                parentLabel->setText ("-", juce::dontSendNotification);
                parentLabel->setJustificationType (juce::Justification::centred);
                parentLabel->setColour (juce::Label::textColourId, color::redLineColor);
                parentLabel->addListener (this);

                //centers juce::TextEditor (hack since juce::Label is not doing it by default)
                parentLabel->onEditorShow = [parentLabel] {
                    if (auto editor = parentLabel->getCurrentTextEditor())
                    {
                        auto labelJustification = parentLabel->getJustificationType();
                        if (editor->getJustificationType() != labelJustification)
                        {
                            editor->setJustification (parentLabel->getJustificationType());
                        }
                    }
                };
            }

            for (auto l : paddingLabels)
            {
                content->addChildComponent (l);
                l->setText ("-", juce::dontSendNotification);
                l->setJustificationType (juce::Justification::centred);
                l->setColour (juce::Label::textColourId, color::white);
                l->setColour (juce::Label::backgroundColourId, color::blueLineColor);
                l->setColour (juce::TextEditor::ColourIds::highlightColourId, color::blueLineColor.darker());

                //centers juce::TextEditor (hack since juce::Label is not doing it by default)
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

            model.addListener (*this);
            setContent (content.get());
        }

        ~BoxModel() override
        {
            model.removeListener (*this);
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
            content->setSize (getWidth(), 250);

            CollapsablePanel::resized();


            auto bounds = content->getLocalBounds();
            auto center = bounds.getCentre();
            auto labelWidth = 60;
            auto labelHeight = 30;

            parentComponentLabel.setBounds (bounds.getX(), bounds.getY() - labelHeight + 5, bounds.getWidth(), labelHeight);
            componentLabel.setBounds (componentRectangle().getX(), componentRectangle().getY() - labelHeight + 5, componentRectangle().getWidth(), labelHeight);

            widthLabel.setBounds (center.getX() - 70, center.getY() - 15, labelWidth, labelHeight);
            byLabel.setBounds (center.getX() - 10, center.getY() - 15, 20, labelHeight);
            heightLabel.setBounds (center.getX() + 10, center.getY() - 15, labelWidth, labelHeight);

            topToParentLabel.setBounds (center.getX() - labelWidth / 2, padding + paddingToParent / 2 - labelHeight / 2, labelWidth, labelHeight);
            rightToParentLabel.setBounds (getWidth() - padding - paddingToParent / 2 - labelWidth / 2, center.getY() - labelHeight / 2, labelWidth, labelHeight);
            bottomToParentLabel.setBounds (center.getX() - labelWidth / 2, getHeight() - padding - paddingToParent / 2 - labelHeight / 2, labelWidth, labelHeight);
            leftToParentLabel.setBounds (padding + paddingToParent / 2 - labelWidth / 2, center.getY() - labelHeight / 2, labelWidth, labelHeight);

            auto area1 = bounds.reduced(paddingToParent).removeFromTop (labelHeight);
            paddingTopLabel.setBounds (area1);

            auto area2 = bounds.reduced(paddingToParent).removeFromBottom (labelHeight);
            paddingBottomLabel.setBounds (area2);

            auto area3 = bounds.reduced(paddingToParent).removeFromLeft (labelHeight).withTrimmedTop (labelHeight).withTrimmedBottom (labelHeight);
            paddingLeftLabel.setBounds (area3);

            auto area4 = bounds.reduced(paddingToParent).removeFromRight (labelHeight).withTrimmedTop (labelHeight).withTrimmedBottom (labelHeight);
            paddingRightLabel.setBounds (area4);
        }

        void labelTextChanged (juce::Label* changedLabel) override
        {
            if (changedLabel == &widthLabel || changedLabel == &heightLabel)
            {
                model.getSelectedComponent()->setSize (widthLabel.getText().getIntValue(), heightLabel.getText().getIntValue());
            }
            if (changedLabel == &paddingRightLabel || changedLabel == &paddingLeftLabel
                || changedLabel == &paddingTopLabel || changedLabel == &paddingBottomLabel)
            {
                updateDispayedCompPaddingProperties (paddingRightLabel.getText().getIntValue(), paddingLeftLabel.getText().getIntValue(), paddingTopLabel.getText().getIntValue(), paddingBottomLabel.getText().getIntValue());
            }
            if (changedLabel == &topToParentLabel || changedLabel == &bottomToParentLabel
                || changedLabel == &leftToParentLabel || changedLabel == &rightToParentLabel)
            {
                auto topVal = topToParentLabel.getText().getIntValue();
                auto leftVal = leftToParentLabel.getText().getIntValue();
                auto bottomVal = bottomToParentLabel.getText().getIntValue();
                auto rightVal = rightToParentLabel.getText().getIntValue();
                model.getSelectedComponent()->setTopLeftPosition (leftVal, topVal);
                model.getSelectedComponent()->setSize (model.getSelectedComponent()->getParentWidth() - rightVal - leftVal,
                    model.getSelectedComponent()->getParentHeight() - bottomVal - topVal);
            }
        }

        void reset()
        {
            juce::Label* labels[6] = { &widthLabel, &heightLabel, &topToParentLabel, &rightToParentLabel, &bottomToParentLabel, &leftToParentLabel };

            for (auto label : labels)
            {
                label->setText ("-", juce::dontSendNotification);
                label->setEditable (false);
            }

            juce::Label* paddingLabels[4] = { &paddingTopLabel, &paddingRightLabel, &paddingLeftLabel, &paddingBottomLabel };
            for (auto label : paddingLabels)
            {
                label->setVisible (false);
            }

            componentLabel.setText ("", juce::dontSendNotification);
            parentComponentLabel.setText ("", juce::dontSendNotification);
        }

        void componentChanged (ComponentModel& model) override
        {
            updateLabels();
            updatePaddingLabelsIfNeeded();
        }

    private:
        ComponentModel& model;
        std::unique_ptr<juce::Component> content;

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

        int padding = 8;
        int paddingToParent = 44;
        juce::Path parentRectanglePath; // complicated b/c it's dashed

        juce::Rectangle<int> parentComponentRectangle()
        {
            return content->getBounds().reduced (padding);
        }

        juce::Rectangle<int> componentRectangle()
        {
            return parentComponentRectangle().reduced (paddingToParent);
        }

        void updateLabels()
        {
            if(!model.getSelectedComponent()){
                reset();
                return;
            }
            auto boundsInParent = model.getSelectedComponent()->getBoundsInParent();

            parentComponentLabel.setText (componentString (model.getSelectedComponent()->getParentComponent()), juce::dontSendNotification);
            componentLabel.setText (componentString (model.getSelectedComponent()), juce::dontSendNotification);

            widthLabel.setText (juce::String (model.getSelectedComponent()->getWidth()), juce::dontSendNotification);
            heightLabel.setText (juce::String (model.getSelectedComponent()->getHeight()), juce::dontSendNotification);

            widthLabel.setEditable (true);
            heightLabel.setEditable (true);

            topToParentLabel.setText (juce::String (boundsInParent.getY()), juce::dontSendNotification);
            topToParentLabel.setEditable (true);

            rightToParentLabel.setText (juce::String (model.getSelectedComponent()->getParentWidth() - model.getSelectedComponent()->getWidth() - boundsInParent.getX()), juce::dontSendNotification);
            rightToParentLabel.setEditable (true);

            bottomToParentLabel.setText (juce::String (model.getSelectedComponent()->getParentHeight() - model.getSelectedComponent()->getHeight() - boundsInParent.getY()), juce::dontSendNotification);
            bottomToParentLabel.setEditable (true);

            leftToParentLabel.setText (juce::String (boundsInParent.getX()), juce::dontSendNotification);
            leftToParentLabel.setEditable (true);

            repaint();
        }

        // See Melatonin's PaddedComponent or store this info in your component's getProperties
        void updatePaddingLabelsIfNeeded()
        {
            if (!model.getSelectedComponent())
            {
                //if model.getSelectedComponent() is null, getting props will fail
                juce::Label* paddingLabels[4] = { &paddingTopLabel, &paddingRightLabel, &paddingLeftLabel, &paddingBottomLabel };

                for (auto pl : paddingLabels)
                {
                    pl->setText ("-", juce::dontSendNotification);
                    pl->setEditable (false);
                    pl->removeListener (this);
                }

                return;
            }
            auto component = model.getSelectedComponent();
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
            paddingTopLabel.setVisible (isPaddingComponent);
            paddingBottomLabel.setVisible (isPaddingComponent);
            paddingLeftLabel.setVisible (isPaddingComponent);
            paddingRightLabel.setVisible (isPaddingComponent);

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
            if (model.getSelectedComponent())
            {
                auto& props = model.getSelectedComponent()->getProperties();
                props.set ("paddingLeft", paddingLeft);
                props.set ("paddingTop", paddingTop);
                props.set ("paddingRight", paddingRight);
                props.set ("paddingBottom", paddingBottom);
                model.getSelectedComponent()->resized();
                model.getSelectedComponent()->repaint();
            }
        }
    };
}
