#pragma once

#include "colour_property_component.h"
#include "juce_gui_extra/juce_gui_extra.h"
#include "melatonin_inspector/melatonin/components/overlay.h"
#include "preview.h"

namespace melatonin
{

    class ColorPicker : public juce::Component, private ComponentModel::Listener
    {
    public:
        explicit ColorPicker (ComponentModel& _model, Preview& p) : model (_model), preview (p)
        {
            addAndMakeVisible (colorValField);
            addAndMakeVisible (colorPickerButton);
            addAndMakeVisible (&panel);

            colorValField.setJustificationType (juce::Justification::centred);
            colorValField.setFont (juce::Font (13.0f, juce::Font::bold));
            colorValField.setColour (juce::TextEditor::ColourIds::textColourId, colors::customPropertyName);

            selectedColor = juce::Colours::transparentBlack;

            // we overlap the header, so let people click that as usual
            setInterceptsMouseClicks (false, true);

            model.addListener (*this);

            // hide the text when the picker isn't active
            colorPickerButton.onClick = [this]() { if(!colorPickerButton.isEnabled()) colorValField.setVisible(false); };
        }

        ~ColorPicker() override
        {
            model.removeListener (*this);
            if (root != nullptr)
                root->removeMouseListener (this);
        }

        void paint (juce::Graphics& g) override
        {
            g.setColour (colors::panelLineSeparatorColor);
            g.drawHorizontalLine (getHeight() - 1, 0, (float) getWidth()); // separator
        }

        void resized() override
        {
            colorPickerButton.setBounds (getLocalBounds()
                                             .removeFromRight (32)
                                             .removeFromTop (29)
                                             .withSizeKeepingCentre (32, 32));

            auto area = getLocalBounds();

            // overlaps with the panel + bit of padding
            colorValField.setBounds (area.removeFromTop (32).withTrimmedRight (36));

            if (!model.colors.empty())
            {
                area.removeFromTop (5);
                panel.setBounds (area);
            }
        }

        void mouseEnter (const juce::MouseEvent& event) override
        {
            if (root == nullptr)
                return;

            event.eventComponent->setMouseCursor (colorPickerButton.enabled ? eyedropperCursor : juce::MouseCursor::NormalCursor);

            auto rootPos = event.getEventRelativeTo (root).getPosition();

            image = std::make_unique<juce::Image> (root->createComponentSnapshot ({ root->getWidth(), root->getHeight() }, false));

            updatePicker (rootPos);
        }

        void mouseMove (const juce::MouseEvent& event) override
        {
            if (root == nullptr || image == nullptr)
                return;
            auto rootPos = event.getEventRelativeTo (root).getPosition();

            updatePicker (rootPos);
        }

        void mouseExit (const juce::MouseEvent& event) override
        {
            event.eventComponent->setMouseCursor (colorPickerButton.enabled ? eyedropperCursor : juce::MouseCursor::NormalCursor);
        }

        void mouseDown (const juce::MouseEvent& /*event*/) override
        {
            if (root == nullptr || image == nullptr)
                return;

            if (selectedColor != juce::Colours::transparentBlack)
            {
                model.pickedColor.setValue ((int) selectedColor.getARGB());
                getParentComponent()->repaint(); // might need to resize the panel
            }
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
            panel.clear();
            updateLabels();
            resized();
        }

        void componentModelChanged (ComponentModel&) override
        {
            panel.clear();
            juce::Array<juce::PropertyComponent*> props;

            for (auto& nv : model.colors)
            {
                auto* prop = new ColourPropertyComponent (nv.value, colors::enumNameIfPresent (nv.name), true);
                if (nv.name == "Last Picked")
                {
                    prop->setColour (juce::PropertyComponent::labelTextColourId, colors::customPropertyName);
                }
                prop->setLookAndFeel (&getLookAndFeel());
                props.add (prop);
            }
            panel.addProperties (props, 5);
            resized();
        }

    private:
        ComponentModel& model;
        Preview& preview;

        juce::PropertyPanel panel { "Properties" };
        InspectorImageButton colorPickerButton { "Eyedropper", { 0, 6 }, true };
        juce::Label colorValField { "Color value" };

        juce::Image eyedropperCursorImage = getIcon ("Eyedropperon").rescaled (16, 16);
        juce::MouseCursor eyedropperCursor { eyedropperCursorImage, 0, 15 };

        std::unique_ptr<juce::Image> image;
        juce::Colour selectedColor { juce::Colours::transparentBlack };

        juce::Component* root {};

        void updatePicker (juce::Point<int>& point)
        {
            if (!colorPickerButton.enabled)
                return;

            selectedColor = image->getPixelAt (point.x, point.y);

            // we are creating a 20x zoom
            // at minimum width of 380, that's 19 pixels total
            // we want an odd number of zoomed in width/height pixels so our selected pixel is always centered
            int maxNumOfFullHorizontalPixels = int (getWidth() / 20);
            int extraBleed = 2; // 1 extra pixel for bleed on each side
            if (maxNumOfFullHorizontalPixels % 2 == 0)
            {
                maxNumOfFullHorizontalPixels -= 1;
            }
            int xRadius = (maxNumOfFullHorizontalPixels + extraBleed - 1) / 2;
            preview.setVisible (true);
            preview.setZoomedImage (image->getClippedImage ({ point.x - xRadius, point.y - 2, maxNumOfFullHorizontalPixels + extraBleed, 5 }));
            updateLabels();
            colorValField.setVisible (true);
        }

        void updateLabels()
        {
            juce::String rgbaString = juce::String::formatted ("%d, %d, %d",
                selectedColor.getRed(),
                selectedColor.getGreen(),
                selectedColor.getBlue());
            colorValField.setText (rgbaString, juce::dontSendNotification);
            colorValField.setVisible (true);

            resized();
            repaint();
        }

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ColorPicker)
    };
}
