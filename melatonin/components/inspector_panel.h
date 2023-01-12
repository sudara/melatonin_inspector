#pragma once
#include "box_model.h"
#include "melatonin_inspector/melatonin/color_model.h"
#include "melatonin_inspector/melatonin/component_tree_view_item.h"
#include "melatonin_inspector/melatonin/lookandfeel.h"
#include "preview_model.h"
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

            colorModel.setRootComponent (&root);

            emptySelectionPrompt.setJustificationType (juce::Justification::centredTop);
            emptySearchLabel.setJustificationType (juce::Justification::centredTop);

            addAndMakeVisible (boxModel);
            addChildComponent (colorModel);
            addChildComponent (propertiesModel);
            addChildComponent (previewComponent);

            colorModel.setVisible (enabledAtStart);
            propertiesModel.setVisible (enabledAtStart);
            previewComponent.setVisible (enabledAtStart);

            toggleButton.setButtonText ("Enabled inspector");
            toggleButton.setColour (juce::TextButton::textColourOffId, colors::titleTextColor);
            toggleButton.setColour (juce::TextButton::textColourOnId, colors::yellowColor);
            toggleButton.setLookAndFeel (&toggleBtnLookAndFeel);
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

                //try to find the first item that matches the search string
                if (searchText.isNotEmpty())
                {
                    getRoot()->filterNodesRecursively (searchText);
                }

                //display empty label
                if (getRoot()->getNumSubItems() == 0
                    && !searchText.containsIgnoreCase (getRoot()->getComponentName())
                    && tree.getNumSelectedItems() == 0)
                {
                    DBG (getRoot()->getComponentName());
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
            juce::Colour startColor (0xFF22112E);
            juce::Colour endColor (0xFF180A23);

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
            auto area = getLocalBounds();

            auto inspectorEnabled = toggleButton.getToggleState();
            auto columnMinWidth = inspectorEnabled ? juce::jmax (380, area.getWidth() / 2)
                                                   : getWidth();

            auto mainCol = area.removeFromLeft (columnMinWidth);

            auto toggleBtnHeight = 48;
            auto logoSide = 48;
            //used for toggle btn overlay and logo
            topArea = juce::Rectangle<int> (mainCol).removeFromTop (toggleBtnHeight);

            toggleButton.setBounds (mainCol
                                        .removeFromTop (toggleBtnHeight)
                                        .withSize (mainCol.getWidth() - logoSide, toggleBtnHeight)
                                        .withX (8));

            boxModel.resized();
            boxModel.setBounds (mainCol.removeFromTop (boxModel.getHeight())
                                    .withX (mainCol.getX())
                                    .withWidth (mainCol.getWidth()));

            previewComponent.resized();
            previewComponent.setBounds (mainCol.removeFromTop (static_cast<int> (previewComponent.getHeight()))
                                            .withX (mainCol.getX())
                                            .withWidth (mainCol.getWidth()));

            colorModel.resized();
            colorModel.setBounds (mainCol.removeFromTop (static_cast<int> (colorModel.getHeight()))
                                      .withX (mainCol.getX())
                                      .withWidth (mainCol.getWidth()));

            propertiesModel.resized();
            propertiesModel.setBounds (mainCol);

            //using btn toggle state (better to switch to using class variable
            //or inspectors prop)

            searchBoxBounds = area.removeFromTop (toggleBtnHeight);
            auto b = searchBoxBounds;

            clearBtn.setBounds (b.removeFromRight (48));
            searchIcon.setBounds (b.removeFromLeft (48));
            searchBox.setBounds (b);

            emptySearchLabel.setBounds (area.reduced (4));

            treeViewBounds = area;

            if (tree.isVisible())
            {
                tree.setBounds (area); // padding in these default components are a mess
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
                model.displayComponent (component);

                repaint();
                resized();

                //Selects and highlights
                if (component != nullptr)
                {
                    //getRoot()->recursivelyCloseSubItems();

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

                colorModel.reset();

                resized();
            }
        }

        void toggle (bool enabled)
        {
            toggleButton.setToggleState (enabled, juce::dontSendNotification);

            previewComponent.setVisible (enabled);
            colorModel.setVisible (enabled);
            propertiesModel.setVisible (enabled);

            if (!enabled)
                model.deselectComponent();
        }

        std::function<void (Component* c)> selectComponentCallback;
        std::function<void (Component* c)> outlineComponentCallback;
        std::function<void (bool enabled)> toggleCallback;

    private:
        struct ToggleBtnLookAndFeel : InspectorLookAndFeel
        {
        public:
            void drawToggleButton (juce::Graphics& g, juce::ToggleButton& button, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
            {
                auto fontSize = juce::jmin (15.0f, (float) button.getHeight() * 0.75f);
                auto tickWidth = fontSize * 1.1f;

                drawTickBox (g, button, 4.0f, ((float) button.getHeight() - tickWidth) * 0.5f, tickWidth, tickWidth, button.getToggleState(), button.isEnabled(), shouldDrawButtonAsHighlighted, shouldDrawButtonAsDown);

                //flip between text textColourOnId and textColourOffId
                if (button.getToggleState())
                {
                    g.setColour (button.findColour (juce::TextButton::textColourOnId));
                }
                else
                    g.setColour (button.findColour (juce::TextButton::textColourOffId));
                g.setFont (fontSize);

                if (!button.isEnabled())
                    g.setOpacity (0.5f);

                g.drawFittedText (button.getButtonText(),
                    button.getLocalBounds().withTrimmedLeft (juce::roundToInt (tickWidth) + 10).withTrimmedRight (2),
                    juce::Justification::centredLeft,
                    10);
            }
        };

        Component::SafePointer<Component> selectedComponent;
        Component& root;

        ToggleBtnLookAndFeel toggleBtnLookAndFeel;
        juce::ToggleButton toggleButton;

        ComponentModel model;

        juce::Rectangle<int> topArea, searchBoxBounds, treeViewBounds;

        BoxModel boxModel { model };
        ColorModel colorModel;
        ComponentPreviewModel previewComponent { model };
        PropertiesModel propertiesModel { model };

        //todo move to it's own component
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

            colorModel.reset();

            model.deselectComponent();
            tree.setRootItem (getRoot());

            resized();
        }

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (InspectorPanel)
    };
}
