/*
BEGIN_JUCE_MODULE_DECLARATION

ID:               melatonin_inspector
vendor:           Sudara
version:          1.2.0
name:             Melatonin Inspector
description:      A component inspector for JUCE, inspired by Figma, web inspector and Jim Credland's Component Debugger
license:          MIT
dependencies:     juce_gui_basics

END_JUCE_MODULE_DECLARATION
*/
#pragma once
#include "LatestCompiledAssets/InspectorBinaryData.h"
#include "melatonin/lookandfeel.h"
#include "melatonin_inspector/melatonin/components/overlay.h"
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
        explicit Inspector (juce::Component& rootComponent, bool enabledAtStart = true, juce::PropertiesFile* settings_ = nullptr)
            : juce::DocumentWindow ("Melatonin Inspector", colors::background, 7, true),
              inspectorComponent (rootComponent, enabledAtStart),
              root (rootComponent),
              enabled (enabledAtStart)
        {
            root.addAndMakeVisible (overlay);
            overlay.setBounds (root.getLocalBounds());
            root.addComponentListener (this);

            // allow us to open/close the inspector by key command
            // bit sketchy because we're modifying the source app to accept key focus
            root.addKeyListener (&keyListener);
            root.setWantsKeyboardFocus (true);
            this->addKeyListener (&keyListener);

            // don't use the app's lnf for overlay or inspector
            setLookAndFeel (&inspectorLookAndFeel);
            overlay.setLookAndFeel (&inspectorLookAndFeel);
            inspectorComponent.setLookAndFeel (&inspectorLookAndFeel);

            setResizable (true, false);
            setSettings (settings_);
            updateWindowSize();

            setUsingNativeTitleBar (true);
            setContentNonOwned (&inspectorComponent, true);
            setupCallbacks();
        }

        void setSettings (juce::PropertiesFile* settings_)
        {
            if (settings_ != nullptr)
            {
                settings.set (settings_, false);
            }
            else
            {
                juce::PropertiesFile::Options opts;

                opts.applicationName = "melatonin_inspector";
                opts.filenameSuffix = ".xml";
                opts.folderName = "sudara";
                opts.osxLibrarySubFolder = "Application Support";
                opts.commonToAllUsers = false;
                opts.ignoreCaseOfKeyNames = false;
                opts.doNotSave = false;
                opts.millisecondsBeforeSaving = 1;
                opts.storageFormat = juce::PropertiesFile::storeAsXML;

                settings.set (new juce::PropertiesFile (opts), true);
            }
        }

        void moved() override
        {
            DocumentWindow::resized();
            saveWindowPosition();
        }

        void resized() override
        {
            DocumentWindow::resized();
            saveWindowPosition();
        }

        void saveWindowPosition()
        {
            if (settings != nullptr)
            {
                settings->setValue ("melatoninInspectorPosition", getWindowStateAsString());
                settings->saveIfNeeded();
            }
        }

        void updateWindowSize()
        {
            auto position = settings->getValue ("melatoninInspectorPosition", "");

            setResizeLimits (380, 400, 1200, 1200);

            if (position.isNotEmpty())
            {
                restoreWindowStateFromString (position);
                inspectorComponent.setSize (getWidth(), getHeight());
            }
            else
            {
                auto width = enabled ? juce::jmax (700, getWidth()) : 380;
                auto height = enabled ? juce::jmax (getHeight(), 800) : 400;
                setSize (width, height);
                inspectorComponent.setSize (width, height);
            }
        }

        ~Inspector() override
        {
            root.removeKeyListener (&keyListener);
            this->removeKeyListener (&keyListener);
            root.removeComponentListener (this);
            setLookAndFeel (nullptr);
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
                onClose();
            }
            else
            {
                toggle (false);
                setVisible (false);
            }
        }

        void toggle (bool newStatus)
        {
            enabled = newStatus;
            overlay.setVisible (newStatus);

            // once the inspector is open, leave the minimal version open
            setVisible (true);
            inspectorComponent.toggle (newStatus);

            updateWindowSize();
        }

        void toggle()
        {
            toggle (!enabled);
        }

        std::function<void ()> onClose;

    private:
        // LNF has to be declared before components using it
        juce::OptionalScopedPointer<juce::PropertiesFile> settings;

        melatonin::InspectorLookAndFeel inspectorLookAndFeel;

        melatonin::InspectorComponent inspectorComponent;
        juce::Component& root;
        bool enabled;
        melatonin::Overlay overlay;
        melatonin::MouseInspector mouseInspector { root };
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
            mouseInspector.outlineComponentCallback = [this] (Component* c) { this->outlineComponent (c); };
            mouseInspector.outlineDistanceCallback = [this] (Component* c) { this->outlineDistanceCallback (c); };
            mouseInspector.selectComponentCallback = [this] (Component* c) { this->selectComponent (c, true); };
            mouseInspector.componentStartDraggingCallback = [this] (Component* c, const juce::MouseEvent& e) { this->startDragComponent (c, e); };
            mouseInspector.componentDraggedCallback = [this] (Component* c, const juce::MouseEvent& e) { this->dragComponent (c, e); };
            mouseInspector.mouseExitCallback = [this]() { if (this->enabled) inspectorComponent.redisplaySelectedComponent(); };

            inspectorComponent.selectComponentCallback = [this] (Component* c) { this->selectComponent (c, true); };
            inspectorComponent.outlineComponentCallback = [this] (Component* c) { this->outlineComponent (c); };
            inspectorComponent.toggleCallback = [this] (bool enable) { this->toggle (enable); };
        }
    };
}
