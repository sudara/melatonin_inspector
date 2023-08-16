#pragma once

#include "colour_property_component.h"
#include "juce_gui_extra/juce_gui_extra.h"
#include "melatonin_inspector/melatonin/components/overlay.h"
#include "preview.h"

namespace melatonin
{
    class RGBAToggle : public juce::Component
    {
    public:
        bool rgba = true;
        std::function<void()> onClick;

        RGBAToggle()
        {
            rgba = settings->props->getBoolValue ("rgba", true);
        }

        void paint (juce::Graphics& g) override
        {
            g.setColour (colors::customBackground);
            g.fillRoundedRectangle (getLocalBounds().withSizeKeepingCentre (38, 16).toFloat(), 3);
            g.setColour (colors::label);
            g.setFont (juce::Font ("Verdana", 9, juce::Font::FontStyleFlags::bold));
            g.drawText (rgba ? "RGBA" : "HEX", getLocalBounds(), juce::Justification::centred);
        }

        void mouseDown (const juce::MouseEvent& /*event*/) override
        {
            rgba = !rgba;
            settings->props->setValue ("rgba", rgba);
            if (onClick)
                onClick();
            repaint();
        }

    private:
        juce::SharedResourcePointer<InspectorSettings> settings;
    };

    class ColorPicker : public juce::Component, private ComponentModel::Listener
    {
    public:
        std::function<void (bool)> togglePickerCallback;

        explicit ColorPicker (ComponentModel& _model, Preview& p) : model (_model), preview (p)
        {
            addAndMakeVisible (colorPickerButton);
            addAndMakeVisible (panel);
            addAndMakeVisible (rgbaToggle);

            selectedColor = juce::Colours::transparentBlack;

            // we overlap the header, so let people click that as usual
            setInterceptsMouseClicks (false, true);

            model.addListener (*this);

            colorPickerButton.onClick = [this]() {
                // hides the text when the picker isn't active
                // uncertain why, but this must be accessed through "this"
                if (this->colorPickerButton.enabled)
                {
                    preview.setVisible (true);

                    // pick an arbitrary first position in the overlay
                    if (root != nullptr)
                        updatePicker ({ root->getX() + 10, root->getY() + 10 });
                }
                else
                {
                    preview.switchToPreview();
                    selectedColor = juce::Colours::transparentBlack;
                }

                // might need to resize the panel if we need to toggle paint timings
                // or there's a change in number of colors
                getParentComponent()->resized();
                repaint();

                // toggle overlay
                if (togglePickerCallback)
                    togglePickerCallback (!colorPickerButton.enabled);
            };

            // sets the color properties with the correct display format
            rgbaToggle.onClick = [this]() { componentModelChanged (model); };
        }

        ~ColorPicker() override
        {
            model.removeListener (*this);
            if (root != nullptr)
                root->removeMouseListener (this);
        }

        void paint (juce::Graphics& g) override
        {
            if (colorPickerButton.enabled)
            {
                g.setColour (colors::black);

                // rect with only bottom corners rounded
                g.fillRect (colorValueBounds.withBottom (4));
                g.fillRoundedRectangle (colorValueBounds.withTrimmedBottom (1).toFloat(), 4);

                g.setColour (colors::text);
                g.setFont (juce::Font ("Verdana", 14.5, juce::Font::FontStyleFlags::plain));
                g.drawText (stringForColor (selectedColor), colorValueBounds.withTrimmedBottom (2), juce::Justification::centred);
            }

            if (model.colors.empty())
            {
                g.setColour (colors::propertyName);
                g.setFont (juce::Font ("Verdana", 15, juce::Font::FontStyleFlags::plain));
                g.drawText ("No Color Properties", panelBounds.withTrimmedLeft (5), juce::Justification::topLeft);
            }
        }

        void resized() override
        {
            auto buttonsArea = getLocalBounds().removeFromTop (32);
            colorPickerButton.setBounds (buttonsArea.removeFromRight (32).translated (2, -4));
            buttonsArea.removeFromRight (12);
            rgbaToggle.setBounds (buttonsArea.removeFromRight (38));

            auto area = getLocalBounds();

            // overlaps with the panel + bit of padding
            colorValueBounds = area.removeFromTop (32).withTrimmedRight (36).withSizeKeepingCentre (rgbaToggle.rgba ? 100 : 90, 32);

            area.removeFromTop (5);
            panelBounds = area;
            if (!model.colors.empty())
            {
                panel.setBounds (panelBounds);
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

            if (colorPickerButton.enabled && selectedColor != juce::Colours::transparentBlack)
            {
                model.pickedColor.setValue ((int) selectedColor.getARGB());
                model.refresh(); // update Last Picked
                colorPickerButton.enabled = false;
                colorPickerButton.onClick();
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
            resized();
        }

        void componentModelChanged (ComponentModel&) override
        {
            panel.clear();
            juce::Array<juce::PropertyComponent*> props;

            for (auto& nv : model.colors)
            {
                auto* prop = new ColourPropertyComponent (nv.value, colors::enumNameIfPresent (nv.name), rgbaToggle.rgba, true);
                if (nv.name == "Last Picked")
                {
                    prop->setColour (juce::PropertyComponent::labelTextColourId, colors::highlight);
                }
                prop->setLookAndFeel (&getLookAndFeel());
                props.add (prop);
            }
            panel.addProperties (props, 5);
            resized();
        }

        // close the picker if we are hidden
        void visibilityChanged() override
        {
            if (!isVisible())
            {
                colorPickerButton.enabled = false;
                colorPickerButton.onClick();
            }
        }

    private:
        ComponentModel& model;
        Preview& preview;

        juce::PropertyPanel panel { "Properties" };
        InspectorImageButton colorPickerButton { "Eyedropper", { 0, 6 }, true };
        juce::Rectangle<int> colorValueBounds;
        juce::Rectangle<int> panelBounds;
        RGBAToggle rgbaToggle;

        juce::Image eyedropperCursorImage = getIcon ("Eyedropperon").rescaled (16, 16);
        juce::MouseCursor eyedropperCursor { eyedropperCursorImage, 0, 15 };

        std::unique_ptr<juce::Image> image;
        juce::Colour selectedColor { juce::Colours::transparentBlack };

        juce::Component* root {};

        void updatePicker (juce::Point<int> point)
        {
            if (!colorPickerButton.enabled)
                return;

            selectedColor = image->getPixelAt (point.x, point.y);

            // we are creating a 20x zoom
            // (for example at the minimum width of 380, it's 19 pixels total)
            int numberOfPixelsWidth = int (preview.getWidth() / preview.zoomScale);

            // add 1 extra pixel to ensure odd number of pixels
            if (numberOfPixelsWidth % 2 == 0)
                numberOfPixelsWidth -= 1;

            // we want 1 extra pixel for bleed on each side
            int extraBleed = 2;

            // a width of 13 pixels results in 7 pixel radius
            // a width of 14 pixels ALSO results in a 7 pixel radius
            int xRadius = (numberOfPixelsWidth + extraBleed - 1) / 2;

            // zoomed, the resulting image will be larger than the preview panel (due to the bleed)
            preview.setZoomedImage (image->getClippedImage ({ point.x - xRadius, point.y - 2, numberOfPixelsWidth + extraBleed, 5 }));
            repaint();
        }

        juce::String stringForColor (juce::Colour& color)
        {
            return rgbaToggle.rgba ? colors::rgbaString (color) : colors::hexString (color);
        }

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ColorPicker)
    };
}
