/*
BEGIN_JUCE_MODULE_DECLARATION

 ID:               melatonin_inspector
 vendor:           Sudara
 version:          1.1.0
 name:             Melatonin Inspector
 description:      A component inspector for JUCE, inspired by Figma, web inspector and Jim Credland's Component Debugger
 license:          MIT
 dependencies:     juce_gui_basics 

END_JUCE_MODULE_DECLARATION
*/
#pragma once
#include "melatonin/inspector_panel.h"
#include "melatonin/overlay.h"
#include "melatonin/lookandfeel.h"

namespace melatonin
{
    class Inspector : public juce::ComponentListener, public juce::DocumentWindow
    {
    public:
        Inspector (juce::Component& rootComponent, bool enabledAtStart = true)
            : juce::DocumentWindow ("Melatonin Inspector", melatonin::color::background, 7, true),
              panel (rootComponent, enabledAtStart),
              root (rootComponent),
              enabled (enabledAtStart)
        {
            root.addAndMakeVisible (overlay);
            root.addComponentListener (this);

            // don't use the app's lnf for overlay or panel
            setLookAndFeel (&inspectorLookAndFeel);
            overlay.setLookAndFeel (&inspectorLookAndFeel);
            panel.setLookAndFeel (&inspectorLookAndFeel);

            setResizable (true, false);

            updateWindowSizeOnToggle();

            setUsingNativeTitleBar (true);
            setContentNonOwned (&panel, true);
            setupCallbacks();
        }
        void updateWindowSizeOnToggle()
        {
            auto width = enabled ? jmax(700, getWidth()) : 380;
            auto height = enabled ? jmax(getHeight(), 800) : 400;
            setResizeLimits (width, height, 1200, 1200);
            setSize (width, height);
            panel.setSize (width, height);
        }

        ~Inspector() override
        {
            root.removeComponentListener (this);
            setLookAndFeel (nullptr);
        }

        void outlineComponent (Component* c)
        {
            // don't dogfood the overlay
            if (!enabled || overlay.isParentOf (c))
                return;

            overlay.outlineComponent (c);
            panel.displayComponentInfo (c);
        }

        void outlineDistanceCallback (Component* c)
        {
            if (!enabled || overlay.isParentOf (c))
                return;

            overlay.outlineDistanceCallback (c);
        }

        void selectComponent (Component* c, bool collapseTree = true)
        {
            if (!enabled || overlay.isParentOf (c))
                return;

            overlay.selectComponent (c);
            panel.selectComponent (c, collapseTree);
        }

        void dragComponent (Component* c, const MouseEvent& e)
        {
            if (!enabled || overlay.isParentOf (c))
                return;

            overlay.dragSelectedComponent(e);
            panel.displayComponentInfo (c);
        }

        void startDragComponent (Component* c, const MouseEvent& e)
        {
            if (!enabled || overlay.isParentOf (c))
                return;

            overlay.startDraggingComponent(e);
        }

        // closing the window means turning off the inspector
        void closeButtonPressed() override
        {
            toggle (false);
            setVisible (false);
        }

        void toggle (bool newStatus)
        {
            enabled = newStatus;
            overlay.setVisible (newStatus);
            panel.toggle (newStatus);

            updateWindowSizeOnToggle();
        }

    private:
        // LNF has to be declared before components using it
        melatonin::InspectorLookAndFeel inspectorLookAndFeel;

        melatonin::InspectorPanel panel;
        juce::Component& root;
        bool enabled;
        melatonin::Overlay overlay;
        melatonin::MouseInspector mouseInspector { root };

        // Resize our overlay when the root component changes
        void componentMovedOrResized (Component& rootComponent, bool wasMoved, bool wasResized) override
        {
            if (wasResized || wasMoved)
            {
                overlay.setBounds (rootComponent.getBounds());
            }
        }

        void setupCallbacks()
        {
            mouseInspector.outlineComponentCallback = [this] (Component* c) { this->outlineComponent (c); };
            mouseInspector.outlineDistanceCallback = [this] (Component* c) { this->outlineDistanceCallback (c); };
            mouseInspector.selectComponentCallback = [this] (Component* c) { this->selectComponent (c, true); };
            mouseInspector.componentStartDraggingCallback = [this] (Component* c, const MouseEvent& e) { this->startDragComponent (c, e); };
            mouseInspector.componentDraggedCallback = [this] (Component* c, const MouseEvent& e) { this->dragComponent (c, e); };
            mouseInspector.mouseExitCallback = [this]() { if (this->enabled) panel.redisplaySelectedComponent(); };

            panel.selectComponentCallback = [this] (Component* c) { this->selectComponent (c, true); };
            panel.outlineComponentCallback = [this] (Component* c) { this->outlineComponent (c); };
            panel.toggleCallback = [this] (bool enable) { this->toggle (enable); };
        }
    };
}
