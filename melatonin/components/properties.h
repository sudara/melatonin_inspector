#pragma once
#include "melatonin_inspector/melatonin/component_model.h"

namespace melatonin
{
    class Properties : public juce::Component, private ComponentModel::Listener
    {
    public:
        explicit Properties (ComponentModel& _model) : model (_model)
        {
            reset();

            addAndMakeVisible (&panel);
            model.addListener (*this);
        }

        ~Properties() override
        {
            model.removeListener (*this);
        }

        void resized() override
        {
            // let the property panel know what total height we need to be
            panel.setBounds (getLocalBounds());
        }

    private:
        ComponentModel& model;
        juce::PropertyPanel panel { "Properties" };

        int padding = 5;
        int propsSize = 0;

        void componentChanged (ComponentModel&) override
        {
            updateProperties();
        }

        void reset()
        {
            panel.clear();

            resized();
        }

        void updateProperties()
        {
            panel.clear();
            auto props = createTextEditors();
            for (auto* p : props)
            {
                p->setLookAndFeel (&getLookAndFeel());
            }
            panel.addProperties (props, padding);

            // don't see any method to get num of properties inside PropertyPanel
            propsSize = props.size();

            resized();
        }

        [[nodiscard]] juce::Array<juce::PropertyComponent*> createTextEditors() const
        {
            auto opaque = new juce::BooleanPropertyComponent (juce::Value (model.opaqueValue), "Opaque", "");
            // we can't actually set these values from the front end, so disable them
            opaque->setEnabled (false);

            auto cachedImage = new juce::BooleanPropertyComponent (juce::Value (model.hasCachedImageValue), "CachedToImage", "");
            cachedImage->setEnabled (false);

            auto focused = new juce::BooleanPropertyComponent (juce::Value (model.focused), "Focused", "");
            focused->setEnabled (false);

            auto accessibilityHandled = new juce::BooleanPropertyComponent (juce::Value (model.accessibilityHandled), "Accessibility", "");
            accessibilityHandled->setEnabled (false);

            auto interceptsMouse = new juce::BooleanPropertyComponent (juce::Value (model.interceptsMouse), "Intercepts Mouse", "");
            interceptsMouse->setEnabled (false);

            auto childrenInterceptsMouse = new juce::BooleanPropertyComponent (juce::Value (model.childrenInterceptsMouse), "Children Intercepts", "");
            childrenInterceptsMouse->setEnabled (false);

            auto enabled = new juce::BooleanPropertyComponent (juce::Value (model.enabled), "Enabled", "");
            enabled->setEnabled (false);

            return {
                new juce::TextPropertyComponent (juce::Value (model.lookAndFeel), "LookAndFeel", 200, false, false),
                new juce::TextPropertyComponent (model.xValue, "X", 5, false),
                new juce::TextPropertyComponent (model.yValue, "Y", 5, false),
                new juce::TextPropertyComponent (model.widthValue, "Width", 5, false),
                new juce::TextPropertyComponent (model.heightValue, "Height", 5, false),
                new juce::TextPropertyComponent (juce::Value (model.fontValue), "Font", 5, false, false),
                new juce::TextPropertyComponent (juce::Value (model.alphaValue), "Alpha", 5, false, false),
                enabled,
                opaque,
                focused,
                accessibilityHandled,
                cachedImage,
                interceptsMouse,
                childrenInterceptsMouse
            };
        }

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Properties)
    };
}
