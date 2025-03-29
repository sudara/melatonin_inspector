#pragma once
#include "../helpers/colors.h"
#include "juce_gui_basics/juce_gui_basics.h"

namespace melatonin
{
    class UndoManagerList : public juce::Component, juce::ListBoxModel, juce::ChangeListener
    {
    public:
        UndoManagerList (juce::UndoManager* um) : undoManager (um)
        {
            listBox.setModel (this);
            listBox.setRowHeight (24);
            addAndMakeVisible (listBox);
            undoManager->addChangeListener (this); // listen for changes to the undo manager
            listBox.setColour (juce::ListBox::backgroundColourId, colors::panelBackgroundLighter);
            addAndMakeVisible (clearButton);
            clearButton.onClick = [&]() { undoManager->clearUndoHistory(); };
        }

        ~UndoManagerList()
        {
            undoManager->removeChangeListener (this);
        }

        void paint (juce::Graphics& g) override
        {
            g.fillAll (colors::panelBackgroundDarker);
            g.setColour (colors::label);
            const auto numUndos = undoManager->getUndoDescriptions().size();
            const auto numRedos = undoManager->getRedoDescriptions().size();
            g.drawText (juce::String (numUndos) + " Undos" + " | " + juce::String (numRedos) + " Redos", juce::Rectangle { 40, 40, 200, 20 }, juce::Justification::centredLeft, true);
        }

        void changeListenerCallback (juce::ChangeBroadcaster* source) override
        {
            listBox.updateContent();
            repaint();
        }

        // ListBoxModel implementation
        int getNumRows() override
        {
            return undoManager->getRedoDescriptions().size() + undoManager->getUndoDescriptions().size() + 2; // including headers
        }

        void paintListBoxItem (int rowNumber, juce::Graphics& g, int width, int height, bool /*rowIsSelected*/) override
        {
            auto redos = undoManager->getRedoDescriptions();
            auto undos = undoManager->getUndoDescriptions();
            constexpr int textPadding = 6;

            juce::String text;
            if (rowNumber == 0)
            {
                g.setColour (colors::headerBackground);
                g.fillRect (0, 0, width, height);
                g.setFont (InspectorLookAndFeel::getInspectorFont (14.5, juce::Font::FontStyleFlags::bold).withExtraKerningFactor (0.1f));
                text = "REDO STACK";
                g.setColour (colors::label);
                g.drawText (text, textPadding, 0, width - 2 * textPadding, height, juce::Justification::centredLeft);
            }
            else if (rowNumber <= redos.size())
            {
                g.setFont (InspectorLookAndFeel::getInspectorFont (15, juce::Font::FontStyleFlags::plain));
                g.setColour (colors::text);
                text = redos[rowNumber - 1];
                g.drawText (text, textPadding, 0, width - 2 * textPadding, height, juce::Justification::centredLeft);
            }
            else if (rowNumber == redos.size() + 1)
            {
                g.setColour (colors::headerBackground);
                g.fillRect (0, 0, width, height);
                g.setFont (InspectorLookAndFeel::getInspectorFont (14.5, juce::Font::FontStyleFlags::bold).withExtraKerningFactor (0.1f));
                text = "UNDO STACK";
                g.setColour (colors::label);
                g.drawText (text, textPadding, 0, width - 2 * textPadding, height, juce::Justification::centredLeft);
            }
            else
            {
                g.setFont (InspectorLookAndFeel::getInspectorFont (15, juce::Font::FontStyleFlags::plain));
                g.setColour (colors::text);
                text = undos[rowNumber - redos.size() - 2];
                g.drawText (text, textPadding, 0, width - 2 * textPadding, height, juce::Justification::centredLeft);
            }
        }

        void resized() override
        {
            contentBounds = getLocalBounds().reduced (40);
            clearButton.setBounds(contentBounds.removeFromTop (30).removeFromRight (60));
            listBox.setBounds (contentBounds);
        }

    private:
        juce::ListBox listBox;
        juce::TextButton clearButton { "clear" };
        juce::UndoManager* undoManager; // not owned, passed in via constructor
        juce::Rectangle<int> contentBounds;
    };

    class UndoManagerInspector : public juce::DocumentWindow
    {
    public:
        UndoManagerInspector (juce::UndoManager* um) : DocumentWindow ("Undo Manager Inspector", colors::panelBackgroundDarker, allButtons)
        {
            setContentOwned (new UndoManagerList (um), true);
            setResizable (true, false);
            setSize (400, 600);
            setUsingNativeTitleBar (true);
        }

        void closeButtonPressed() override
        {
            setVisible (false);
        }
    };
}
