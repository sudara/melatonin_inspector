#pragma once
#include "melatonin_inspector/melatonin/component_model.h"
#include "colour_property_component.h"

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

        void componentModelChanged (ComponentModel&) override
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
            // we can't actually set these values from the front end, so disable them
            auto cachedImage = new juce::BooleanPropertyComponent (model.hasCachedImageValue, "CachedToImage", "");
            cachedImage->setEnabled (false);

            juce::Array<juce::PropertyComponent*> props =
            {
                new juce::TextPropertyComponent (model.typeValue, "Class", 200, false, false),
                new juce::TextPropertyComponent (model.lookAndFeelValue, "LookAndFeel", 200, false, false),
                new juce::TextPropertyComponent (model.xValue, "X", 5, false),
                new juce::TextPropertyComponent (model.yValue, "Y", 5, false),
                new juce::TextPropertyComponent (model.widthValue, "Width", 5, false),
                new juce::TextPropertyComponent (model.heightValue, "Height", 5, false),
                new juce::TextPropertyComponent (model.fontValue, "Font", 5, false, false),
                new juce::TextPropertyComponent (model.alphaValue, "Alpha", 5, false),
                new juce::BooleanPropertyComponent (model.visibleValue, "Visible", ""),
                new juce::BooleanPropertyComponent (model.enabledValue, "Enabled", ""),
                new juce::BooleanPropertyComponent (model.opaqueValue, "Opaque", ""),
                new juce::BooleanPropertyComponent (model.wantsFocusValue, "Wants Keyboard Focus", ""),
                new juce::BooleanPropertyComponent (model.accessibilityHandledValue, "Accessibility", ""),
                cachedImage,
                new juce::BooleanPropertyComponent (model.interceptsMouseValue, "Intercepts Mouse", ""),
                new juce::BooleanPropertyComponent (model.childrenInterceptsMouseValue, "Children Intercepts", "")
            };

            for (auto& nv : model.namedProperties)
            {
                if (nv.name.startsWith ("jcclr_"))
                    props.add (new ColourPropertyComponent (nv.value, nv.name, true));
                else if (nv.value.getValue().isBool())
                    props.add (new juce::BooleanPropertyComponent (nv.value, nv.name, ""));
                else
                    props.add (new juce::TextPropertyComponent (nv.value, nv.name, 200, false));
            }

            return props;
        }

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Properties)
    };
}
