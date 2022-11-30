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
            setResizeLimits (enabled ? 700 : 380, 500, 1200, 1200);
            setSize (enabled ? 700 : 380, 500);
            panel.setSize (enabled ? 700 : 380, 500);
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

        void selectComponent (Component* c, bool collapseTree = true)
        {
            if (!enabled || overlay.isParentOf (c))
                return;

            overlay.selectComponent (c);
            panel.selectComponent (c, collapseTree);
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
                overlay.setBounds (rootComponent.getBounds());
        }

        void setupCallbacks()
        {
            mouseInspector.outlineComponentCallback = [this] (Component* c) { this->outlineComponent (c); };
            mouseInspector.selectComponentCallback = [this] (Component* c) { this->selectComponent (c, true); };
            mouseInspector.mouseExitCallback = [this]() { if (this->enabled) panel.redisplaySelectedComponent(); };

            panel.selectComponentCallback = [this] (Component* c) { this->selectComponent (c, true); };
            panel.outlineComponentCallback = [this] (Component* c) { this->outlineComponent (c); };
            panel.toggleCallback = [this] (bool enable) { this->toggle (enable); };
        }
    };
}
