#pragma once
#include "juce_gui_basics/juce_gui_basics.h"

namespace melatonin
{
    // Unfortunately the DocumentWindow cannot behave as our root component mouse listener
    // without some strange side effects. That's why we are doing the whole lambda dance...
    class OverlayMouseListener : public juce::MouseListener
    {
    public:
        OverlayMouseListener()
        {
        }

        ~OverlayMouseListener() override
        {
            if (enabled && root)
                root->removeMouseListener (this);
        }

        void setRoot (juce::Component& c)
        {
            root = &c;

            if (enabled)
                root->addMouseListener (this, true);
        }

        void clearRoot()
        {
            if (enabled && root)
                root->removeMouseListener (this);

            root = nullptr;
        }

        void enable()
        {
            enabled = true;
            root->addMouseListener (this, true);
        }

        void disable()
        {
            enabled = false;
            root->removeMouseListener (this);
        }

        void enableDragging (bool enable)
        {
            dragEnabled = enable;
        }

        void enableSelection (bool enableSelection)
        {
            selectionEnabled = enableSelection;
        }

        void mouseEnter (const juce::MouseEvent& event) override
        {
            if(!selectionEnabled)
                return;

            outlineComponentCallback (event.originalComponent);
        }

        void mouseMove (const juce::MouseEvent& event) override
        {
            if(!selectionEnabled || !dragEnabled)
                return;

            if (outlineDistanceCallback && event.mods.isAltDown())
                outlineDistanceCallback (event.originalComponent);
            else
                outlineDistanceCallback (nullptr);
        }

        void mouseUp (const juce::MouseEvent& event) override
        {
            if (event.mods.isLeftButtonDown() && !isDragging && selectionEnabled)
            {
                selectComponentCallback (event.originalComponent);
            }
            isDragging = false;
        }

        void mouseDown (const juce::MouseEvent& event) override
        {
            if(!dragEnabled)
                return;

            if (event.mods.isLeftButtonDown() && event.originalComponent->isMouseOverOrDragging())
            {
                componentStartDraggingCallback (event.originalComponent, event);
            }
        }

        void mouseDrag (const juce::MouseEvent& event) override
        {
            if(!dragEnabled)
                return;

            // takes care of small mouse position drift on selection
            if (event.getDistanceFromDragStart() > 3 && event.originalComponent->isMouseOverOrDragging())
            {
                isDragging = true;
                componentDraggedCallback (event.originalComponent, event);
            }
        }

        void mouseExit (const juce::MouseEvent& event) override
        {
            if (event.originalComponent == root)
            {
                mouseExitCallback();
            }
        }

        std::function<void (juce::Component* c)> outlineComponentCallback;
        std::function<void (juce::Component* c)> outlineDistanceCallback;
        std::function<void (juce::Component* c)> selectComponentCallback;
        std::function<void (juce::Component* c, const juce::MouseEvent& e)> componentStartDraggingCallback;
        std::function<void (juce::Component* c, const juce::MouseEvent& e)> componentDraggedCallback;
        std::function<void()> mouseExitCallback;

    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OverlayMouseListener)

        juce::Component* root = nullptr;
        bool enabled = false;
        bool isDragging { false };
        bool dragEnabled { false }, selectionEnabled { false };
    };
}
