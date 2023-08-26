#pragma once
#include <juce_audio_processors/juce_audio_processors.h>

// VBlank added in 7.0.3
#if JUCE_MAJOR_VERSION > 7 || (JUCE_MAJOR_VERSION == 7 && JUCE_MINOR_VERSION > 0) || (JUCE_MAJOR_VERSION == 7 && JUCE_BUILDNUMBER >= 3)
DBG ("Using VBlank")
#endif

namespace melatonin
{
    class Fps : public juce::Component, public juce::HighResolutionTimer
    {
    protected:
        constexpr static int interval = 1000; // 1 second
        std::atomic<unsigned int> frameCount = 0;
        juce::VBlankAttachment vblank { this, [&]() { update(); } };

    public:
        unsigned int displayedFPS = 0;

        Fps()
        {
            startTimer (interval);
        }

        ~Fps() override
        {
            stopTimer();
        }

        void paint (juce::Graphics& g) override
        {
            g.fillAll (juce::Colours::black);
            g.setColour (juce::Colours::white);
            g.setFont (juce::Font (20.0f));
            g.drawText (juce::String (displayedFPS), getLocalBounds(), juce::Justification::centred, true);
        }

    private:
        void hiResTimerCallback() override
        {
            displayedFPS = frameCount;
            frameCount = 0;
            repaint();
        }
        void update()
        {
            ++frameCount;
        }
    };
}
