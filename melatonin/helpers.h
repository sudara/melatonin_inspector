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
            return "";
        }
    }

    static inline juce::String dimensionsString (juce::Rectangle<int> bounds)
    {
        return juce::String (bounds.getWidth()) + L" × " + juce::String (bounds.getHeight());
    }

    namespace color
    {
        const juce::Colour white = juce::Colours::white; // consistency
        const juce::Colour background = juce::Colour::fromString ("FF0F0F0F");
        const juce::Colour blueLineColor = juce::Colour::fromString ("FF429DE2");
        const juce::Colour blueLabelBackgroundColor = juce::Colour::fromString ("FF149DF9");
        const juce::Colour blueLabelTextColor = juce::Colour::fromString ("FFF4FDFF");
        const juce::Colour redLineColor = juce::Colour::fromString ("FFD4563F");
        const juce::Colour redLabelBackgroundColor = juce::Colour::fromString ("FFF54514");
        const juce::Colour greyLineColor = juce::Colours::lightgrey;
    }

    class InspectorLookAndFeel : public juce::LookAndFeel_V4
    {
    public:
        InspectorLookAndFeel()
        {
            // often the app overrides this
            setColour (juce::Label::outlineWhenEditingColourId, color::redLineColor);
        }

        // we don't want our resizer in the overlay to have a fugly border
        void drawResizableFrame (juce::Graphics& g, int w, int h, const juce::BorderSize<int>& border) override
        {
            ignoreUnused (g, w, h, border);
        }

        // For some reason this is actually *needed* which is strange.
        // But we want to adjust the color and size of triangles anyway
        void drawTreeviewPlusMinusBox (juce::Graphics& g, const juce::Rectangle<float>& area, juce::Colour backgroundColour, bool isOpen, bool /*isMouseOver*/) override
        {
            juce::Path p;
            p.addTriangle (0.0f, 0.0f, 1.0f, isOpen ? 0.0f : 0.5f, isOpen ? 0.5f : 0.0f, 1.0f);
            g.setColour (backgroundColour);
            g.fillPath (p, p.getTransformToScaleToFit (area.reduced (0, area.getHeight() / 4).translated (1, 0), true));
        }

        // more friendly scrolling
        int getDefaultScrollbarWidth() override
        {
            return 10;
        }

        // don't use the target app's font
        juce::Font getLabelFont (juce::Label& label) override
        {
            return juce::Font ("Verdana", label.getFont().getHeight(), juce::Font::FontStyleFlags::plain);
        }

        // oh i dream of css resets...
        juce::BorderSize<int> getLabelBorderSize (juce::Label&) override
        {
            return juce::BorderSize<int> (0);
        }
    };

}
