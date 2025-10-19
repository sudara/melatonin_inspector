#pragma once
#include "melatonin_inspector/melatonin/component_model.h"

namespace melatonin
{
    class CustomProperties : public juce::Component,
                             ComponentModel::Listener
    {
    public:
        explicit CustomProperties (ComponentModel& _model) : model (_model)
        {
            reset();
            addAndMakeVisible (&panel);
            model.addListener (*this);
        }

        ~CustomProperties() override
        {
            model.removeListener (*this);
        }

        void resized() override
        {
            TRACE_COMPONENT();
            panel.setBounds (getLocalBounds().withTrimmedTop (padding));
        }

        void reset()
        {
            updateProperties();
            resized();
        }

        // Check if there are any custom properties to display
        [[nodiscard]] bool hasCustomProperties() const
        {
            return !model.namedProperties.empty();
        }

    private:
        ComponentModel& model;
        juce::PropertyPanel panel { "CustomProperties" };
        int padding = 3;

        void componentModelChanged (ComponentModel&) override
        {
            updateProperties();
        }

        void updateProperties()
        {
            TRACE_COMPONENT();
            panel.clear();

            if (!model.getSelectedComponent())
                return;

            auto props = createPropertyComponents();
            for (auto* p : props)
            {
                p->setLookAndFeel (&getLookAndFeel());
            }
            panel.addProperties (props, padding);

            resized();
        }

        [[nodiscard]] juce::Array<juce::PropertyComponent*> createPropertyComponents() const
        {
            TRACE_COMPONENT();
            juce::Array<juce::PropertyComponent*> props;

            // Display all custom properties from namedProperties
            for (auto& nv : model.namedProperties)
            {
                if (nv.value.getValue().isBool())
                {
                    props.add (new juce::BooleanPropertyComponent (nv.value, nv.name, ""));
                }
                else if (nv.value.getValue().isInt64() && nv.name.getLastCharacters (2) == "At")
                {
                    // Handle timestamp properties specially
                    auto datetime = juce::Value (juce::Time (nv.value.getValue()).toString (false, true, true, true));
                    auto datetimeProp = new juce::TextPropertyComponent (datetime, nv.name, 200, false, false);
                    datetimeProp->setEnabled (false);
                    props.add (datetimeProp);
                }
                else
                {
                    const auto value = nv.value.getValue().isObject()
                                           ? juce::Value (nv.value.getValue().toString())
                                           : nv.value;
                    auto customProperty = new juce::TextPropertyComponent (value, nv.name, 200, false);
                    customProperty->getProperties().set ("isUserProperty", true);
                    props.add (customProperty);
                }
            }

            return props;
        }

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CustomProperties)
    };
}
