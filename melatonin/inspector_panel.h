#pragma once
#include "box_model.h"
#include "component_tree_view_item.h"
#include "properties_model.h"

namespace melatonin
{
    class InspectorPanel : public juce::Component, public juce::Button::Listener
    {
    public:
        explicit InspectorPanel (juce::Component& rootComponent, bool enabledAtStart = true) : root (rootComponent)
        {
            addAndMakeVisible (tree);
            addAndMakeVisible (boxModel);
            addAndMakeVisible (propertiesModel);
            toggleButton.setButtonText ("Enabled");
            toggleButton.setToggleState (enabledAtStart, juce::dontSendNotification);
            addAndMakeVisible (toggleButton);
            toggleButton.addListener (this);
            tree.setIndentSize (12);
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

            auto columnMinWidth = juce::jmax(380, area.getWidth() / 2);
            area.removeFromTop (20);

            auto mainCol = area.removeFromLeft(columnMinWidth);
            toggleButton.setBounds (mainCol.removeFromTop (20).withTrimmedLeft (27));
            boxModel.setBounds (mainCol.removeFromTop (250));
            propertiesModel.setBounds (mainCol.removeFromTop (250));
            tree.setBounds (area); // padding in these default components are a mess
        }

        void displayComponentInfo (Component* component)
        {
            if (!rootSet)
                reconstructRoot();

            // only show on hover if there isn't something selected
            if (!selectedComponent || selectedComponent == component)
            {
                boxModel.displayComponent (component);
                propertiesModel.displayComponent (component);
                repaint();
            }
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
            if (component && selectedComponent == component)
            {
                deselectComponent();
                return;
            }

            selectedComponent = component;
            displayComponentInfo (selectedComponent);
            if (collapseTreeBeforeSelection)
            {
                getRoot()->recursivelyCloseSubItems();
            }
            getRoot()->openTreeAndSelect (component);

            tree.scrollToKeepItemVisible (tree.getSelectedItem(0));
        }

        void buttonClicked (juce::Button* button) override
        {
            if (button == &toggleButton)
            {
                auto enabled = toggleButton.getToggleState();
                toggle (enabled);
                toggleCallback (enabled);
                tree.setVisible (enabled);
                boxModel.reset();
                propertiesModel.reset();
            }
        }

        void toggle (bool enabled)
        {
            toggleButton.setToggleState (enabled, juce::dontSendNotification);
        }

        std::function<void (Component* c)> selectComponentCallback;
        std::function<void (Component* c)> outlineComponentCallback;
        std::function<void (bool enabled)> toggleCallback;

    private:
        Component::SafePointer<Component> selectedComponent;
        Component& root;
        juce::ToggleButton toggleButton;
        BoxModel boxModel;
        PropertiesModel propertiesModel;
        juce::TreeView tree;
        bool rootSet = false;

        ComponentTreeViewItem* getRoot()
        {
            return dynamic_cast<ComponentTreeViewItem*> (tree.getRootItem());
        }

        void deselectComponent()
        {
            selectedComponent = nullptr;
            tree.clearSelectedItems();
            boxModel.reset();
            propertiesModel.reset();
        }
    };
}
