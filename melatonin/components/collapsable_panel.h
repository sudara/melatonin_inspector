#pragma once
#include "../helpers/colors.h"
#include <juce_gui_basics/juce_gui_basics.h>

// create component with toggle button to show/hide content below including component size change
class CollapsablePanel : public juce::Component
{
public:
    explicit CollapsablePanel (const juce::String& name)
    {
        toggleButton.setLookAndFeel (&toggleButtonLookAndFeel);
        addAndMakeVisible (toggleButton);
        toggleButton.setButtonText (name);

        toggleButton.onClick = [this] {
            if (content)
                content->setVisible (toggleButton.getToggleState());

            getParentComponent()->resized();
        };
    }

    void setContent (juce::Component* _content)
    {
        content = _content;
        addAndMakeVisible (content);

        toggleButton.setToggleState (content != nullptr, juce::dontSendNotification);
    }

    void paint (juce::Graphics& g) override
    {
        g.setColour (colors::background);
        g.drawRect (getLocalBounds(), 1);
    }

    // important to call parent (this) resized() from child resized() after content size is set
    void resized() override
    {
        juce::Rectangle<int> r (0, 8, getWidth(), 32);
        auto buttonHeight = 32;

        if (content && content->isVisible())
        {
            // content->resized();
            r = r.withHeight (content->getHeight() + buttonHeight);

            auto r1 = r;
            content->setBounds (r1.removeFromBottom (content->getHeight())
                                    .reduced (paddingHor, paddingVer));
        }
        auto tbBounds = r;

        auto font = juce::Font ("Verdana", (float) buttonHeight, juce::Font::FontStyleFlags::plain);
        auto btnWidth = font.getStringWidth (toggleButton.getButtonText());
        toggleButton.setBounds (tbBounds
                                    .withX (8)
                                    .removeFromTop (buttonHeight)
                                    .withWidth (btnWidth));

        setSize (r.getWidth(), r.getHeight() + 8);
    }

    int paddingHor = 32;
    int paddingVer = 8;

private:
    struct ToggleButtonLnF : juce::LookAndFeel_V4
    {
        ToggleButtonLnF()
        {
            setColour (juce::ToggleButton::textColourId, colors::titleTextColor);
            setColour (juce::ToggleButton::tickColourId, colors::titleTextColor);
        }

        // override function for drawing toggle btn
        void drawToggleButton (juce::Graphics& g, juce::ToggleButton& button, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
        {
            auto font = juce::Font ("Verdana", juce::jmin (15.0f, (float) button.getHeight() * 0.75f), juce::Font::FontStyleFlags::plain);
            auto tickWidth = font.getHeight();

            drawTickBox (g, button, 4.0f, ((float) button.getHeight() - tickWidth) * 0.5f, tickWidth, tickWidth, button.getToggleState(), button.isEnabled(), shouldDrawButtonAsHighlighted, shouldDrawButtonAsDown);

            g.setColour (button.findColour (juce::ToggleButton::textColourId));
            g.setFont (font);

            if (!button.isEnabled())
                g.setOpacity (0.5f);

            g.drawFittedText (button.getButtonText(),
                button.getLocalBounds().withTrimmedLeft (juce::roundToInt (tickWidth) + 10).withTrimmedRight (2),
                juce::Justification::centredLeft,
                10);
        }

        // override function for drawing tick box
        void drawTickBox (juce::Graphics& g, juce::Component& /*component*/, float x, float y, float w, float h, const bool ticked, const bool isEnabled, const bool /*shouldDrawButtonAsHighlighted*/, const bool /*shouldDrawButtonAsDown*/) override
        {
            juce::Rectangle<float> tickBounds (x, y, w, h);

            tickBounds.reduce (0, 2);
            auto boxSize = tickBounds.getHeight();

            juce::Path p;
            p.addTriangle (tickBounds.getX(), tickBounds.getY(), tickBounds.getX() + boxSize + 2, tickBounds.getY(), tickBounds.getX() + boxSize * 0.5f + 1, tickBounds.getY() + boxSize);

            auto tickColour = findColour (juce::ToggleButton::tickColourId);
            g.setColour (isEnabled ? tickColour : tickColour.darker());

            auto transform = juce::AffineTransform::rotation (!ticked ? juce::degreesToRadians (270.0f) : 0,
                tickBounds.getCentreX(),
                tickBounds.getCentreY());

            if (!ticked)
                transform = transform.translated (0, -boxSize * 0.25f + 1);

            g.fillPath (p, transform);
        }
    };

    ToggleButtonLnF toggleButtonLookAndFeel;

    juce::ToggleButton toggleButton;
    juce::Component* content = nullptr;
};
