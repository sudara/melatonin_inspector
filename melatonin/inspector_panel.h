#pragma once
#include "box_model.h"
#include "component_tree_view_item.h"

namespace melatonin
{
    class InspectorPanel : public Component
    {
    public:
        InspectorPanel (Component& rootComponent, Component& overlay) : root (rootComponent)
        {
            addAndMakeVisible (tree);
            addAndMakeVisible (boxModel);
            //tree.setLookAndFeel (&getLookAndFeel());
        }

        void paint (Graphics& g) override
        {
            //g.drawRect ()
        }

        void reconstructRoot()
        {
            jassert (selectComponentCallback);
            rootSet = true;
            tree.setRootItem (new ComponentTreeViewItem (&root, outlineComponentCallback, selectComponentCallback));
            getRoot()->setOpenness(ComponentTreeViewItem::Openness::opennessOpen);
        }

        void resized() override
        {
            boxModel.setBounds (0, 0, getWidth(), 250);
            tree.setBounds (30, 250, getWidth()-30, getHeight() - 250);
        }

        void displayComponentInfo (Component* component)
        {
            if (! rootSet)
                reconstructRoot();
            selectedComponent = component;
            boxModel.setComponent (component);
            repaint();
        }

        void selectComponent (Component* component)
        {
            resetTree();
            getRoot()->openTreeAndSelect (component);

            tree.scrollToKeepItemVisible (dynamic_cast<ComponentTreeViewItem*> (tree.getSelectedItem (0)));
        }

        std::function<void (Component* c)> selectComponentCallback;
        std::function<void (Component* c)> outlineComponentCallback;

    private:
        Component::SafePointer<Component> selectedComponent;
        Component& root;
        TreeView tree;
        BoxModel boxModel;
        bool rootSet = false;

        ComponentTreeViewItem* getRoot()
        {
            return dynamic_cast<ComponentTreeViewItem*> (tree.getRootItem());
        }

        void resetTree()
        {
            getRoot()->recursivelyCloseSubItems();
        }
    };
}
