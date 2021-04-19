/*
BEGIN_JUCE_MODULE_DECLARATION

 ID:               melatonin_inspector
 vendor:           Sudara
 version:          1.0.0
 name:             Melatonin Inspector
 description:      A component inspector for JUCE, inspired by Figma, web inspector and Jim Credland's Component Debugger
 license:          MIT
 dependencies:     juce_gui_basics 

END_JUCE_MODULE_DECLARATION
*/
#pragma once
#include "melatonin/inspector_panel.h"
#include "melatonin/overlay.h"

class MelatoninInspector : public juce::ComponentListener, public juce::DocumentWindow
{
public:
    MelatoninInspector (juce::Component& rootComponent, bool enabledAtStart)
        : juce::DocumentWindow ("Melatonin Inspector", juce::Colours::black, 7, true),
          panel (rootComponent, overlay, enabledAtStart),
          root (rootComponent),
          mouseInspector (rootComponent),
          enabled(enabledAtStart)
    {
        root.addAndMakeVisible (overlay);
        root.addComponentListener (this);

        mouseInspector.outlineComponentCallback = [this] (Component* c) { this->outlineComponent (c); };
        mouseInspector.selectComponentCallback = [this] (Component* c) { this->selectComponent (c, true); };
        panel.selectComponentCallback = [this] (Component* c) { this->selectComponent (c, false); };
        panel.outlineComponentCallback = [this] (Component* c) { this->outlineComponent (c); };
        panel.toggleCallback = [this] (bool enabled) { this->toggle (enabled); };
        panel.setSize (400, 400);
        // don't use the app's lnf for overlay or panel
        setLookAndFeel (&inspectorLookAndFeel);
        overlay.setLookAndFeel (&inspectorLookAndFeel);
        panel.setLookAndFeel (&inspectorLookAndFeel);

        setResizable (true, false);
        setResizeLimits (300, 500, 1200, 1200);
        setSize (350, 650);
        setUsingNativeTitleBar (true);
        setContentNonOwned (&panel, true);
        setVisible (true);
    }

    ~MelatoninInspector() override
    {
        root.removeComponentListener (this);
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
        panel.displayComponentInfo (c);
        panel.selectComponent (c, collapseTree);
    }

    // closing the window means turning off the inspector
    void closeButtonPressed() override
    {
        toggle(false);
        setVisible (false);
    }

    void toggle (bool newStatus)
    {
        enabled = newStatus;
        overlay.setVisible (newStatus);
    }

private:
    // Resize our overlay when the root component changes
    void componentMovedOrResized (Component& root, bool /*wasMoved*/, bool wasResized) override
    {
        if (wasResized)
            overlay.setBounds (root.getBounds());
    }

    bool enabled;

    // LNF has to be declared before components using it
    melatonin::InspectorLookAndFeel inspectorLookAndFeel;

    juce::Component& root;
    melatonin::InspectorPanel panel;
    melatonin::Overlay overlay;
    melatonin::MouseInspector mouseInspector;
};
