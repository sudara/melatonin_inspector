#pragma once
#include "melatonin_inspector/melatonin/component_model.h"

namespace melatonin
{
    class StyleProperties : public juce::Component, private ComponentModel::Listener
    {
    public:
        explicit StyleProperties(ComponentModel& _model) : model(_model)
        {
            reset();

            addAndMakeVisible(&panel);
            model.addListener(*this);
        }

        ~StyleProperties() override
        {
            model.removeListener(*this);
        }

        void resized() override
        {
            TRACE_COMPONENT();
            panel.setBounds(getLocalBounds().withTrimmedTop(padding));
        }

        void reset()
        {
            updateProperties();
            resized();
        }

    private:
        ComponentModel& model;
        juce::PropertyPanel panel{"Style Properties"};
        int padding = 3;

        void componentModelChanged(ComponentModel&) override
        {
            updateProperties();
        }

        void updateProperties()
        {
            TRACE_COMPONENT();
            panel.clear();

            if (!model.getSelectedComponent())
                return;

            auto props = createStylePropertyComponents();
            for (auto* p : props)
            {
                p->setLookAndFeel(&getLookAndFeel());
            }
            panel.addProperties(props, padding);

            resized();
        }

        [[nodiscard]] juce::Array<juce::PropertyComponent*> createStylePropertyComponents() const
        {
            TRACE_COMPONENT();
            juce::Array<juce::PropertyComponent*> props;

            for (const auto& style : model.styles)
            {
                if (!style.options.isEmpty())
                {
                    // For enumerated/choice properties
                    juce::Array<juce::var> optionIndices;
                    for (int i = 0; i < style.options.size(); ++i)
                        optionIndices.add(i);
                    auto choiceComp = new juce::ChoicePropertyComponent(
                        style.value,
                        style.getDisplayName(),
                        style.options,
                        optionIndices);
                    props.add(choiceComp);
                }
                else if (style.value.getValue().isBool())
                {
                    // For boolean properties
                    props.add(new juce::BooleanPropertyComponent(
                        style.value,
                        style.getDisplayName(),
                        ""));
                }
                else
                {
                    // For text/numeric properties
                    props.add(new juce::TextPropertyComponent(
                        style.value,
                        style.getDisplayName(),
                        50,
                        false));
                }
            }

            return props;
        }

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StyleProperties)
    };
}