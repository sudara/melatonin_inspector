#pragma once
#include "box_model.h"
#include "color_model.h"
#include "component_preview.h"
#include "component_tree_view_item.h"
#include "lookandfeel.h"
#include "properties_model.h"

namespace melatonin
{
    class InspectorPanel : public juce::Component, public juce::Button::Listener
    {
    public:
        explicit InspectorPanel (juce::Component& rootComponent, bool enabledAtStart = true) : root (rootComponent)
        {
            addChildComponent (tree);
            addChildComponent (emptySearchLabel);

            //searchBox.setLookAndFeel(&melatoninLookAndFeel);
            //clearBtn.setLookAndFeel(&melatoninLookAndFeel);
            //emptySelectionPrompt.setLookAndFeel(&melatoninLookAndFeel);

            emptySelectionPrompt.setJustificationType (juce::Justification::centredTop);
            emptySearchLabel.setJustificationType (juce::Justification::topLeft);

            addAndMakeVisible (boxModel);
            addAndMakeVisible (colorModel);
            addAndMakeVisible (propertiesModel);
            addAndMakeVisible (previewComponent);

            toggleButton.setButtonText ("Enabled");
            toggleButton.setToggleState (enabledAtStart, juce::dontSendNotification);

            addAndMakeVisible (toggleButton);
            toggleButton.addListener (this);
            tree.setIndentSize (12);

            searchBox.setHelpText ("search");
            searchBox.setTextToShowWhenEmpty("search", juce::Colours::white);
            addAndMakeVisible (searchBox);
            addAndMakeVisible (clearBtn);

            searchBox.onTextChange = [this] {
                auto searchText = searchBox.getText();
                reconstructRoot();

                //try to find the first item that matches the search string
                if(searchText.isNotEmpty()){
                    getRoot()->filterNodesRecursively (searchText);
                }

                //display empty label
                if(getRoot()->getNumSubItems() == 0
                    && !searchText.containsIgnoreCase(getRoot()->getComponentName())
                    && tree.getNumSelectedItems() == 0)
                {
                    DBG(getRoot()->getComponentName());
                    tree.setVisible(false);
                    emptySearchLabel.setVisible(true);

                    resized();
                }

                clearBtn.setVisible(searchBox.getText().isNotEmpty());
                tree.setVisible(searchBox.getText().isNotEmpty());
            };
            clearBtn.onClick = [this]{
                searchBox.setText("");
            };

        }

        void reconstructRoot()
        {
            jassert (selectComponentCallback);
            if (rootItem)
                tree.setRootItem(nullptr);
            rootItem = std::make_unique<ComponentTreeViewItem> (&root, outlineComponentCallback, selectComponentCallback);
            tree.setRootItem (rootItem.get());
            getRoot()->setOpenness (ComponentTreeViewItem::Openness::opennessOpen);

            tree.setVisible(true);

            resized();
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
            toggleButton.setBounds (mainCol.removeFromTop (20).withTrimmedLeft (8));
            boxModel.setBounds (mainCol.removeFromTop (250));

            previewComponent.resized();
            previewComponent.setBounds (mainCol.removeFromTop (static_cast<int> (previewComponent.getHeight()))
                                            .withX (mainCol.getX()).withWidth (mainCol.getWidth()));

            colorModel.setBounds (mainCol.removeFromTop (static_cast<int> (jmin (180.0, mainCol.getHeight() * 0.25))).withTrimmedBottom (4));
            propertiesModel.setBounds (mainCol);

            //using btn toggle state (better to switch to using class variable
            //or inspectors prop)

            if (tree.isVisible())
            {
                auto searchRow = area.removeFromTop (30).reduced(4, 4);
                clearBtn.setBounds (searchRow.removeFromRight(56));
                searchRow.removeFromRight(8);
                searchBox.setBounds (searchRow);

                tree.setBounds (area); // padding in these default components are a mess
                emptySearchLabel.setBounds(area.reduced(4));
            }
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
                colorModel.displayComponent (component);

                tree.setVisible (true);

                repaint();
                resized();

                //Selects and highlights
                if (component != nullptr)
                {
                    getRoot()->recursivelyCloseSubItems();

                    getRoot()->openTreeAndSelect (component);
                    tree.scrollToKeepItemVisible (tree.getSelectedItem (0));
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
            model.selectComponent (component);

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
        BoxModel boxModel{ model };
        ColorModel colorModel;
        ComponentPreviewChild previewComponent{ model};
        PropertiesModel propertiesModel { model };

        //todo move to it's own component
        juce::TreeView tree;
        juce::Label emptySelectionPrompt { "SelectionPrompt", "Select any component to see components tree" };
        juce::Label emptySearchLabel { "EmptySearchResultsPrompt", "No component found" };
        juce::TextEditor searchBox { "Search box" };
        juce::TextButton clearBtn { "clear"};
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

            model.deselectComponent();
            tree.setRootItem(getRoot());

            resized();
        }
    };
}
