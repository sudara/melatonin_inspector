#pragma once
#include "helpers.h"

namespace melatonin
{
    class Overlay;

    class ComponentTreeViewItem
        : public TreeViewItem,
          public MouseListener,
          public ComponentListener
    {
    public:
        bool hasTabbedComponent = false;

        explicit ComponentTreeViewItem (Component* c, std::function<void (Component* c)> outline, std::function<void (Component* c)> select)
            : outlineComponentCallback (outline), selectComponentCallback (select), component (c)
        {
            // TabbedComponents don't offer up their children easily...
            if (auto tabs = dynamic_cast<TabbedComponent*> (c))
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

            // Make our tree self aware
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
            return component ? (component->getNumChildComponents() > 0) : false;
        }

        // naive but functional...
        void openTreeAndSelect (Component* target)
        {
            // don't let us select something already selected
            if (component == target && ! isSelected())
            {
                setSelected (true, true);
                setOpen(true);
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

        void paintItem (Graphics& g, int w, int h) override
        {
            if (! component)
                return;
            if (isSelected())
            {
                g.setColour (color::blueLabelBackgroundColor);
                g.fillRect (0, 0, w - 30, h);
            }

            g.setColour (color::blueLabelTextColor);
            if (! component->isVisible())
                g.setColour (Colours::grey);

            g.setFont (Font ("Verdana", 14, Font::FontStyleFlags::plain));
            g.drawText (componentString (component), 5, 0, w - 5, h, Justification::left, true);
        }

        // must override to set the disclosure triangle color
        void paintOpenCloseButton (Graphics& g, const Rectangle<float>& area, Colour backgroundColour, bool isMouseOver) override
        {
            getOwnerView()->getLookAndFeel().drawTreeviewPlusMinusBox (g, area, color::blueLabelBackgroundColor, isOpen(), isMouseOver);
        }

        void itemClicked (const MouseEvent&) override
        {
            selectComponentCallback (component);
            openTabIfNeeded();
        }

        void mouseEnter (const MouseEvent& event) override
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

        // Callback from the component listener. Reconstruct children when component is deleted
        void componentChildrenChanged (Component& changedComponent) override
        {
            validateSubItems();
        }

        std::function<void (Component* c)> outlineComponentCallback;
        std::function<void (Component* c)> selectComponentCallback;

    private:
        Component::SafePointer<Component> component;
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ComponentTreeViewItem)

        void recursivelyAddChildrenFor (Component* child)
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
                auto subItemToValidate = dynamic_cast<ComponentTreeViewItem*> (getSubItem (i));
                if (subItemToValidate->component == nullptr)
                {
                    // scorched earth: if any child has a deleted component, we re-render the whole branch
                    // this is because we don't explicitly know if things were added or removed
                    clearSubItems();
                    addItemsForChildComponents();
                    break;
                }
            }
        }

        void openTabIfNeeded()
        {
            if (!getParentItem())
                return;
            
            auto parent = dynamic_cast<ComponentTreeViewItem*> (getParentItem());
            if (parent->hasTabbedComponent && ! component->isVisible())
            {
                dynamic_cast<TabbedComponent*> (parent->component.getComponent())->setCurrentTabIndex (getIndexInParent());
            }
        }
    };
}
