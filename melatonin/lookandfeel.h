#pragma once

#include "helpers.h"

namespace melatonin
{
    class MelatoninLookAndFeel : public juce::LookAndFeel_V4
    {
    public:
        MelatoninLookAndFeel(){

            setColour (juce::Label::textColourId, color::white);

            setColour (juce::TextEditor::textColourId, color::white);
            setColour (juce::TextEditor::backgroundColourId, juce::Colours::transparentBlack);
            setColour (juce::TextEditor::highlightColourId, color::redLineColor.withAlpha(0.8f));
            setColour (juce::CaretComponent::caretColourId, color::redLineColor);

            setColour (juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
            setColour (juce::ComboBox::outlineColourId, juce::Colours::transparentBlack);
        }

    public:
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

    private:
        const int cornerRadius = 4;
    };

}
