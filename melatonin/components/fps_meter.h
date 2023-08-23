#pragma once
#include "juce_gui_basics/juce_gui_basics.h"
#include "melatonin_inspector/melatonin/helpers/colors.h"

namespace melatonin
{
    class FPSMeter : public juce::Component, private juce::Timer
    {
    public:
        explicit FPSMeter (juce::Component& o) : overlay (o)
        {
            overlay.addChildComponent (this);
            setInterceptsMouseClicks (false, false);
        }

        void timerCallback() override
        {
            repaint();
        }

        void visibilityChanged() override
        {
            frameTime = 0;
            lastTime = 0;
            if (isVisible())
            {
#if MELATONIN_VBLANK
                // right before every paint, call repaint
                // syncs to ensure each paint call is preceded by a recalculation
                vBlankCallback = { this,
                    [this] {
                        repaint();
                    } };
#else
                // avoid as much aliasing with the display refresh times as possible
                startTimerHz (120);
#endif
            }
            else
            {
#if MELATONIN_VBLANK
                vBlankCallback = {};
#else
                stopTimer();
#endif
            }
        }

        void paint (juce::Graphics& g) override
        {
            update();
            g.setImageResamplingQuality (juce::Graphics::ResamplingQuality::lowResamplingQuality);
            g.setFont (font);
            g.setColour (colors::black.withAlpha (0.5f));
            g.fillRect (getLocalBounds());
            g.setColour (colors::white);
            g.setFont (juce::Font (12.0f));
            g.drawText (juce::String (juce::String (juce::roundToInt (1000 / frameTime) + 1) + " FPS"), getLocalBounds(), juce::Justification::centredRight, true);
        }

        void update()
        {
            auto now = juce::Time::getMillisecondCounterHiRes();
            auto elapsed = now - lastTime;

            if (juce::approximatelyEqual(lastTime, 0.0))
            {
                lastTime = now;
            }
            else if (juce::approximatelyEqual(frameTime, 0.f))
            {
                frameTime = (float) elapsed;
            }
            else
            {
                // use a static number of hypothetical fps to smooth the value
                // https://stackoverflow.com/a/87333
                float smoothing = std::pow (0.9f, (float) elapsed * 5 / 1000);
                frameTime = (frameTime * smoothing) + ((float) elapsed * (1.0f - smoothing));
            }
            lastTime = now;
        }

    private:
        juce::Component& overlay;
        juce::Rectangle<int> bounds;
        juce::Font font = juce::Font (juce::Font::getDefaultMonospacedFontName(), 12.0f, juce::Font::plain);
        float frameTime = 0;
        double lastTime = juce::Time::getMillisecondCounterHiRes();

#if MELATONIN_VBLANK
        juce::VBlankAttachment vBlankCallback;
#endif
    };
}
