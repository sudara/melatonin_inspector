#pragma once
#include <cxxabi.h>

namespace melatonin
{
    using namespace juce;
    
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
    static inline String type (const T& t)
    {
        return demangle (typeid (t).name());
    }

    // do our best to derive a useful UI string from a component
    static inline String componentString (Component* c)
    {
        if (auto label = dynamic_cast<Label*> (c))
        {
            return String ("Label: ") + label->getText().substring (0, 20);
        }
        else if (auto editor = dynamic_cast<AudioProcessorEditor*> (c))
        {
            return String ("Editor: ") + editor->getAudioProcessor()->getName();
        }
        else if (c->getName().isEmpty())
        {
            return type (*c);
        }
        else
        {
            return c->getName();
        }
    }

    static inline String dimensionsString (Rectangle<int> bounds)
    {
        return String (bounds.getWidth()) + L" × " + String (bounds.getHeight());
    }

    namespace color
    {
        const Colour white = Colours::white; // consistency
        const Colour background = Colour::fromString ("FF0F0F0F");
        const Colour blueLineColor = Colour::fromString ("FF429DE2");
        const Colour blueLabelBackgroundColor = Colour::fromString ("FF149DF9");
        const Colour blueLabelTextColor = Colour::fromString ("FFF4FDFF");
        const Colour redLineColor = Colour::fromString ("FFD4563F");
        const Colour redLabelBackgroundColor = Colour::fromString ("FFF54514");
    }

    class InspectorLookAndFeel : public LookAndFeel_V4
    {
    public:
        InspectorLookAndFeel()
        {
            // often the app overrides this
            setDefaultSansSerifTypefaceName (Font::getDefaultSansSerifFontName());
            setColour (Label::outlineWhenEditingColourId, color::redLineColor);
        }
        void drawResizableFrame (Graphics& g, int w, int h, const BorderSize<int>& border) override
        {
            ignoreUnused (g, w, h, border);
        }

        void drawTreeviewPlusMinusBox (Graphics& g, const Rectangle<float>& area, Colour backgroundColour, bool isOpen, bool isMouseOver) override
        {
            Path p;
            p.addTriangle (0.0f, 0.0f, 1.0f, isOpen ? 0.0f : 0.5f, isOpen ? 0.5f : 0.0f, 1.0f);
            g.setColour (backgroundColour);
            g.fillPath (p, p.getTransformToScaleToFit (area.reduced (2, area.getHeight() / 4).translated (3, 0), true));
        }
    };
}
