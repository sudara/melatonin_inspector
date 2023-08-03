#pragma once
#include "../../LatestCompiledAssets/InspectorBinaryData.h"
#include <juce_audio_processors/juce_audio_processors.h>

namespace melatonin
{
    static inline juce::String dimensionsString (juce::Rectangle<int> bounds)
    {
        // account for outline border drawing (1 on each side)
        return juce::String (bounds.getWidth() - 2) + L" × " + juce::String (bounds.getHeight() - 2);
    }

    static inline juce::String distanceString (juce::Line<float> line)
    {
        // account for outline border drawing (1 on each side)
        auto v = line.getLength() + 3;
        return juce::String (v);
    }

    static inline juce::Image getIcon (juce::String iconName)
    {
        int size = 0;
        auto filename = iconName.replace (" ", "") + "_png";
        auto data = InspectorBinaryData::getNamedResource (filename.getCharPointer(), size);
        if (size > 0)
        {
            return juce::ImageCache::getFromMemory (data, size);
        }
        else
        {
            // you told me to load an image that doesn't exist
            jassertfalse;
            return {};
        }
    }

    // see ComponentModel::populateCustomValues
    static void setCustomInspectorProperty (juce::Component* component, juce::String key, juce::var value)
    {
        auto& props = component->getProperties();

        // allow up to 10 custom properties for now
        for (auto i = 1; i < 11; ++i)
        {
            if (!props.contains ("inspectorPropertyName" + juce::String (i)))
            {
                props.set ("inspectorPropertyName" + juce::String (i), key);
                props.set ("inspectorPropertyValue" + juce::String (i), value);
                return;
            }
        }
    }
}
