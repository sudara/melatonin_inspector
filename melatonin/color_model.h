#pragma once

#include "helpers.h"
#include "juce_gui_extra/juce_gui_extra.h"

namespace melatonin
{

    class ColorModel : public CollapsablePanel
    {
    public:
        ColorModel(): CollapsablePanel("COLOR")
        {

            setContent (&contentPanel);
        }

        void paint (juce::Graphics& g) override
        {
            g.setColour (selectedColor);
            auto bounds = contentPanel.getBounds().removeFromLeft(32).withSizeKeepingCentre(32, 32).toFloat();
            g.fillRoundedRectangle (bounds, 2.f);
        }

        void resized() override
        {
            contentPanel.setSize(getWidth(), 80);

            CollapsablePanel::resized();

            auto r = contentPanel.getLocalBounds();
            auto labelHeight = 32;

            auto fieldBounds = r.removeFromBottom (labelHeight);
            r.removeFromBottom(4);

            //account for color selector
            fieldBounds.removeFromLeft(juce::jmin (fieldBounds.getWidth(), labelHeight))
                        .toFloat();
            colorValField.setBounds (fieldBounds);
        }

        void mouseEnter (const juce::MouseEvent& event) override
        {
            if (root == nullptr)
                return;

            auto rootPos = event.position.toInt();

            const auto w = root->getWidth();
            const auto h = root->getHeight();

            if (auto disp = juce::Desktop::getInstance().getDisplays().getDisplayForPoint (root->localPointToGlobal (rootPos)))
                scale = float (disp->scale);

            image = std::make_unique<juce::Image> (root->createComponentSnapshot ({ w, h }, false, scale));

            selectedColor = image->getPixelAt (rootPos.x, rootPos.y);
            updateLabels();
        }

        void mouseMove (const juce::MouseEvent& event) override
        {
            if (root == nullptr || image == nullptr)
                return;
            auto rootPos = event.position.toInt();

            selectedColor = image->getPixelAt (rootPos.x, rootPos.y);
            updateLabels();
        }

        void mouseExit (const juce::MouseEvent&) override
        {
            selectedColor = juce::Colours::transparentBlack;
        }

        void setRootComponent (Component* rootComponent)
        {
            if(root != nullptr)
                root->removeMouseListener(this);

             root = rootComponent;

             if (root == nullptr){
                selectedColor = juce::Colours::transparentBlack;
                reset();

                return;
             }

             root->addMouseListener(this, true);

             image = std::make_unique<juce::Image> (root->createComponentSnapshot ({ root->getWidth(), root->getHeight() }, false, scale));
        }

        void reset()
        {
             colorValField.setText ("-", juce::dontSendNotification);
             colorValField.setEditable (false, juce::dontSendNotification);

             resized();
        }

    private:
        juce::Component contentPanel;
        juce::DrawableButton colorPickerButton{ "Color Picker", juce::DrawableButton::ImageFitted };
        juce::Label colorValField { "Color value", "-" };

        std::unique_ptr<juce::Image> image;
        juce::Colour selectedColor { juce::Colours::transparentBlack };

        juce::Component* root{};
        juce::Rectangle<float> colorSelectorBounds;

        int padding = 30;
        int paddingToParent = 4;

        float scale { 1.0 };

        juce::Rectangle<int> parentComponentRectangle()
        {
            return contentPanel.getLocalBounds().reduced (padding, 0);
        }

        juce::Rectangle<int> componentRectangle()
        {
            return parentComponentRectangle().reduced (paddingToParent);
        }

        void updateLabels()
        {
            juce::String rgbaString = juce::String::formatted("RGBA: (%d, %d, %d, %d)",
                selectedColor.getRed(), selectedColor.getGreen(),
                selectedColor.getBlue(), selectedColor.getAlpha());
            colorValField.setText (rgbaString, juce::dontSendNotification);
            colorValField.setVisible (true);

            resized();
            repaint();
        }

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ColorModel)
    };
}
