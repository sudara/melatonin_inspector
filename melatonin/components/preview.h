#pragma once

#include "melatonin_inspector/melatonin/component_model.h"

namespace melatonin
{

    class Preview : public juce::Component, public ComponentModel::Listener
    {
    public:
        explicit Preview (ComponentModel& _model) : model (_model)
        {
            model.addListener (*this);
        }

        ~Preview() override
        {
            model.removeListener (*this);
        }

        void paint (juce::Graphics& g) override
        {
            g.fillAll (colors::black);
            if (zoom)
            {
                // lets see them pixels!
                g.setImageResamplingQuality (juce::Graphics::ResamplingQuality::lowResamplingQuality);

                // place image in center, zoomed 20x
                int numberOfVerticalLines = getWidth() / 20;
                int offsetX = (getWidth() - numberOfVerticalLines * 20) / 2;
                g.drawImageTransformed (previewImage, juce::AffineTransform::scale (20.f, 20.f).translated ((float) offsetX, 0));

                // draw grid
                g.setColour (juce::Colours::grey.withAlpha (0.3f));
                for (auto i = 0; i < getHeight() / 20; i++)
                    g.drawHorizontalLine (i * 20, 0, (float) getWidth());
                for (auto i = 0; i < numberOfVerticalLines; i++)
                    g.drawVerticalLine (offsetX + i * 20, 0, (float) getHeight());

                // highlight the center pixel in both black and white boxes
                g.setColour (juce::Colours::black);
                int highlightedPixelX = offsetX + numberOfVerticalLines / 2 * 20;
                g.drawRect (highlightedPixelX, getHeight() / 2 - 10, 20, 20);
                g.setColour (juce::Colours::white);
                g.drawRect (highlightedPixelX - 2, getHeight() / 2 - 12, 24, 24, 2);
            }
            else
            {
                g.setImageResamplingQuality (juce::Graphics::ResamplingQuality::highResamplingQuality);
                g.drawImage (previewImage, getLocalBounds().reduced (32, 16).toFloat(), juce::RectanglePlacement::centred);
            }
        }

        // called by color picker
        void setZoomedImage (const juce::Image& image)
        {
            previewImage = image;
            zoom = true;
            repaint();
        }

        ComponentModel& model;

    private:
        juce::Image previewImage;
        bool zoom = false;
        juce::Path parentRectanglePath;

        void componentModelChanged (ComponentModel&) override
        {
            if (auto component = model.getSelectedComponent())
                previewImage = component->createComponentSnapshot ({ component->getWidth(), component->getHeight() }, false, 2.0f);
            else
                previewImage = juce::Image();

            zoom = false;
        }

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Preview)
    };
}
