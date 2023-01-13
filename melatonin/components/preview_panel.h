#pragma once

#include "melatonin_inspector/melatonin/component_model.h"

namespace melatonin
{

    class PreviewPanel : public CollapsablePanel, public ComponentModel::Listener
    {
    public:
        explicit PreviewPanel (ComponentModel& _model) : CollapsablePanel ("PREVIEW"), model (_model)
        {
            setContent (&content);

            model.addListener (*this);
        }

        ~PreviewPanel() override
        {
            model.removeListener (*this);
        }

        void paint (juce::Graphics& g) override
        {
            if(content.isVisible())
                g.fillAll(colors::blackColor);
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

        void componentChanged (ComponentModel&) override
        {
            if(auto component = model.getSelectedComponent())
                content.setImage (component->createComponentSnapshot ({ component->getWidth(), component->getHeight() }, false, 1.0f));
            else
                content.setImage (juce::Image());
        }

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PreviewPanel)
    };
}
