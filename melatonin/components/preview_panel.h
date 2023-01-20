#pragma once

#include "melatonin_inspector/melatonin/component_model.h"

namespace melatonin
{

    class Preview : public juce::Component, public ComponentModel::Listener
    {
    public:
        explicit Preview (ComponentModel& _model) : model (_model)
        {
            addAndMakeVisible (previewImage);

            model.addListener (*this);
        }

        ~Preview() override
        {
            model.removeListener (*this);
        }

        void paint (juce::Graphics& g) override
        {
            g.fillAll (colors::blackColor);
        }

        void resized() override
        {
            previewImage.setBounds(getLocalBounds().reduced(32, 16));
        }

        ComponentModel& model;

    private:
        juce::ImageComponent previewImage;
        juce::Path parentRectanglePath;

        void componentChanged (ComponentModel&) override
        {
            if (auto component = model.getSelectedComponent())
                previewImage.setImage (component->createComponentSnapshot ({ component->getWidth(), component->getHeight() }, false, 2.0f));
            else
                previewImage.setImage (juce::Image());
        }

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Preview)
    };
}
