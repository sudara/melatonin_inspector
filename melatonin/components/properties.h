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
            TRACE_COMPONENT();
            // let the property panel know what total height we need to be
            panel.setBounds (getLocalBounds().withTrimmedTop (padding));
        }

        void reset()
        {
            updateProperties();
            resized();
        }

    private:
        ComponentModel& model;
        juce::PropertyPanel panel { "Properties" };

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

            // we can't actually set these values from the front end, so disable them
            auto hasCachedImage = new juce::BooleanPropertyComponent (model.hasCachedImageValue, "CachedToImage", "");
            hasCachedImage->setEnabled (false);

            // Always have class up top
            juce::Array<juce::PropertyComponent*> props = {
                new juce::TextPropertyComponent (model.typeValue, "Class", 200, false, false),
                new juce::TextPropertyComponent (model.nameValue, "Name", 200, false, false),
            };

            // add class specific properies
            if (dynamic_cast<juce::Button*> (model.getSelectedComponent()))
            {
                props.addArray (juce::Array<juce::PropertyComponent*> {
                    new juce::BooleanPropertyComponent (model.isToggleable, "Is Toggleable", ""),
                    new juce::BooleanPropertyComponent (model.toggleState, "Toggle State", ""),
                    new juce::BooleanPropertyComponent (model.clickTogglesState, "Clicking Toggles State", ""),
                    new juce::TextPropertyComponent (model.radioGroupId, "Radio Group ID", 5, false) });
            }

            // then the rest of the component flags
            props.addArray (juce::Array<juce::PropertyComponent*> {
                new juce::TextPropertyComponent (model.lookAndFeelValue, "LookAndFeel", 200, false, false),
                new juce::BooleanPropertyComponent (model.visibleValue, "Visible", ""),
                new juce::BooleanPropertyComponent (model.enabledValue, "Enabled", ""),
                new juce::TextPropertyComponent (model.alphaValue, "Alpha", 5, false),
                new juce::BooleanPropertyComponent (model.opaqueValue, "Opaque", ""),
                new juce::TextPropertyComponent (model.fontValue, "Font", 5, false, false),
                new juce::BooleanPropertyComponent (model.wantsFocusValue, "Wants Keyboard Focus", ""),
                new juce::BooleanPropertyComponent (model.accessibilityHandledValue, "Accessibility", ""),
                hasCachedImage,
                new juce::BooleanPropertyComponent (model.interceptsMouseValue, "Intercepts Mouse", ""),
                new juce::BooleanPropertyComponent (model.childrenInterceptsMouseValue, "Children Intercepts", "") });

            return props;
        }

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Properties)
    };
}
