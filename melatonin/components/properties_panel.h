#pragma once
#include "melatonin_inspector/melatonin/component_model.h"

namespace melatonin
{
    class PropertiesPanel : public CollapsablePanel,
                            private ComponentModel::Listener
    {
    public:
        explicit PropertiesPanel (ComponentModel& _model) : CollapsablePanel ("PROPERTIES"), model (_model)
        {
            reset();

            setContent (&panel);
            model.addListener (*this);
        }

        ~PropertiesPanel() override
        {
            model.removeListener (*this);
        }

        void paint (juce::Graphics& g) override
        {
            g.setColour (colors::panelLineSeparatorColor);
            g.drawHorizontalLine (0, 0, (float) getWidth()); // draw line at top
        }

        void resized() override
        {
            panel.setBounds (propsRectangle());
            paddingHor = 8;
            CollapsablePanel::resized();
        }

    private:
        ComponentModel& model;
        juce::PropertyPanel panel { "Properties" };

        int padding = 5;
        int propsSize = 0;

        [[nodiscard]] juce::Rectangle<int> propsRectangle() const
        {
            // from juce::PropertyPanel
            int preferredHeight = 25;
            return { getWidth(), propsSize + 1 * padding + propsSize * preferredHeight };
        }

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
            auto opaque = new juce::BooleanPropertyComponent (juce::Value (model.opaqueValue), "Opaque:", "");
            opaque->setEnabled (false);

            auto cachedImage = new juce::BooleanPropertyComponent (juce::Value (model.hasCachedImageValue), "CachedToImage:", "");
            cachedImage->setEnabled (false);

            auto focused = new juce::BooleanPropertyComponent (juce::Value (model.focused), "Focused:", "");
            focused->setEnabled (false);

            auto accessibilityHandled = new juce::BooleanPropertyComponent (juce::Value (model.accessibilityHandled), "Accessibility:", "");
            accessibilityHandled->setEnabled (false);

            return {
                new juce::TextPropertyComponent (juce::Value (model.lookAndFeel), "LookAndFeel:", 200, false, false),
                new juce::TextPropertyComponent (model.xValue, "X:", 5, false),
                new juce::TextPropertyComponent (model.yValue, "Y:", 5, false),
                new juce::TextPropertyComponent (model.widthValue, "Width:", 5, false),
                new juce::TextPropertyComponent (model.heightValue, "Height:", 5, false),
                new juce::TextPropertyComponent (juce::Value (model.fontValue), "Font:", 5, false, false),
                new juce::TextPropertyComponent (juce::Value (model.alphaValue), "Alpha:", 5, false, false),
                accessibilityHandled,
                focused,
                opaque,
                cachedImage
            };
        }

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PropertiesPanel)
    };
}
