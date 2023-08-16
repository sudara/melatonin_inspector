/*
BEGIN_JUCE_MODULE_DECLARATION

ID:               melatonin_inspector
vendor:           Sudara
version:          1.3.0
name:             Melatonin Inspector
description:      A component inspector for JUCE, inspired by Figma, web inspector and Jim Credland's Component Debugger
license:          MIT
dependencies:     juce_gui_basics, juce_gui_extra

END_JUCE_MODULE_DECLARATION
*/
#pragma once
#include "LatestCompiledAssets/InspectorBinaryData.h"
#include "melatonin/lookandfeel.h"
#include "melatonin_inspector/melatonin/components/overlay.h"
#include "melatonin_inspector/melatonin/helpers/inspector_settings.h"
#include "melatonin_inspector/melatonin/helpers/mouse_listener.h"
#include "melatonin_inspector/melatonin/inspector_component.h"

namespace melatonin
{
    class Inspector : public juce::ComponentListener, public juce::DocumentWindow
    {
    public:
        class OpenInspector : public juce::KeyListener
        {
        public:
            explicit OpenInspector (Inspector& i) : inspector (i) {}
            Inspector& inspector;

            bool keyPressed (const juce::KeyPress& key, Component* /*originatingComponent*/) override
            {
#if JUCE_WINDOWS
                bool modifierPresent = juce::ModifierKeys::getCurrentModifiers().isCtrlDown();
#else
                bool modifierPresent = juce::ModifierKeys::getCurrentModifiers().isCommandDown();
#endif
                if (key.isKeyCurrentlyDown ('I') && modifierPresent)
                {
                    inspector.toggle();
                    return true;
                }

                // let the keypress propagate
                return false;
            }
        };
        explicit Inspector (juce::Component& rootComponent, bool enabledAtStart = true)
            : juce::DocumentWindow ("Melatonin Inspector", colors::background, 7, true),
              inspectorComponent (rootComponent, enabledAtStart),
              root (rootComponent),
              enabled (enabledAtStart)
        {
            setMouseClickGrabsKeyboardFocus (false);
            root.addAndMakeVisible (overlay);
            overlay.setBounds (root.getLocalBounds());
            root.addComponentListener (this);

            // allow us to open/close the inspector by key command
            // bit sketchy because we're modifying the source app to accept key focus
            root.addKeyListener (&keyListener);
            root.setWantsKeyboardFocus (true);
            this->addKeyListener (&keyListener);

            // needs to come before the LNF
            restoreBoundsIfNeeded();

            // use our own lnf for both overlay and inspector
            setLookAndFeel (&inspectorLookAndFeel);
            overlay.setLookAndFeel (&inspectorLookAndFeel);
            inspectorComponent.setLookAndFeel (&inspectorLookAndFeel);

            setUsingNativeTitleBar (true);
            setContentNonOwned (&inspectorComponent, true);
            setupCallbacks();
        }

        ~Inspector() override
        {
            root.removeKeyListener (&keyListener);
            this->removeKeyListener (&keyListener);
            root.removeComponentListener (this);

            // needed, otherwise removing look and feel will save bounds
            settings->props.reset();
            setLookAndFeel (nullptr);
        }

        void moved() override
        {
            DocumentWindow::resized();
            saveBounds();
        }

        void resized() override
        {
            DocumentWindow::resized();
            saveBounds();
        }

        // this is a bit brittle and called a bit too frequently
        // for example 4-5 times on construction
        void saveBounds()
        {
            if (settings->props == nullptr)
                return;

            settings->props->setValue ("x", getX());
            settings->props->setValue ("y", getY());

            // only overwrite width/height when enabled.
            // the disabled dimensions are fixed,
            // so this lets us "open back up" when re-enabling
            if (enabled)
            {
                settings->props->setValue ("enabledWidth", getWidth());
                settings->props->setValue ("enabledHeight", getHeight());
            }

            settings->saveIfNeeded();
        }

        void restoreBoundsIfNeeded()
        {
            // disabled is a fixed 380x400
            // enabled must be at least 700x800
            auto minWidth = enabled ? 700 : 380;
            auto minHeight = enabled ? 800 : 400;

            auto x = settings->props->getIntValue ("x", 0);
            auto y = settings->props->getIntValue ("y", 0);

            if (enabled)
            {
                auto width = settings->props->getIntValue ("enabledWidth", minWidth);
                auto height = settings->props->getIntValue ("enabledHeight", minHeight);
                setResizable (true, false);
                setResizeLimits (minWidth, minHeight, 1200, 1200);
                setBounds (x, y, width, height);
            }
            else
            {
                // decrease the resize limits first for the setSize call to work!
                setResizeLimits (minWidth, minHeight, minWidth, minHeight);
                setBounds (x, y, minWidth, minHeight);
                setResizable (false, false);
            }

            inspectorComponent.setBounds (getLocalBounds());
        }

        void outlineComponent (Component* c)
        {
            // don't dogfood the overlay
            if (!enabled || overlay.isParentOf (c))
                return;

            overlay.outlineComponent (c);
            inspectorComponent.displayComponentInfo (c);
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
            inspectorComponent.selectComponent (c, collapseTree);
        }

        void dragComponent (Component* c, const juce::MouseEvent& e)
        {
            if (!enabled || overlay.isParentOf (c))
                return;

            overlay.dragSelectedComponent (e);
            inspectorComponent.displayComponentInfo (c);
        }

        void startDragComponent (Component* c, const juce::MouseEvent& e)
        {
            if (!enabled || overlay.isParentOf (c))
                return;

            overlay.startDraggingComponent (e);
        }

        // closing the window means turning off the inspector
        void closeButtonPressed() override
        {
            if (onClose)
            {
                // you can provide a callback to destroy the inspector
                onClose();
            }
            else
            {
                // otherwise we'll just hide it
                toggle (false);
                setVisible (false);
            }
        }

        void toggle (bool newStatus)
        {
            // the DocumentWindow always stays open, even when disabled
            setVisible (true);

            enabled = newStatus;
            overlay.setVisible (newStatus);
            inspectorComponent.toggle (newStatus);

            restoreBoundsIfNeeded();
        }

        void toggle()
        {
            toggle (!enabled);
        }

        std::function<void()> onClose;

    private:
        juce::SharedResourcePointer<InspectorSettings> settings;
        melatonin::InspectorLookAndFeel inspectorLookAndFeel;
        melatonin::InspectorComponent inspectorComponent;
        juce::Component& root;
        bool enabled;
        melatonin::Overlay overlay;
        melatonin::MouseListener mouseListener { root };
        OpenInspector keyListener { *this };

        // Resize our overlay when the root component changes
        void componentMovedOrResized (Component& rootComponent, bool wasMoved, bool wasResized) override
        {
            if (wasResized || wasMoved)
            {
                overlay.setBounds (rootComponent.getLocalBounds());
            }
        }

        void setupCallbacks()
        {
            mouseListener.outlineComponentCallback = [this] (Component* c) { this->outlineComponent (c); };
            mouseListener.outlineDistanceCallback = [this] (Component* c) { this->outlineDistanceCallback (c); };
            mouseListener.selectComponentCallback = [this] (Component* c) { this->selectComponent (c, true); };
            mouseListener.componentStartDraggingCallback = [this] (Component* c, const juce::MouseEvent& e) { this->startDragComponent (c, e); };
            mouseListener.componentDraggedCallback = [this] (Component* c, const juce::MouseEvent& e) { this->dragComponent (c, e); };
            mouseListener.mouseExitCallback = [this]() { if (this->enabled) inspectorComponent.redisplaySelectedComponent(); };

            inspectorComponent.selectComponentCallback = [this] (Component* c) { this->selectComponent (c, true); };
            inspectorComponent.outlineComponentCallback = [this] (Component* c) { this->outlineComponent (c); };
            inspectorComponent.toggleCallback = [this] (bool enable) { this->toggle (enable); };
            inspectorComponent.toggleOverlayCallback = [this] (bool enable) {
                this->overlay.setVisible (enable);
                mouseListener.enabled = enable;
            };
        }
    };
}
