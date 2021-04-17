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
    MelatoninInspector (juce::Component& rootComponent)
        : juce::DocumentWindow ("Melatonin Inspector", Colours::black, 7, true),
          panel (rootComponent, overlay),
          mouseInspector (rootComponent),
          root (rootComponent)
    {
        root.addAndMakeVisible (overlay);
        root.addComponentListener (this);

        mouseInspector.outlineComponentCallback = [this] (Component* c) { this->outlineComponent (c); };
        mouseInspector.selectComponentCallback = [this] (Component* c) { this->selectComponent (c); };
        panel.selectComponentCallback = [this] (Component* c) { this->selectComponent (c); };
        panel.outlineComponentCallback = [this] (Component* c) { this->outlineComponent (c); };
        panel.setSize (400, 400);
        // don't use the app's lnf for overlay or panel
        setLookAndFeel (&inspectorLookAndFeel);
        overlay.setLookAndFeel (&inspectorLookAndFeel);
        panel.setLookAndFeel (&inspectorLookAndFeel);

        setResizable (true, false);
        setResizeLimits (300, 500, 1200, 1200);
        setSize (350, 550);
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
        if (overlay.isParentOf (c))
            return;

        overlay.outlineComponent (c);
        panel.displayComponentInfo (c);
    }

    void selectComponent (Component* c)
    {
        if (overlay.isParentOf (c))
            return;
        overlay.selectComponent (c);
        panel.displayComponentInfo (c);
        panel.selectComponent (c);
    }

    void closeButtonPressed() override
    {
        setVisible (false);
    }

private:
    // Resize our overlay when the root component changes
    void componentMovedOrResized (Component& component, bool /*wasMoved*/, bool wasResized) override
    {
        if (wasResized)
            overlay.setBounds (component.getBounds());
    }

    juce::Component& root;
    melatonin::InspectorPanel panel;
    melatonin::Overlay overlay;
    melatonin::MouseInspector mouseInspector;
    melatonin::InspectorLookAndFeel inspectorLookAndFeel;
    float originalWidth = 0.f;
};
