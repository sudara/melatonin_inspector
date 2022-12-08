#pragma once

#include "helpers.h"
#include "juce_gui_extra/juce_gui_extra.h"

namespace melatonin
{

    class ColorModel : public juce::Component, public juce::ComponentListener
    {
    public:
        ColorModel()
        {
            colorField.setColour (juce::Label::ColourIds::textColourId, juce::Colours::white);
            colorValField.setColour (juce::Label::ColourIds::textColourId, juce::Colours::white);

            addChildComponent (colorField);
            addChildComponent (colorValField);
        }

        void paint (juce::Graphics& g) override
        {
            g.setColour (selectedColor);
            g.fillRoundedRectangle (colorSelectorBounds.toFloat(), 2.f);

            if (displayedComponent)
            {
                g.setColour (juce::Colours::lightgrey.withAlpha (0.2f));
                g.drawRect (photoImageBounds.expanded (4));

                g.setColour (juce::Colours::lightgrey);
            }
            if (image)
            {
                g.drawImage (*image, photoImageBounds.toFloat(), juce::RectanglePlacement::centred);
            }
        }

        void resized() override
        {
            auto r = componentRectangle();
            auto labelWidth = juce::jmin (200, r.getWidth() / 2);
            auto labelHeight = 32;

            auto fieldBounds = r.removeFromBottom (labelHeight);
            r.removeFromBottom(4);
            photoImageBounds = r.reduced (2);

            colorField.setBounds (fieldBounds.removeFromLeft (labelWidth));
            colorValField.setBounds (fieldBounds.removeFromLeft (100));

            colorSelectorBounds = fieldBounds.removeFromRight (juce::jmin (fieldBounds.getWidth(), labelHeight))
                                      .withSizeKeepingCentre (juce::jmin (fieldBounds.getWidth(), labelHeight), static_cast<int> (labelHeight * 0.75f));
        }

        void displayComponent (Component* componentToDisplay)
        {
            if (displayedComponent)
            {
                displayedComponent->removeMouseListener (this);
            }

            displayedComponent = componentToDisplay;
            displayedComponent->addMouseListener (this, true);

            image = std::make_unique<juce::Image> (displayedComponent->createComponentSnapshot ({ displayedComponent->getWidth(), displayedComponent->getHeight() }, false, scale));

            selectedColor = image->getPixelAt (displayedComponent->getBounds().getCentreX(),
                displayedComponent->getBounds().getCentreY());
            updateLabels();
        }

        void mouseEnter (const juce::MouseEvent& event) override
        {
            if (displayedComponent == nullptr)
                return;

            auto root = displayedComponent;
            auto rootPos = event.position.toInt();

            const auto w = root->getWidth();
            const auto h = root->getHeight();
            //const auto zoom = 1;

            if (auto disp = juce::Desktop::getInstance().getDisplays().getDisplayForPoint (root->localPointToGlobal (rootPos)))
                scale = float (disp->scale);

            image = std::make_unique<juce::Image> (root->createComponentSnapshot ({ w, h }, false, scale));

            selectedColor = image->getPixelAt (rootPos.x, rootPos.y);
            updateLabels();
        }

        void mouseMove (const juce::MouseEvent& event) override
        {
            if (displayedComponent == nullptr || image == nullptr)
                return;
            auto rootPos = event.position.toInt();

            selectedColor = image->getPixelAt (rootPos.x, rootPos.y);
            updateLabels();
        }

        void mouseExit (const juce::MouseEvent&) override
        {
        }

        void reset()
        {
            colorField.setText ("Color: ", juce::dontSendNotification);
            colorValField.setText ("-", juce::dontSendNotification);
            colorValField.setEditable (false, juce::dontSendNotification);

            colorField.setVisible (false);
            colorValField.setVisible (false);

            colorSelectorBounds = { 0, 0 };
            photoImageBounds = { 0, 0 };

            resized();
        }

    private:
        Component::SafePointer<Component> displayedComponent;

        juce::Label colorField { "Color label", "Color:" };
        juce::Label colorValField { "Color value", "-" };
        juce::Colour selectedColor { juce::Colours::transparentBlack };
        juce::Rectangle<int> colorSelectorBounds, photoImageBounds;

        int padding = 30;
        int paddingToParent = 4;

        std::unique_ptr<juce::Image> image;
        juce::Point<float> point;
        float scale { 1.0 };

        juce::Rectangle<int> parentComponentRectangle()
        {
            return getLocalBounds().reduced (padding, 0);
        }

        juce::Rectangle<int> componentRectangle()
        {
            return parentComponentRectangle().reduced (paddingToParent);
        }

        void updateLabels()
        {
            colorField.setText ("Color:", juce::dontSendNotification);
            juce::String rgbaString = juce::String::formatted("RGBA: (%d, %d, %d, %d)",
                selectedColor.getRed(), selectedColor.getGreen(),
                selectedColor.getBlue(), selectedColor.getAlpha());
            colorValField.setText (rgbaString, juce::dontSendNotification);

            colorField.setVisible (true);
            colorValField.setVisible (true);

            resized();
            repaint();
        }
    };
}
