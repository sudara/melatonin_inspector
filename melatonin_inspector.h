/*
BEGIN_JUCE_MODULE_DECLARATION

ID:                 melatonin_inspector
vendor:             Sudara
version:            1.3.0
name:               Melatonin Inspector
description:        A component inspector for JUCE, inspired by Figma, web inspector and Jim Credland's Component Debugger
license:            MIT
dependencies:       juce_gui_basics, juce_gui_extra
minimumCppStandard: 17

END_JUCE_MODULE_DECLARATION
*/
#pragma once

#ifndef PERFETTO
    #define TRACE_COMPONENT(...)
#endif

#include "melatonin/lookandfeel.h"
#include "melatonin_inspector/melatonin/components/overlay.h"
#include "melatonin_inspector/melatonin/helpers/inspector_settings.h"
#include "melatonin_inspector/melatonin/helpers/mouse_listener.h"
#include "melatonin_inspector/melatonin/inspector_component.h"
#include <melatonin_inspector/melatonin/components/fps_meter.h>

namespace melatonin
{
    class Inspector : public juce::ComponentListener, public juce::DocumentWindow
    {
    public:
        class InspectorKeyCommands : public juce::KeyListener
        {
        public:
            explicit InspectorKeyCommands (Inspector& i) : inspector (i) {}
            Inspector& inspector;

            bool keyPressed (const juce::KeyPress& keyPress, Component* /*originatingComponent*/) override
            {
#if JUCE_WINDOWS
                bool modifierPresent = juce::ModifierKeys::getCurrentModifiers().isCtrlDown();
#else
                bool modifierPresent = juce::ModifierKeys::getCurrentModifiers().isCommandDown();
#endif
                if (keyPress.isKeyCurrentlyDown ('I') && modifierPresent)
                {
                    inspector.toggle();
                    return true;
                }
                else if (keyPress.isKeyCode (juce::KeyPress::escapeKey))
                {
                    if (inspector.inspectorEnabled)
                    {
                        inspector.clearSelections();
                    }
                    return true;
                }

                // let the keypress propagate
                return false;
            }
        };
        explicit Inspector (juce::Component& rootComponent, bool inspectorEnabledAtStart = true)
            : juce::DocumentWindow ("Melatonin Inspector", colors::background, 7, true),
              inspectorComponent (rootComponent),
              root (rootComponent)
        {
            TRACE_COMPONENT();
            root.addChildComponent (overlay);
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

            toggle (inspectorEnabledAtStart);
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
            TRACE_COMPONENT();
            DocumentWindow::resized();
            saveBounds();
        }

        void resized() override
        {
            TRACE_COMPONENT();
            DocumentWindow::resized();
            saveBounds();
        }

        // this is a bit brittle and called a bit too frequently
        // for example 4-5 times on construction
        void saveBounds()
        {
            TRACE_COMPONENT();
            if (settings->props == nullptr)
                return;

            settings->props->setValue ("x", getX());
            settings->props->setValue ("y", getY());

            // only overwrite width/height when inspectorEnabled.
            // the disabled dimensions are fixed,
            // so this lets us "open back up" when re-enabling
            if (inspectorEnabled)
            {
                settings->props->setValue ("inspectorEnabledWidth", getWidth());
                settings->props->setValue ("inspectorEnabledHeight", getHeight());
            }

            settings->saveIfNeeded();
        }

        void restoreBoundsIfNeeded()
        {
            TRACE_COMPONENT();
            // disabled is a fixed 380x400
            // inspectorEnabled must be at least 700x800
            auto minWidth = inspectorEnabled ? 700 : 380;
            auto minHeight = inspectorEnabled ? 800 : 400;

            auto x = settings->props->getIntValue ("x", 0);
            auto y = settings->props->getIntValue ("y", 0);

            if (inspectorEnabled)
            {
                auto width = settings->props->getIntValue ("inspectorEnabledWidth", minWidth);
                auto height = settings->props->getIntValue ("inspectorEnabledHeight", minHeight);
                setResizable (true, false); // calls resized
                setResizeLimits (minWidth, minHeight, 1200, 1200);
                setBounds (x, y, width, height);
            }
            else
            {
                // decrease the resize limits first for the setSize call to work!
                // the order of these calls matters a lot
                setResizeLimits (minWidth, minHeight, minWidth, minHeight);
                setBounds (x, y, minWidth, minHeight);
                setResizable (false, false);
            }

            inspectorComponent.setBounds (getLocalBounds());
        }

        void outlineComponent (Component* c)
        {
            TRACE_COMPONENT();

            // don't dogfood the overlay
            if (!inspectorEnabled || overlay.isParentOf (c))
                return;

            overlay.outlineComponent (c);
            inspectorComponent.displayComponentInfo (c);
        }

        void outlineDistanceCallback (Component* c)
        {
            TRACE_COMPONENT();

            if (!inspectorEnabled || overlay.isParentOf (c))
                return;

            overlay.outlineDistanceCallback (c);
        }

        void selectComponent (Component* c)
        {
            TRACE_COMPONENT();

            if (!inspectorEnabled || overlay.isParentOf (c))
                return;

            overlay.selectComponent (c);
            inspectorComponent.selectComponent (c);
        }

        void dragComponent (Component* c, const juce::MouseEvent& e)
        {
            TRACE_COMPONENT();

            if (!inspectorEnabled || overlay.isParentOf (c))
                return;

            overlay.dragSelectedComponent (e);
            inspectorComponent.displayComponentInfo (c);
        }

        void startDragComponent (Component* c, const juce::MouseEvent& e)
        {
            TRACE_COMPONENT();

            if (!inspectorEnabled || overlay.isParentOf (c))
                return;

            overlay.startDraggingComponent (e);
        }

        void clearSelections()
        {
            TRACE_COMPONENT();

            inspectorComponent.deselectComponent();
            overlay.outlineComponent (nullptr);
            overlay.selectComponent (nullptr);
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
            TRACE_COMPONENT();

            // the DocumentWindow always stays open, even when disabled
            setVisible (true);

            inspectorEnabled = newStatus;
            inspectorComponent.toggle (newStatus);
            restoreBoundsIfNeeded();

            overlay.setVisible (newStatus);
            if (newStatus)
                // without this, target apps that have UI to open the inspector
                // will select that piece of UI within the same click, see #70
                juce::Timer::callAfterDelay (500, [&]() { overlayMouseListener.enable(); });
            else
            {
                clearSelections();
                overlayMouseListener.disable();
            }
        }

        void toggle()
        {
            toggle (!inspectorEnabled);
        }

        std::function<void()> onClose;

    private:
        juce::SharedResourcePointer<InspectorSettings> settings;
        melatonin::InspectorLookAndFeel inspectorLookAndFeel;
        melatonin::InspectorComponent inspectorComponent;
        juce::Component& root;
        bool inspectorEnabled = false;
        melatonin::Overlay overlay;
        melatonin::FPSMeter fpsMeter { overlay };
        melatonin::MouseListener overlayMouseListener { root, false };
        InspectorKeyCommands keyListener { *this };

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
            overlayMouseListener.outlineComponentCallback = [this] (Component* c) { this->outlineComponent (c); };
            overlayMouseListener.outlineDistanceCallback = [this] (Component* c) { this->outlineDistanceCallback (c); };
            overlayMouseListener.selectComponentCallback = [this] (Component* c) { this->selectComponent (c); };
            overlayMouseListener.componentStartDraggingCallback = [this] (Component* c, const juce::MouseEvent& e) { this->startDragComponent (c, e); };
            overlayMouseListener.componentDraggedCallback = [this] (Component* c, const juce::MouseEvent& e) { this->dragComponent (c, e); };
            overlayMouseListener.mouseExitCallback = [this]() { if (this->inspectorEnabled) inspectorComponent.redisplaySelectedComponent(); };

            inspectorComponent.selectComponentCallback = [this] (Component* c) { this->selectComponent (c); };
            inspectorComponent.outlineComponentCallback = [this] (Component* c) { this->outlineComponent (c); };
            inspectorComponent.toggleCallback = [this] (bool enable) { this->toggle (enable); };
            inspectorComponent.toggleOverlayCallback = [this] (bool enable) {
                this->overlay.setVisible (enable);
                overlayMouseListener.enable();
            };
            inspectorComponent.toggleFPSCallback = [this] (bool enable) {
                if (enable)
                    this->fpsMeter.setBounds (this->overlay.getLocalBounds().removeFromRight (50).removeFromTop (30));
                this->fpsMeter.setVisible (enable);
            };
        }
    };
}
