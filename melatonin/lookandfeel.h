#pragma once

#include "helpers/colors.h"

namespace melatonin
{

    class InspectorLookAndFeel : public juce::LookAndFeel_V4
    {
    public:
        InspectorLookAndFeel()
        {
            // often the app overrides this
            setColour (juce::Label::outlineWhenEditingColourId, colors::highlightColor);

            setColour (juce::Label::textColourId, colors::blueTextLabelColor);

            setColour (juce::ToggleButton::ColourIds::textColourId, colors::mainTextColor);
            setColour (juce::ToggleButton::ColourIds::tickDisabledColourId, colors::mainTextColor);
            setColour (juce::ToggleButton::ColourIds::tickColourId, colors::highlightColor);

            setColour (juce::TextEditor::textColourId, colors::blueTextLabelColor);
            setColour (juce::TextEditor::backgroundColourId, juce::Colours::transparentBlack);
            setColour (juce::TextEditor::highlightColourId, juce::Colour::fromString ("FF373737"));
            setColour (juce::CaretComponent::caretColourId, colors::blueLineColor);

            setColour (juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
            setColour (juce::ComboBox::outlineColourId, juce::Colours::transparentBlack);

            setColour (juce::PropertyComponent::backgroundColourId, juce::Colours::transparentBlack);
            setColour (juce::TextEditor::outlineColourId, juce::Colours::transparentBlack);

            setColour (juce::TextPropertyComponent::backgroundColourId, juce::Colours::transparentBlack);
            setColour (juce::TextPropertyComponent::outlineColourId, juce::Colours::transparentBlack);
            setColour (juce::TextPropertyComponent::textColourId, colors::propertyValue);
            setColour (juce::PropertyComponent::ColourIds::labelTextColourId, colors::propertyName);

            setColour (juce::BooleanPropertyComponent::backgroundColourId, juce::Colours::transparentBlack);
            setColour (juce::BooleanPropertyComponent::outlineColourId, juce::Colours::transparentBlack);

            setColour (juce::TreeView::ColourIds::selectedItemBackgroundColourId, colors::blackColor);
            setColour (juce::TreeView::ColourIds::backgroundColourId, juce::Colours::transparentBlack);
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
            auto tickBounds = area;
            tickBounds.reduce (0, 2);
            auto boxSize = juce::jmin (tickBounds.getHeight(), tickBounds.getWidth());

            juce::Path p;
            p.addTriangle (tickBounds.getX() + 1, tickBounds.getY() + boxSize * 0.5f, tickBounds.getX() + boxSize + 1, tickBounds.getY() + boxSize * 0.5f, tickBounds.getX() + boxSize * 0.5f + 1, tickBounds.getY() + boxSize + boxSize * 0.25f);

            g.setColour (backgroundColour);

            auto transform = juce::AffineTransform::rotation (!isOpen ? juce::degreesToRadians (270.0f) : 0,
                tickBounds.getCentreX(),
                tickBounds.getCentreY());

            // nothing in JUCE's widget library is properly aligned...
            transform = transform.translated (0, 1.5f);

            g.fillPath (p, transform);
        }

        // more friendly scrolling
        int getDefaultScrollbarWidth() override
        {
            return 22;
        }

        // don't use the target app's font
        juce::Font getLabelFont (juce::Label& label) override
        {
            return { "Verdana", label.getFont().getHeight(), juce::Font::FontStyleFlags::plain };
        }

        // oh i dream of css resets...
        juce::BorderSize<int> getLabelBorderSize (juce::Label&) override
        {
            return juce::BorderSize<int> (0);
        }

        // the main "enable inspector" checkbox
        void drawToggleButton (juce::Graphics& g, juce::ToggleButton& button, bool /*shouldDrawButtonAsHighlighted*/, bool /*shouldDrawButtonAsDown*/) override
        {
            float toggleWidth = 14;
            float leftPadding = 6;

            juce::Rectangle<float> bounds (leftPadding, (float) button.getHeight() / 2 - toggleWidth / 2.f, toggleWidth, toggleWidth);

            if (button.getToggleState())
            {
                g.setColour (colors::highlightColor);
                g.fillRoundedRectangle (bounds, 1.5f);
            }
            else
            {
                g.setColour (colors::mainTextColor);
                g.drawRoundedRectangle (bounds, 1, 1.5f);
            }

            if (button.getToggleState())
            {
                juce::Path tickPath;
                tickPath.startNewSubPath (bounds.getX() + 3.5f, bounds.getCentreY() + 1.0f);
                tickPath.lineTo (bounds.getCentreX() - 1.0f, bounds.getBottom() - 4.0f);
                tickPath.lineTo (bounds.getRight() - 3.5f, bounds.getY() + 4.0f);

                g.setColour (colors::checkboxCheck);
                g.strokePath (tickPath, juce::PathStrokeType (2.0f, juce::PathStrokeType::mitered, juce::PathStrokeType::rounded));
            }

            g.setColour (button.findColour (button.getToggleState() ? juce::TextButton::textColourOnId : juce::TextButton::textColourOffId));

            g.setFont (g.getCurrentFont().withPointHeight (13));

            g.drawText (button.getButtonText(),
                button.getLocalBounds().withTrimmedLeft (juce::roundToInt (toggleWidth) + (int) leftPadding + 12).withTrimmedRight (2),
                juce::Justification::centredLeft,
                false);
        }

        void drawTextEditorOutline (juce::Graphics& g, int width, int height, juce::TextEditor& textEditor) override
        {
            if (dynamic_cast<juce::AlertWindow*> (textEditor.getParentComponent()) == nullptr)
            {
                if (textEditor.isEnabled())
                {
                    juce::Rectangle<int> b { 0, 0, width, height };
                    if (textEditor.hasKeyboardFocus (true) && !textEditor.isReadOnly())
                    {
                        g.setColour (textEditor.findColour (juce::TextEditor::focusedOutlineColourId));
                        g.drawRoundedRectangle (b.reduced (1).toFloat(), 0, 1);
                    }
                    else
                    {
                        g.setColour (textEditor.findColour (juce::TextEditor::outlineColourId));
                        g.drawRoundedRectangle (b.reduced (1).toFloat(), 0, 1);
                    }
                }
            }
        }

        void drawScrollbar (juce::Graphics& g, juce::ScrollBar&, int x, int y, int width, int height, bool isScrollbarVertical, int thumbStartPosition, int thumbSize, bool, bool) override
        {
            // fill bg in black
            g.fillAll (colors::blackColor);

            juce::Rectangle<int> thumbBounds;
            if (isScrollbarVertical)
                thumbBounds = juce::Rectangle<int> (x, thumbStartPosition, width, thumbSize);
            else
                thumbBounds = juce::Rectangle<int> (thumbStartPosition, y, thumbSize, height);

            g.setColour (colors::scrollbar);
            g.fillRoundedRectangle (thumbBounds.reduced (5).toFloat(), 2);
        }
    };

}
