#pragma once
#include "helpers.h"

namespace melatonin
{
    class Overlay;

    class ComponentTreeViewItem
        : public juce::TreeViewItem,
          public juce::MouseListener,
          public juce::ComponentListener
    {
    public:
        bool hasTabbedComponent = false;

        explicit ComponentTreeViewItem (juce::Component* c, std::function<void (juce::Component* c)> outline, std::function<void (juce::Component* c)> select)
            : outlineComponentCallback (outline), selectComponentCallback (select), component (c)
        {
            // A few JUCE component types need massaging to get their child components
            if (auto multiPanel = dynamic_cast<juce::MultiDocumentPanel*> (c))
            {
                recursivelyAddChildrenFor (multiPanel->getCurrentTabbedComponent());
            }
            else if (auto tabs = dynamic_cast<juce::TabbedComponent*> (c))
            {
                hasTabbedComponent = true;
                for (int i = 0; i < tabs->getNumTabs(); ++i)
                {
                    recursivelyAddChildrenFor (tabs->getTabContentComponent (i));
                }
            }
            else
            {
                addItemsForChildComponents();
            }

            setDrawsInLeftMargin(true);

            // Make our tree self-aware
            component->addComponentListener (this);
        }

        ~ComponentTreeViewItem() override
        {
            // The component can be deleted before this tree view item
            if (component)
                component->removeComponentListener (this);
        }

        bool mightContainSubItems() override
        {
            return component != nullptr && (component->getNumChildComponents() > 0);
        }

        // naive but functional...
        void openTreeAndSelect (juce::Component* target)
        {
            // don't let us select something already selected
            if (component == target && !isSelected())
            {
                setSelected (true, true, juce::dontSendNotification);
                setOpen (true);
            }
            else if (component->isParentOf (target))
            {
                jassert (target);
                setOpen (true);
                // recursively open up tree to get at target
                for (int i = 0; i < getNumSubItems(); ++i)
                {
                    dynamic_cast<ComponentTreeViewItem*> (getSubItem (i))->openTreeAndSelect (target);
                }
            }
        }

        void paintItem (juce::Graphics& g, int w, int h) override
        {
            if (!component)
                return;
            if (isSelected())
            {
                g.setColour (color::blackColor);
                g.fillRect (3, 0, w - 30, h);
            }

            g.setColour (color::blueLabelTextColor);
            if (!component->isVisible())
                g.setColour (juce::Colours::grey);

            auto textIndent = mightContainSubItems() ? 7 : 5;
            auto wantKeyboardFocus = component->getWantsKeyboardFocus();
            juce::String keyboard = wantKeyboardFocus ? " (wantsKeyboard)" : "";

            if (wantKeyboardFocus)
                g.setColour (isSelected() ? color::redLineColor.darker (0.7f) : color::redLineColor);
            g.setFont (juce::Font ("Verdana", 14, juce::Font::FontStyleFlags::plain));
            g.drawText (componentString (component) + keyboard, textIndent, 0, w - textIndent, h, juce::Justification::left, true);
        }

        // must override to set the disclosure triangle color
        void paintOpenCloseButton (juce::Graphics& g, const juce::Rectangle<float>& area, juce::Colour /*backgroundColour*/, bool isMouseOver) override
        {
            getOwnerView()->getLookAndFeel().drawTreeviewPlusMinusBox (g, area, color::blueLabelBackgroundColor, isOpen(), isMouseOver);
        }

        void itemClicked (const juce::MouseEvent&) override
        {
            selectComponentCallback (component);
            selectTabbedComponentChildIfNeeded();
            if (mightContainSubItems())
                setOpen (true);
        }

        void mouseEnter (const juce::MouseEvent& /*event*/) override
        {
            outlineComponentCallback (component);
        }

        void recursivelyCloseSubItems()
        {
            for (int i = 0; i < getNumSubItems(); ++i)
            {
                getSubItem (i)->setOpen (false);
                dynamic_cast<ComponentTreeViewItem*> (getSubItem (i))->recursivelyCloseSubItems();
            }
        }

        void filterNodesRecursively (const juce::String& searchString)
        {
            // Iterate over the child nodes of the current node
            for (int i = getNumSubItems() - 1; i >= 0; --i)
            {
                // Recursively call the function on the current child node
                if (auto* ct = dynamic_cast<ComponentTreeViewItem*> (getSubItem (i)))
                {
                    ct->filterNodesRecursively (searchString);
                }
            }
            // Check if the current node's name does not contain the search string
            if (!getComponentName().containsIgnoreCase(searchString))
            {
                // Remove the subtree rooted at the current node
                if(getParentItem() != nullptr && getNumSubItems() == 0)
                {
                    getParentItem()->removeSubItem (getIndexInParent());
                    DBG("For removal: "<< getComponentName());
                }
                else
                    setOpen (true);
            }
            else if(getComponentName().startsWithIgnoreCase(searchString))
            {
                outlineComponentCallback (component);
                setSelected(true, true);
                setOpen (true);
            }
            else {
                setOpen (true);
            }
        }

        // Callback from the component listener. Reconstruct children when component is deleted
        void componentChildrenChanged (juce::Component& /*changedComponent*/) override
        {
            validateSubItems();
        }

        juce::String getComponentName()
        {
            juce::String res = "";
            if (component && !component->getName().isEmpty())
            {
                return component->getName();
            }
            else if (component)
            {
                return type (*component);
            }
            return res;
        }

        std::function<void (juce::Component* c)> outlineComponentCallback;
        std::function<void (juce::Component* c)> selectComponentCallback;

    private:
        juce::Component::SafePointer<juce::Component> component;
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ComponentTreeViewItem)

        void recursivelyAddChildrenFor (juce::Component* child)
        {
            // Components such as Labels can have a nullptr component child
            // Rather than display empty placeholders in the tree view, we will hide them
            if (child)
                addSubItem (new ComponentTreeViewItem (child, outlineComponentCallback, selectComponentCallback));
        }

        void addItemsForChildComponents()
        {
            for (int i = 0; i < component->getNumChildComponents(); ++i)
            {
                auto child = component->getChildComponent (i);
                if (componentString (child) != "Melatonin Overlay")
                    recursivelyAddChildrenFor (child);
            }
        }

        void validateSubItems()
        {
            for (int i = 0; i < getNumSubItems(); ++i)
            {
                // Ideally we'd just re-render the sub-items branch:
                // auto subItemToValidate = dynamic_cast<ComponentTreeViewItem*> (getSubItem (i));

                // However, that wasn't working so the scorched earth strategy is
                // if any child has a deleted component, we re-render the whole branch
                // (we don't explicitly know if things were added or removed)
                clearSubItems();
                addItemsForChildComponents();
                break;
            }
        }

        void selectTabbedComponentChildIfNeeded()
        {
            if (!getParentItem())
                return;

            auto parent = dynamic_cast<ComponentTreeViewItem*> (getParentItem());
            if (parent->hasTabbedComponent && !component->isVisible())
            {
                dynamic_cast<juce::TabbedComponent*> (parent->component.getComponent())->setCurrentTabIndex (getIndexInParent());
            }
        }
    };
}
