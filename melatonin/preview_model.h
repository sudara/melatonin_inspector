#pragma once

#include "helpers.h"
#include "model.h"

namespace melatonin
{

    //create child component of ComponentPreview
    class ComponentPreviewChild : public CollapsablePanel, public ComponentModel::Listener
    {
    public:
        ComponentPreviewChild (ComponentModel& _model) : CollapsablePanel ("PREVIEW"), model (_model)
        {
            setContent (&content);

            model.addListener (*this);
        }

        ~ComponentPreviewChild() override
        {
            model.removeListener (*this);
        }

        void paint (juce::Graphics& g) override
        {
            g.setColour (color::panelLineSeparatorColor);
            g.drawHorizontalLine(0, 0, getWidth()); //draw line at top

            auto bounds = content.getBounds().expanded(2);
            g.setColour (color::blackColor);
            g.fillRect (bounds);


            g.setColour (color::blueLineColor);

            float dashLengths[2] = { 2.f, 2.f };
            parentRectanglePath.clear();
            parentRectanglePath.addRectangle (bounds);
            auto parentStroke = juce::PathStrokeType (0.5);
            parentStroke.createDashedStroke (parentRectanglePath, parentRectanglePath, dashLengths, 2);
            g.strokePath (parentRectanglePath, parentStroke);
        }

        void resized() override
        {
            paddingHor = 32;
            paddingVer = 16;

            content.setSize (getWidth(), 100);
            CollapsablePanel::resized();
        }

        ComponentModel& model;

    private:
        juce::ImageComponent content;
        juce::Path parentRectanglePath;

        void componentChanged (ComponentModel& model) override
        {
            if(auto component = model.getSelectedComponent())
                content.setImage (component->createComponentSnapshot ({ component->getWidth(), component->getHeight() }, false, 1.0f));
        }
    };
}
