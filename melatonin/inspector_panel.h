#pragma once
#include "box_model.h"
#include "component_tree_view_item.h"
#include "properties_model.h"
#include "color_model.h"

namespace melatonin
{
    class InspectorPanel : public juce::Component, public juce::Button::Listener
    {
    public:
        explicit InspectorPanel (juce::Component& rootComponent, bool enabledAtStart = true) : root (rootComponent)
        {
            addChildComponent (tree);
            addAndMakeVisible (emptySelectionPrompt);
            emptySelectionPrompt.setColour (juce::Label::textColourId, color::white);
            emptySelectionPrompt.setJustificationType (juce::Justification::centredTop);
            addAndMakeVisible (boxModel);
            addAndMakeVisible (colorModel);
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
            if (rootItem)
                tree.deleteRootItem();
            rootItem = std::make_unique<ComponentTreeViewItem>(&root, outlineComponentCallback, selectComponentCallback);
            tree.setRootItem (rootItem.get());
            getRoot()->setOpenness (ComponentTreeViewItem::Openness::opennessOpen);
        }

        void resized() override
        {
            using namespace juce;
            auto area = getLocalBounds();

            auto inspectorEnabled = toggleButton.getToggleState();
            auto columnMinWidth = inspectorEnabled ? juce::jmax (380, area.getWidth() / 2)
                                                   : getWidth();
            area.removeFromTop (20);

            auto mainCol = area.removeFromLeft (columnMinWidth);
            toggleButton.setBounds (mainCol.removeFromTop (20).withTrimmedLeft (27));
            boxModel.setBounds (mainCol.removeFromTop (250));
            colorModel.setBounds (mainCol.removeFromTop(static_cast<int> (jmin (180.0, mainCol.getHeight() * 0.25))).withTrimmedBottom(4));
            propertiesModel.setBounds (mainCol);

            //using btn toggle state (better to switch to using class variable
            //or inspectors prop)

            if (tree.isVisible())
                tree.setBounds (area); // padding in these default components are a mess
            else
                emptySelectionPrompt.setBounds (area);
        }

        void displayComponentInfo (Component* component)
        {
            if (!rootItem)
                reconstructRoot();

            // only show on hover if there isn't something selected
            if (!selectedComponent || selectedComponent == component)
            {
                boxModel.displayComponent (component);
                propertiesModel.displayComponent (component);
                colorModel.displayComponent(component);

                emptySelectionPrompt.setVisible (false);
                tree.setVisible (true);

                repaint();
                resized();

                //Selects and highlights
                if(component != nullptr)
                {
                    getRoot()->recursivelyCloseSubItems();

                    getRoot()->openTreeAndSelect (component);
                    tree.scrollToKeepItemVisible(tree.getSelectedItem(0));
                }
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

            //update value in the model
            model.selectComponent(component);

            displayComponentInfo (selectedComponent);
            if (collapseTreeBeforeSelection)
            {
                getRoot()->recursivelyCloseSubItems();
            }
            getRoot()->openTreeAndSelect (component);

            tree.scrollToKeepItemVisible (tree.getSelectedItem (0));
        }

        void buttonClicked (juce::Button* button) override
        {
            if (button == &toggleButton)
            {
                auto enabled = toggleButton.getToggleState();
                toggle (enabled);
                toggleCallback (enabled);

                auto hasSelected = selectedComponent != nullptr;
                emptySelectionPrompt.setVisible (!hasSelected);
                tree.setVisible (hasSelected);

                boxModel.reset();
                propertiesModel.reset();
                colorModel.reset();

                resized();
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
        ComponentModel model;
        BoxModel boxModel;
        ColorModel colorModel;
        PropertiesModel propertiesModel{model};
        juce::TreeView tree;
        juce::Label emptySelectionPrompt { "SelectionPrompt", "Select any component to see components tree" };
        std::unique_ptr<ComponentTreeViewItem> rootItem;

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
            colorModel.reset();

            emptySelectionPrompt.setVisible (true);
            tree.setVisible (false);

            model.deselectComponent();

            resized();
        }
    };
}
