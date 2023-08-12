#pragma once

namespace melatonin
{
    //==============================================================================*/
    class ColourPropertyComponent : public juce::PropertyComponent,
                                    private juce::Value::Listener
    {
    public:
        ColourPropertyComponent (const juce::Value& valueToControl, const juce::String& propertyName, bool showAlpha = false)
            : juce::PropertyComponent (propertyName), value (valueToControl), container (value, showAlpha)
        {
            addAndMakeVisible (container);
            value.addListener (this);
        }

        juce::Value& getValueObject()
        {
            return value;
        }

        void refresh() override
        {
            repaint();
        }

        void paint (juce::Graphics& g) override
        {
            PropertyComponent::paint (g);

            g.setColour (findColour (juce::BooleanPropertyComponent::backgroundColourId));
            g.fillRect (container.getBounds());

            g.setColour (findColour (juce::BooleanPropertyComponent::outlineColourId));
            g.drawRect (container.getBounds());
        }

    private:
        void valueChanged (juce::Value&) override
        {
            refresh();
        }

        juce::Value value;

        class ColourSelectorEx : public juce::ColourSelector,
                                 private juce::ChangeListener
        {
        public:
            ColourSelectorEx (int flags = (showAlphaChannel | showColourAtTop | showSliders | showColourspace),
                int edgeGap = 4,
                int gapAroundColourSpaceComponent = 7)
                : juce::ColourSelector (flags, edgeGap, gapAroundColourSpaceComponent)
            {
                addChangeListener (this);
            }

            ~ColourSelectorEx() override
            {
                if (onDismiss)
                    onDismiss();
            }

            void changeListenerCallback (juce::ChangeBroadcaster*) override
            {
                if (onChange)
                    onChange();
            }

            std::function<void()> onDismiss;
            std::function<void()> onChange;
        };

        class Container : public Component
        {
        public:
            Container (juce::Value& value_, bool a)
                : value (value_), alpha (a)
            {
            }

            void paint (juce::Graphics& g) override
            {
                auto c = juce::Colour ((uint32_t) int (value.getValue()));

                auto area = getLocalBounds();

                g.setColour (c);
                g.fillRoundedRectangle (area.removeFromLeft (18).withHeight (18).toFloat(), 1.f);

                area.removeFromLeft (8);
                g.setColour (colors::propertyValue);
                g.drawText (c.toString(), area.withTrimmedBottom(1), juce::Justification::centredLeft);
            }

            void mouseUp (const juce::MouseEvent& e) override
            {
                if (e.mouseWasClicked())
                {
                    auto flags = juce::ColourSelector::showColourAtTop | juce::ColourSelector::showSliders | juce::ColourSelector::showColourspace;
                    if (alpha)
                        flags |= juce::ColourSelector::showAlphaChannel;

                    auto colourSelector = std::make_unique<ColourSelectorEx> (flags);

                    colourSelector->setLookAndFeel (&getLookAndFeel());
                    colourSelector->setSize (300, 280);
                    colourSelector->setCurrentColour (juce::Colour ((uint32_t) int (value.getValue())), juce::dontSendNotification);
                    colourSelector->onDismiss = [this, cs = colourSelector.get()]() {
                        value = (int) cs->getCurrentColour().getARGB();
                        repaint();
                    };
                    colourSelector->onChange = [this, cs = colourSelector.get()]() {
                        value = (int) cs->getCurrentColour().getARGB();
                        repaint();
                    };

                    juce::CallOutBox::launchAsynchronously (std::move (colourSelector), getScreenBounds(), nullptr);
                }
            }

            juce::Value& value;
            bool alpha;
        };

        Container container;
    };
}
