#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

#if !defined(_MSC_VER)
#include <cxxabi.h>
namespace melatonin
{
    // https://stackoverflow.com/a/4541470
    static inline std::string demangle (const char* name)
    {
        int status = -4; // some arbitrary value to eliminate the compiler warning

        std::unique_ptr<char, void (*) (void*)> res {
            abi::__cxa_demangle (name, nullptr, nullptr, &status),
            std::free
        };

        return (status == 0) ? res.get() : name;
    }

    template <class T>
    static inline juce::String type (const T& t)
    {
        return demangle (typeid (t).name());
    }
}
#else
template <class T>
static inline juce::String type (const T& t)
{
    return juce::String (typeid (t).name()).replace ("class ", "").replace ("struct ", "");
}
#endif

namespace melatonin
{

    // do our best to derive a useful UI string from a component
    static inline juce::String componentString (juce::Component* c)
    {
        if (auto* label = dynamic_cast<juce::Label*> (c))
        {
            return juce::String ("Label: ") + label->getText().substring (0, 20);
        }
        else if (auto editor = dynamic_cast<juce::AudioProcessorEditor*> (c))
        {
            return juce::String ("Editor: ") + editor->getAudioProcessor()->getName();
        }
        else if (c && !c->getName().isEmpty())
        {
            return c->getName();
        }
        else if (c)
        {
            return type (*c);
        }
        else
        {
            return "";
        }
    }

    // do our best to derive a useful UI string from a component
    static inline juce::String componentFontValue (juce::Component* c)
    {
        if (auto* label = dynamic_cast<juce::Label*> (c))
        {
            auto font = label->getFont();
            return juce::String (font.getTypefaceName() + " " + font.getTypefaceStyle() + " " + juce::String(font.getHeight()));
        }
        else if (auto btn = dynamic_cast<juce::TextButton*> (c))
        {
            auto font = btn->getLookAndFeel().getTextButtonFont(*btn, btn->getHeight());
            return juce::String (font.getTypefaceName() + " " + font.getTypefaceStyle() + " " + juce::String(font.getHeight()));
        }
        else if (auto slider = dynamic_cast<juce::Slider*> (c))
        {
            auto font = slider->getLookAndFeel().getSliderPopupFont(*slider);
            return juce::String (font.getTypefaceName() + " " + font.getTypefaceStyle() + " " + juce::String(font.getHeight()));
        }
        else if (auto cb = dynamic_cast<juce::ComboBox*> (c))
        {
            auto font = cb->getLookAndFeel().getComboBoxFont(*cb);
            return juce::String (font.getTypefaceName() + " " + font.getTypefaceStyle() + " " + juce::String(font.getHeight()));
        }
        else
        {
            return "-";
        }
    }

    //returns name of assigned LnF
    static inline juce::String lnfString (juce::Component* c)
    {
        if (c)
        {
            auto &lnf = c->getLookAndFeel();
            return type (lnf);
        }
        else
        {
            return "-";
        }
    }

    static inline juce::String dimensionsString (juce::Rectangle<int> bounds)
    {
        //account for outline border drawing (1 on each side)
        return juce::String (bounds.getWidth() - 2) + L" × " + juce::String (bounds.getHeight() - 2);
    }

    namespace color
    {
        const juce::Colour white = juce::Colours::white; // consistency
        const juce::Colour background = juce::Colour::fromString ("FF2B1D32");
        const juce::Colour blueLineColor = juce::Colour::fromString ("FF429DE2");
        const juce::Colour blueLabelBackgroundColor = juce::Colour::fromString ("FF149DF9");
        const juce::Colour blueLabelTextColor = juce::Colour::fromString ("FFF4FDFF");
        const juce::Colour redLineColor = juce::Colour::fromString ("FFD4563F");
        const juce::Colour redLabelBackgroundColor = juce::Colour::fromString ("FFF54514");
        const juce::Colour greyLineColor = juce::Colours::lightgrey;

        const juce::Colour panelLineSeparatorColor = juce::Colour::fromString("FF0C0713");
        const juce::Colour titleTextColor = juce::Colour::fromString("FF768FBE");
        const juce::Colour yellowColor = juce::Colour::fromString("FFFFE58A");
        const juce::Colour backgroundDarkerColor = juce::Colour::fromString("FF1C1526");
        const juce::Colour blackColor = juce::Colour::fromString("FF000000");

        const juce::Colour bluePropsScrollbarColor = juce::Colour::fromString("FF5C678D");
        const juce::Colour treeViewMinusPlusColor = juce::Colour::fromString("FF776F81");
    }


    //create component with toggle button to show/hide content below including component size change
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
            g.setColour (color::background);
            g.drawRect (getLocalBounds(), 1);
        }

        //important to call parent (this) resized() from child resized() after content size is set
        void resized() override
        {
            juce::Rectangle<int> r (8, 8, getWidth(), 32);
            auto buttonHeight = 32;

            if (content && content->isVisible())
            {
                //content->resized();
                r = r.withHeight (content->getHeight() + buttonHeight);

                auto r1 = r;
                content->setBounds (r1.removeFromBottom (content->getHeight())
                                        .reduced (paddingHor, paddingVer));
            }
            auto tbBounds = r;
            toggleButton.setBounds (tbBounds.removeFromTop (buttonHeight));

            setSize (r.getWidth(), r.getHeight());
        }

        int paddingHor = 32;
        int paddingVer = 8;

    private:
        struct ToggleBtnLoonkAndFeel : juce::LookAndFeel_V4
        {
            ToggleBtnLoonkAndFeel()
            {
                setColour(juce::ToggleButton::textColourId, color::titleTextColor);
                setColour(juce::ToggleButton::tickColourId, color::titleTextColor);
            }

            //override function for drawing toggle btn
            void drawToggleButton (juce::Graphics& g, juce::ToggleButton& button, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
            {
                using namespace juce;

                auto fontSize = jmin (15.0f, (float) button.getHeight() * 0.75f);
                auto tickWidth = fontSize;

                drawTickBox (g, button, 4.0f, ((float) button.getHeight() - tickWidth) * 0.5f, tickWidth, tickWidth, button.getToggleState(), button.isEnabled(), shouldDrawButtonAsHighlighted, shouldDrawButtonAsDown);

                g.setColour (button.findColour (ToggleButton::textColourId));
                g.setFont (fontSize);

                if (!button.isEnabled())
                    g.setOpacity (0.5f);

                g.drawFittedText (button.getButtonText(),
                    button.getLocalBounds().withTrimmedLeft (roundToInt (tickWidth) + 10).withTrimmedRight (2),
                    Justification::centredLeft,
                    10);
            }

            //override function for drawing tick box
            void drawTickBox (juce::Graphics& g, juce::Component& component, float x, float y, float w, float h, const bool ticked, const bool isEnabled, const bool shouldDrawButtonAsHighlighted, const bool shouldDrawButtonAsDown) override
            {
                using namespace juce;

                Rectangle<float> tickBounds (x, y, w, h);

                tickBounds.reduce (0, 2);
                auto boxSize = tickBounds.getHeight();

                Path p;
                p.addTriangle (tickBounds.getX(), tickBounds.getY(), tickBounds.getX() + boxSize + 2, tickBounds.getY(), tickBounds.getX() + boxSize * 0.5f + 1, tickBounds.getY() + boxSize);

                auto tickColour = findColour (ToggleButton::tickColourId);
                g.setColour (isEnabled ? tickColour : tickColour.darker());

                auto transform = AffineTransform::rotation (!ticked ? degreesToRadians(270.0f)
                                                                    : 0,
                    tickBounds.getCentreX(),
                    tickBounds.getCentreY());

                if(!ticked)
                    transform = transform.translated(0, -boxSize * 0.25f + 1);

                g.fillPath (p, transform);
            }
        };

        ToggleBtnLoonkAndFeel toggleButtonLookAndFeel;

        juce::ToggleButton toggleButton;
        juce::Component* content = nullptr;
    };


}
