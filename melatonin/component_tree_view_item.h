#pragma once
#include "helpers.h"

namespace melatonin
{
    class Overlay;

    class ComponentTreeViewItem
        : public TreeViewItem,
          public MouseListener
    {
    public:
        explicit ComponentTreeViewItem (Component* c, std::function<void (Component* c)> outline, std::function<void (Component* c)> select)
            : outlineComponentCallback (outline), selectComponentCallback (select), component (c)
        {
            // TabbedComponents don't offer up their children easily...
            if (auto tabs = dynamic_cast<TabbedComponent*> (c))
            {
                for (int i = 0; i < tabs->getNumTabs(); ++i)
                {
                    auto child = tabs->getTabContentComponent (i);
                    addSubItem (new ComponentTreeViewItem (child, outlineComponentCallback, selectComponentCallback));
                }
            }

            if (componentString (c) != "melatonin::Overlay")
            {
                for (int i = 0; i < component->getNumChildComponents(); ++i)
                {
                    auto child = component->getChildComponent (i);
                    addSubItem (new ComponentTreeViewItem (child, outlineComponentCallback, selectComponentCallback));
                }
            }
        }

        bool mightContainSubItems() override
        {
            return component->getNumChildComponents() > 0;
        }

        // naive but functional...
        void openTreeAndSelect (Component* target)
        {
            // don't let us select something already selected
            if (component == target && ! isSelected())
            {
                setSelected (true, true);
            }
            else if (component->isParentOf (target))
            {
                jassert (target);
                setOpen (true);
                // recurse to open up tree
                for (int i = 0; i < getNumSubItems(); ++i)
                {
                    dynamic_cast<ComponentTreeViewItem*> (getSubItem (i))->openTreeAndSelect (target);
                }
            }
        }

        void paintItem (Graphics& g, int w, int h) override
        {
            jassert (component);
            if (isSelected())
            {
                g.setColour (color::blueLabelBackgroundColor);
                g.fillRect (0, 0, w - 30, h);
            }

            g.setColour (color::blueLabelTextColor);
            if (! component->isVisible())
                g.setColour (Colours::grey);

            g.setFont (16.0f);
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
        }

        void mouseEnter (const MouseEvent& event) override
        {
            outlineComponentCallback (component);
        }

        void recursivelyCloseSubItems()
        {
            for (int i = 0; i < getNumSubItems(); ++i)
            {
                getSubItem (i)->setOpen (true);
                dynamic_cast<ComponentTreeViewItem*> (getSubItem (i))->recursivelyCloseSubItems();
            }
        }

        std::function<void (Component* c)> outlineComponentCallback;
        std::function<void (Component* c)> selectComponentCallback;

    private:
        Component::SafePointer<Component> component;
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ComponentTreeViewItem)
    };
}