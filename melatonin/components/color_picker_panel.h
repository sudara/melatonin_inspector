#pragma once

#include "juce_gui_extra/juce_gui_extra.h"
#include "melatonin_inspector/melatonin/components/overlay.h"

namespace melatonin
{

    class ColorPickerPanel : public CollapsablePanel
    {
    public:
        ColorPickerPanel() : CollapsablePanel ("COLOR")
        {
            contentPanel.addAndMakeVisible (colorValField);
            addAndMakeVisible (colorPickerButton);

            colorPickerButton.setClickingTogglesState (true);
            selectedColor = juce::Colours::black;

            setContent (&contentPanel);
        }

        void paint (juce::Graphics& g) override
        {
            auto bounds = contentPanel.getBounds().removeFromLeft (18).withSizeKeepingCentre (18, 18).toFloat();

            g.setColour (selectedColor);
            g.fillRoundedRectangle (bounds, 2.f);

            if (selectedColor.isTransparent())
            {
                g.setColour (colors::titleTextColor);
                g.drawRoundedRectangle (bounds, 2.f, 1.f);
            }
        }

        void resized() override
        {
            paddingHor = 16;
            contentPanel.setSize (getWidth(), 38);

            CollapsablePanel::resized();

            auto r = contentPanel.getLocalBounds();

            auto fieldBounds = r.removeFromTop (18);
            // r.removeFromBottom(4);

            // account for color selector
            fieldBounds.removeFromLeft (fieldBounds.getHeight())
                .toFloat();

            colorValField.setBounds (fieldBounds.withTrimmedLeft (8));
            colorPickerButton.setBounds (getLocalBounds()
                                             .removeFromRight (48)
                                             .removeFromTop (48)
                                             .withSizeKeepingCentre (32, 32));
        }

        void mouseEnter (const juce::MouseEvent& event) override
        {
            if (root == nullptr)
                return;

            auto rootPos = event.getEventRelativeTo (root).getPosition();

            image = std::make_unique<juce::Image> (root->createComponentSnapshot ({ root->getWidth(), root->getHeight() }, false));
            if (colorPickerButton.getToggleState())
            {
                selectedColor = image->getPixelAt (rootPos.x, rootPos.y);
                updateLabels();
            }

            root->setMouseCursor (colorPickerButton.getToggleState() ? juce::MouseCursor::CrosshairCursor : juce::MouseCursor::NormalCursor);
        }

        void mouseMove (const juce::MouseEvent& event) override
        {
            if (root == nullptr || image == nullptr)
                return;
            auto rootPos = event.getEventRelativeTo (root).getPosition();

            if (colorPickerButton.getToggleState())
            {
                selectedColor = image->getPixelAt (rootPos.x, rootPos.y);
                updateLabels();
            }
        }

        void mouseExit (const juce::MouseEvent&) override
        {
            root->setMouseCursor (colorPickerButton.getToggleState() ? juce::MouseCursor::CrosshairCursor : juce::MouseCursor::NormalCursor);
        }

        void setRootComponent (Component* rootComponent)
        {
            if (root != nullptr)
                root->removeMouseListener (this);

            root = rootComponent;

            if (root == nullptr)
            {
                selectedColor = juce::Colours::transparentBlack;
                reset();

                return;
            }

            root->addMouseListener (this, true);
            image = std::make_unique<juce::Image> (root->createComponentSnapshot ({ root->getWidth(), root->getHeight() }, false));

            reset();
        }

        void reset()
        {
            updateLabels();
            resized();
        }

    private:
        juce::Component contentPanel;
        juce::TextButton colorPickerButton {
            "P",
            "Color Picker",
        }; // juce::DrawableButton::ImageFitted };
        juce::Label colorValField {
            "Color value",
        };

        std::unique_ptr<juce::Image> image;
        juce::Colour selectedColor { juce::Colours::transparentBlack };

        juce::Component* root {};

        void updateLabels()
        {
            juce::String rgbaString = juce::String::formatted ("%d, %d, %d, %d",
                selectedColor.getRed(),
                selectedColor.getGreen(),
                selectedColor.getBlue(),
                selectedColor.getAlpha());
            colorValField.setText (rgbaString, juce::dontSendNotification);
            colorValField.setVisible (true);

            resized();
            repaint();
        }

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ColorPickerPanel)
    };
}
