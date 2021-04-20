#pragma once
#include "box_model.h"
#include "component_tree_view_item.h"

namespace melatonin
{
    class InspectorPanel : public Component, public Button::Listener
    {
    public:
        InspectorPanel (Component& rootComponent, Component& overlay, bool enabledAtStart = true) : root (rootComponent)
        {
            addAndMakeVisible (tree);
            addAndMakeVisible (boxModel);
            toggleButton.setButtonText ("Enabled");
            toggleButton.setToggleState (enabledAtStart, dontSendNotification);
            addAndMakeVisible (toggleButton);
            toggleButton.addListener (this);
        }

        void paint (Graphics& g) override
        {
            //g.drawRect ()
        }

        void reconstructRoot()
        {
            jassert (selectComponentCallback);
            if (rootSet)
                tree.deleteRootItem();
            rootSet = true;
            tree.setRootItem (new ComponentTreeViewItem (&root, outlineComponentCallback, selectComponentCallback));
            getRoot()->setOpenness (ComponentTreeViewItem::Openness::opennessOpen);
        }

        void resized() override
        {
            auto area = getLocalBounds();
            area.removeFromTop (20);
            toggleButton.setBounds (area.removeFromTop (20).withTrimmedLeft (27));
            boxModel.setBounds (area.removeFromTop (250));
            tree.setBounds (area.withTrimmedLeft (25)); // 3px discrepancy from normal padding
        }

        void displayComponentInfo (Component* component)
        {
            if (! rootSet)
                reconstructRoot();
            boxModel.setComponent (component);
            repaint();
        }

        void redisplaySelectedComponent()
        {
            if (selectedComponent)
            {
                displayComponentInfo (selectedComponent);
            }
        }

        void selectComponent (Component* component, bool collapseTreeBeforeSelection)
        {
            selectedComponent = component;
            if (collapseTreeBeforeSelection)
            {
                getRoot()->recursivelyCloseSubItems();
            }
            getRoot()->openTreeAndSelect (component);

            tree.scrollToKeepItemVisible (dynamic_cast<ComponentTreeViewItem*> (tree.getSelectedItem (0)));
        }

        void deselectComponent()
        {
            selectedComponent = nullptr;
            deselectComponentCallback();
        }

        void buttonClicked (juce::Button* button) override
        {
            if (button == &toggleButton)
            {
                auto enabled = toggleButton.getToggleState();
                toggleCallback (enabled);
                tree.setVisible (enabled);
            }
        }

        std::function<void (Component* c)> selectComponentCallback;
        std::function<void (Component* c)> outlineComponentCallback;
        std::function<void (bool enabled)> toggleCallback;
        std::function<void ()> deselectComponentCallback;

    private:
        Component::SafePointer<Component> selectedComponent;
        Component& root;
        ToggleButton toggleButton;
        BoxModel boxModel;
        TreeView tree;
        bool rootSet = false;

        ComponentTreeViewItem* getRoot()
        {
            return dynamic_cast<ComponentTreeViewItem*> (tree.getRootItem());
        }
    };
}
