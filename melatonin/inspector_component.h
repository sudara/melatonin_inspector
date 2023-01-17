#pragma once
#include "melatonin_inspector/melatonin/components/box_model.h"
#include "melatonin_inspector/melatonin/components/color_picker.h"
#include "melatonin_inspector/melatonin/components/component_tree_view_item.h"
#include "melatonin_inspector/melatonin/components/preview_panel.h"
#include "melatonin_inspector/melatonin/components/properties.h"
#include "melatonin_inspector/melatonin/lookandfeel.h"

/*
 * Right now this unfortunately bundles all inspector components
 * as well as the tree view and selection logic.
 */

namespace melatonin
{
    class InspectorComponent : public juce::Component, public juce::Button::Listener
    {
    public:
        explicit InspectorComponent (juce::Component& rootComponent, bool enabledAtStart = true) : root (rootComponent)
        {
            addChildComponent (tree);
            addChildComponent (emptySearchLabel);

            colorPicker.setRootComponent (&root);

            emptySelectionPrompt.setJustificationType (juce::Justification::centredTop);
            emptySearchLabel.setJustificationType (juce::Justification::centredTop);

            addAndMakeVisible (colorPickerPanel);
            addAndMakeVisible (previewComponentPanel);
            addAndMakeVisible (propertiesPanel);

            // visibility of everything but boxModel is managed by the toggle in the above panels
            addAndMakeVisible(boxModel);
            addAndMakeVisible(colorPicker);
            addAndMakeVisible(previewComponent);
            addAndMakeVisible(properties);

            toggleButton.setButtonText ("Enable inspector");
            toggleButton.setColour (juce::TextButton::textColourOffId, colors::titleTextColor);
            toggleButton.setColour (juce::TextButton::textColourOnId, colors::yellowColor);
            toggleButton.setToggleState (enabledAtStart, juce::dontSendNotification);

            addAndMakeVisible (toggleButton);
            toggleButton.addListener (this);
            tree.setIndentSize (12);

            searchBox.setHelpText ("search");
            searchBox.setColour (juce::Label::backgroundColourId, juce::Colours::transparentBlack);
            searchBox.setColour (juce::Label::textColourId, colors::searchTextLabelColor);
            searchBox.setColour (juce::TextEditor::outlineColourId, juce::Colours::transparentBlack);
            searchBox.setColour (juce::TextEditor::focusedOutlineColourId, juce::Colours::transparentBlack);
            searchBox.setTextToShowWhenEmpty ("Search for component...", colors::treeViewMinusPlusColor);
            searchBox.setJustification (juce::Justification::centredLeft);

            addAndMakeVisible (searchBox);
            addAndMakeVisible (searchIcon);
            addChildComponent (clearBtn);

            searchBox.onTextChange = [this] {
                auto searchText = searchBox.getText();
                reconstructRoot();

                // try to find the first item that matches the search string
                if (searchText.isNotEmpty())
                {
                    getRoot()->filterNodesRecursively (searchText);
                }

                // display empty label
                if (getRoot()->getNumSubItems() == 0
                    && !searchText.containsIgnoreCase (getRoot()->getComponentName())
                    && tree.getNumSelectedItems() == 0)
                {
                    tree.setVisible (false);
                    emptySearchLabel.setVisible (true);

                    resized();
                }
                else
                {
                    tree.setVisible (true);
                    emptySearchLabel.setVisible (false);
                }

                clearBtn.setVisible (searchBox.getText().isNotEmpty());
            };
            clearBtn.onClick = [this] {
                searchBox.setText ("");
            };

            // the tree view is empty even if inspector is enabled
            // since at the moment when this panel getting initialized, the root component most likely doesn't have any children YET
            // we can either wait and launch async update or add empty label
        }

        void paint (juce::Graphics& g) override
        {
            g.setColour (colors::blackColor.withAlpha (0.38f));
            g.fillRect (topArea);

            g.setColour (colors::blackColor);
            g.fillRect (searchBoxBounds);

            // Define the colors for the gradient
            juce::Colour endColor (34, 17, 46);
            juce::Colour startColor (24, 10, 35);

            // Fill the background with a gradient from top left to bottom right
            g.setGradientFill (juce::ColourGradient (
                startColor, (float) treeViewBounds.getX(), (float) treeViewBounds.getY(), endColor, (float) treeViewBounds.getWidth(), (float) treeViewBounds.getHeight(), false));
            g.fillRect (treeViewBounds);
        }

        void reconstructRoot()
        {
            jassert (selectComponentCallback);
            if (rootItem)
                tree.setRootItem (nullptr);
            rootItem = std::make_unique<ComponentTreeViewItem> (&root, outlineComponentCallback, selectComponentCallback);
            tree.setRootItem (rootItem.get());
            getRoot()->setOpenness (ComponentTreeViewItem::Openness::opennessOpen);

            tree.setVisible (true);

            resized();
        }

        void resized() override
        {
            DBG("InspectorComponent::resized");
            auto area = getLocalBounds();
            int padding = 8;

            auto inspectorEnabled = toggleButton.getToggleState();
            auto columnMinWidth = inspectorEnabled ? juce::jmax (380, area.getWidth() / 2)
                                                   : getWidth();

            auto mainCol = area.removeFromLeft (columnMinWidth);

            auto toggleBtnHeight = 48;

            topArea = mainCol.removeFromTop (toggleBtnHeight);

            toggleButton.setBounds (topArea.withX (padding));

            boxModel.setBounds(mainCol.removeFromTop (300));
            D(boxModel.getBounds().toString());

            previewComponentPanel.setBounds (mainCol.removeFromTop (32));
            previewComponent.setBounds(mainCol.removeFromTop (previewComponent.isVisible() ? 100 : 0));

            auto colorBounds = mainCol.removeFromTop (32 + 40);
            if (colorPicker.isVisible())
            {
                // we have an icon in the panel header, so we overlap it
                colorPicker.setBounds(colorBounds);
            }
            colorPickerPanel.setBounds (colorBounds.removeFromTop (32));


            propertiesPanel.setBounds (mainCol.removeFromTop(32));
            properties.setBounds (mainCol);

            // using btn toggle state (better to switch to using class variable
            // or inspectors prop)

            searchBoxBounds = area.removeFromTop (toggleBtnHeight);
            auto b = searchBoxBounds;

            clearBtn.setBounds (b.removeFromRight (48));
            searchIcon.setBounds (b.removeFromLeft (48));
            searchBox.setBounds (b);

            emptySearchLabel.setBounds (area.reduced (4));

            // used to paint the background
            treeViewBounds = area;

            if (tree.isVisible())
            {
                tree.setBounds (area.withTrimmedLeft(18).withTrimmedTop(24));
            }
            else
                emptySelectionPrompt.setBounds (area.withTrimmedLeft(18).withTrimmedTop(24));
        }

        void displayComponentInfo (Component* component)
        {
            if (!rootItem)
                reconstructRoot();

            // only show on hover if there isn't something selected
            if (!selectedComponent || selectedComponent == component)
            {
                model.displayComponent (component);

                repaint();
                resized();

                // Selects and highlights
                if (component != nullptr)
                {
                    // getRoot()->recursivelyCloseSubItems();

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

            // update value in the model
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

                colorPicker.reset();

                resized();
            }
        }

        void toggle (bool enabled)
        {
            toggleButton.setToggleState (enabled, juce::dontSendNotification);

            previewComponent.setVisible (enabled);
            colorPicker.setVisible (enabled);
            properties.setVisible (enabled);

            if (!enabled)
                model.deselectComponent();
        }

        std::function<void (Component* c)> selectComponentCallback;
        std::function<void (Component* c)> outlineComponentCallback;
        std::function<void (bool enabled)> toggleCallback;

    private:
        Component::SafePointer<Component> selectedComponent;
        Component& root;

        juce::ToggleButton toggleButton;

        ComponentModel model;

        juce::Rectangle<int> topArea, searchBoxBounds, treeViewBounds;

        BoxModel boxModel { model };

        ColorPicker colorPicker;
        CollapsablePanel colorPickerPanel { "COLOR", &colorPicker };

        Preview previewComponent { model };
        CollapsablePanel previewComponentPanel { "PREVIEW", &previewComponent };

        Properties properties { model };
        CollapsablePanel propertiesPanel { "PROPERTIES", &properties };

        // todo move to its own component
        juce::TreeView tree;
        juce::Label emptySelectionPrompt { "SelectionPrompt", "Select any component to see components tree" };
        juce::Label emptySearchLabel { "EmptySearchResultsPrompt", "No component found" };
        juce::TextEditor searchBox { "Search box" };
        juce::TextButton clearBtn { "CL", "CL" };
        juce::TextButton searchIcon { "S", "S" };
        std::unique_ptr<ComponentTreeViewItem> rootItem;

        ComponentTreeViewItem* getRoot()
        {
            return dynamic_cast<ComponentTreeViewItem*> (tree.getRootItem());
        }

        void deselectComponent()
        {
            selectedComponent = nullptr;
            tree.clearSelectedItems();

            colorPicker.reset();

            model.deselectComponent();
            tree.setRootItem (getRoot());

            resized();
        }

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (InspectorComponent)
    };
}
