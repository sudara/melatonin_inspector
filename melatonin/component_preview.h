#pragma once

#include "helpers.h"
#include "model.h"

namespace melatonin
{

    //create child component of ComponentPreview
    class ComponentPreviewChild : public CollapsablePanel
    {
    public:
        ComponentPreviewChild (ComponentModel& _model) : CollapsablePanel ("PREVIEW"), model (_model)
        {
            content = std::make_unique<juce::TextButton> ("Click me");
            setContent (content.get());
        }

        void paint (juce::Graphics& g) override
        {
            g.setColour (color::panelLineSeparatorColor);
            g.drawHorizontalLine(0, 0, getWidth()); //draw line at top
        }

        void resized() override
        {
            content->setSize (100, 40);
            CollapsablePanel::resized();
        }

        ComponentModel& model;

    private:
        std::unique_ptr<juce::Component> content;
    };
}
