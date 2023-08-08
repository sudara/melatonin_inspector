#pragma once

#include "component_tree_view_item.h"
#include "melatonin_inspector/melatonin/component_model.h"

namespace melatonin
{
    class TreeComponent : public juce::Component, private ComponentModel::Listener
    {
    public:
        explicit TreeComponent (ComponentModel& _model) : model (_model)
        {
            model.addListener (*this);
        }

        ~TreeComponent() override
        {
            model.removeListener (*this);
        }

        void resized() override
        {
            auto area = getLocalBounds();

            if (tree.isVisible())
            {
                auto searchRow = area.removeFromTop (30).reduced (4, 4);
                clearBtn.setBounds (searchRow.removeFromRight (56));
                searchRow.removeFromRight (8);
                searchBox.setBounds (searchRow);

                // padding in stock components are a mess
                tree.setBounds (area);

                emptySearchLabel.setBounds (area.reduced (4));
            }
            else
                emptySelectionPrompt.setBounds (area);
        }

    private:
        ComponentModel& model;

        juce::TreeView tree;
        juce::Label emptySelectionPrompt { "SelectionPrompt", "Select any component to see components tree" };
        juce::Label emptySearchLabel { "EmptySearchResultsPrompt", "No component found" };
        juce::TextEditor searchBox { "Search box" };
        juce::TextButton clearBtn { "clear" };
        std::unique_ptr<ComponentTreeViewItem> rootItem;
    };
}
