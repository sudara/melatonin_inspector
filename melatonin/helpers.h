#pragma once
#if !defined(_MSC_VER)
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
}
#else
    template <class T>
    static inline juce::String type (const T& t)
    {
        return juce::String(typeid (t).name()).replace("class ","").replace("struct ","");
    }
#endif

namespace melatonin
{
    using namespace juce;

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
        else if (c && ! c->getName().isEmpty())
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
            setColour (Label::outlineWhenEditingColourId, color::redLineColor);
        }

        // we don't want our resizer in the overlay to have a fugly border
        void drawResizableFrame (Graphics& g, int w, int h, const BorderSize<int>& border) override
        {
            ignoreUnused (g, w, h, border);
        }

        // For some reason this is actually *needed* which is strange.
        // But we want to adjust the color and size of triangles anyway
        void drawTreeviewPlusMinusBox (Graphics& g, const Rectangle<float>& area, Colour backgroundColour, bool isOpen, bool isMouseOver) override
        {
            Path p;
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
        Font getLabelFont (Label& label) override
        {
            return Font ("Verdana", label.getFont().getHeight(), Font::FontStyleFlags::plain);
        }

        // oh i dream of css resets...
        BorderSize<int> getLabelBorderSize (Label&) override
        {
            return BorderSize<int> (0);
        }
    };

}
