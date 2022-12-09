#pragma once

#include "helpers.h"

namespace melatonin
{

    class InspectorLookAndFeel : public juce::LookAndFeel_V4
    {
    public:
        InspectorLookAndFeel()
        {
            // often the app overrides this
            setColour (juce::Label::outlineWhenEditingColourId, color::redLineColor);
            setColour (juce::ToggleButton::ColourIds::tickDisabledColourId, color::yellowColor);
            setColour (juce::ToggleButton::ColourIds::textColourId, color::titleTextColor);
            setColour (juce::ToggleButton::ColourIds::tickColourId, color::background);

            setColour (juce::Label::textColourId, color::white);

            setColour (juce::TextEditor::textColourId, color::white);
            setColour (juce::TextEditor::backgroundColourId, juce::Colours::transparentBlack);
            setColour (juce::TextEditor::highlightColourId, color::redLineColor.withAlpha(0.8f));
            setColour (juce::CaretComponent::caretColourId, color::redLineColor);

            setColour (juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
            setColour (juce::ComboBox::outlineColourId, juce::Colours::transparentBlack);


            setColour(juce::PropertyComponent::backgroundColourId, juce::Colours::transparentBlack);
            setColour(juce::TextEditor::outlineColourId, juce::Colours::transparentBlack);
            setColour(juce::TextPropertyComponent::backgroundColourId, juce::Colours::transparentBlack);
            setColour(juce::TextPropertyComponent::outlineColourId, juce::Colours::transparentBlack);
            setColour(juce::BooleanPropertyComponent::backgroundColourId, juce::Colours::transparentBlack);
            setColour(juce::BooleanPropertyComponent::outlineColourId, juce::Colours::transparentBlack);

            setColour(juce::TreeView::ColourIds::selectedItemBackgroundColourId, color::blackColor);
            setColour(juce::TreeView::ColourIds::backgroundColourId, color::backgroundDarkerColor);

            setColour(juce::ScrollBar::ColourIds::thumbColourId, color::bluePropsScrollbar);
        }

        // we don't want our resizer in the overlay to have a fugly border
        void drawResizableFrame (juce::Graphics& g, int w, int h, const juce::BorderSize<int>& border) override
        {
            ignoreUnused (g, w, h, border);
        }

        // For some reason this is actually *needed* which is strange.
        // But we want to adjust the color and size of triangles anyway
        void drawTreeviewPlusMinusBox (juce::Graphics& g, const juce::Rectangle<float>& area, juce::Colour backgroundColour, bool isOpen, bool /*isMouseOver*/) override
        {
            juce::Path p;
            p.addTriangle (0.0f, 0.0f, 1.0f, isOpen ? 0.0f : 0.5f, isOpen ? 0.5f : 0.0f, 1.0f);
            g.setColour (backgroundColour);
            g.fillPath (p, p.getTransformToScaleToFit (area.reduced (0, area.getHeight() / 4).translated (1, 0), true));
        }

        // more friendly scrolling
        int getDefaultScrollbarWidth() override
        {
            return 22;
        }

        // don't use the target app's font
        juce::Font getLabelFont (juce::Label& label) override
        {
            return juce::Font ("Verdana", label.getFont().getHeight(), juce::Font::FontStyleFlags::plain);
        }

        // oh i dream of css resets...
        juce::BorderSize<int> getLabelBorderSize (juce::Label&) override
        {
            return juce::BorderSize<int> (0);
        }

        void drawTickBox (juce::Graphics& g, juce::Component& c, float x, float y, float w, float h, bool isTicked, bool isEnabled, bool isMouseOverButton, bool isButtonDown) override
        {

            juce::Rectangle<float> bounds(x + 2.f, y + 2.f, w - 4.f, h - 4.f);
            if(!isTicked){
                g.setColour(findColour(juce::ToggleButton::ColourIds::textColourId));
                g.drawRoundedRectangle(bounds, 2, 1);
            }
            else
            {
                // Fill the background of the tick box with the specified color
                g.setColour(findColour(juce::ToggleButton::ColourIds::tickDisabledColourId));
                g.fillRoundedRectangle(bounds, 2);
            }


            auto tickBoxSize = juce::jmin (bounds.getWidth(), bounds.getHeight());

            // Draw a transparent check mark if the button is ticked
            if (isTicked)
            {
                juce::Path tickPath;
                tickPath.startNewSubPath (bounds.getX() + 3.0f, bounds.getCentreY());
                tickPath.lineTo (bounds.getCentreX(), bounds.getBottom() - 3.0f);
                tickPath.lineTo (bounds.getRight() - 3.0f, bounds.getY() + 3.0f);

                g.setColour (findColour (juce::ToggleButton::ColourIds::tickColourId));
                g.strokePath (tickPath, juce::PathStrokeType (tickBoxSize * 0.2f));
            }
        }

        void drawTextEditorOutline (juce::Graphics& g, int width, int height, juce::TextEditor& textEditor) override
        {
            if (dynamic_cast<juce::AlertWindow*> (textEditor.getParentComponent()) == nullptr)
            {
                if (textEditor.isEnabled())
                {
                    juce::Rectangle<int> b{0, 0, width, height};
                    if (textEditor.hasKeyboardFocus (true) && ! textEditor.isReadOnly())
                    {
                        g.setColour (textEditor.findColour (juce::TextEditor::focusedOutlineColourId));
                        g.drawRoundedRectangle (b.reduced(1).toFloat(), cornerRadius, 2);
                    }
                    else
                    {
                        g.setColour (textEditor.findColour (juce::TextEditor::outlineColourId));
                        g.drawRoundedRectangle(b.reduced(1).toFloat(), cornerRadius, 1);
                    }
                }
            }
        }

        void drawScrollbar (juce::Graphics& g, juce::ScrollBar& scrollbar, int x, int y, int width, int height, bool isScrollbarVertical, int thumbStartPosition, int thumbSize, bool isMouseOver, bool isMouseDown) override
        {

            //fill bg in black
            g.fillAll(color::blackColor);

            juce::Rectangle<int> thumbBounds;
            if (isScrollbarVertical)
                thumbBounds = juce::Rectangle<int> (x, thumbStartPosition, width, thumbSize);
            else
                thumbBounds = juce::Rectangle<int> (thumbStartPosition, y, thumbSize, height);

            g.setColour (findColour(juce::ScrollBar::ColourIds::thumbColourId));
            g.fillRoundedRectangle(thumbBounds.reduced(5).toFloat(), 2);
        }

    private:
        const float cornerRadius = 4.f;
    };

}
