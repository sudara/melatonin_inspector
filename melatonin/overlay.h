#pragma once
#include "helpers.h"

namespace melatonin
{
    using namespace juce;

    class Overlay : public Component, public ComponentListener
    {
    public:
        Overlay()
        {
            setAlwaysOnTop (true);
            setName ("Melatonin Overlay");
            // need to click on the resizeable
            setInterceptsMouseClicks (false, true);
            addAndMakeVisible (dimensions);

            dimensions.setFont (13.f);
            dimensions.setJustificationType (Justification::centred);
            dimensions.setColour (1, color::blueLabelTextColor);
        }
        void paint (Graphics& g) override
        {
            g.setColour (color::blueLineColor);

            // draws inwards as the line thickens
            if (outlinedComponent)
                g.drawRect (outlinedBounds, 2.0f);
            if (selectedComponent)
            {
                // Thinner border than hover (draws inwards)
                g.drawRect (selectedBounds, 1.0f);

                const float dashes[] { 2.0f, 2.0f };
                g.drawDashedLine (lineFromTopToParent, dashes, 2, 1.0f);
                g.drawDashedLine (lineFromLeftToParent, dashes, 2, 1.0f);

                // corners outside
                g.fillRect (Rectangle<int> (selectedBounds.getTopLeft().translated (-4, -4), selectedBounds.getTopLeft().translated (4, 4)));
                g.fillRect (Rectangle<int> (selectedBounds.getTopRight().translated (-4, -4), selectedBounds.getTopRight().translated (4, 4)));
                g.fillRect (Rectangle<int> (selectedBounds.getBottomRight().translated (-4, -4), selectedBounds.getBottomRight().translated (4, 4)));
                g.fillRect (Rectangle<int> (selectedBounds.getBottomLeft().translated (-4, -4), selectedBounds.getBottomLeft().translated (4, 4)));

                // corners inside
                g.setColour (color::white);
                g.fillRect (Rectangle<int> (selectedBounds.getTopLeft().translated (-3, -3), selectedBounds.getTopLeft().translated (3, 3)));
                g.fillRect (Rectangle<int> (selectedBounds.getTopRight().translated (-3, -3), selectedBounds.getTopRight().translated (3, 3)));
                g.fillRect (Rectangle<int> (selectedBounds.getBottomRight().translated (-3, -3), selectedBounds.getBottomRight().translated (3, 3)));
                g.fillRect (Rectangle<int> (selectedBounds.getBottomLeft().translated (-3, -3), selectedBounds.getBottomLeft().translated (3, 3)));

                g.setColour (color::blueLabelBackgroundColor);
                // text doesn't vertically center very nicely without manual offset
                g.fillRoundedRectangle (dimensionsLabelBounds.toFloat().withBottom (dimensionsLabelBounds.getBottom()), 2.0f);
            }
        }

        void resized() override
        {
            if (outlinedComponent)
            {
                outlineComponent (outlinedComponent);
            }
        }

        // Components that belong to overlay are screened out by the caller (inspector)
        void outlineComponent (Component* component)
        {
            // get rid of another outline when re-entering a selected component
            if (selectedComponent == component)
            {
                outlinedComponent = nullptr;
            }

            outlinedComponent = component;
            outlinedBounds = getLocalAreaForOutline (component);
            repaint();
        }

        void selectComponent (Component* component)
        {
            if (!component)
                return;

            if (selectedComponent)
            {
                bool isSameComponent = selectedComponent == component;
                deselectComponent();
                if(isSameComponent)
                    return;
            }

            // listen for those sweet resize calls
            component->addComponentListener (this);
            component->addMouseListener(this, false);

            // take over the outline from the hover
            outlinedComponent = nullptr;
            selectedComponent = component;
            resizable = std::make_unique<ResizableBorderComponent> (component, &constrainer);
            resizable->setBorderThickness (BorderSize<int> (6));
            addAndMakeVisible (*resizable);
            setSelectedAndResizeableBounds (component);
            repaint();

            if(selectedComponent)
            {
                constrainer.setMinimumOnscreenAmounts (selectedComponent->getHeight(), selectedComponent->getWidth(), selectedComponent->getHeight(), selectedComponent->getWidth());
                //reset previous selection and update mouse cursor
                selectedComponent->setMouseCursor(MouseCursor::DraggingHandCursor);
            }
        }

        // When our selected component has been dragged or resized this is our callback
        // We *must* then manually manage the size of the ResizableBorderComponent
        void componentMovedOrResized (Component& component, bool wasMoved, bool wasResized) override
        {
            if (wasResized || wasMoved)
            {
                // sort of annoying if hover triggers on resize
                if (outlinedComponent)
                    outlinedComponent = nullptr;
                setSelectedAndResizeableBounds (&component);
            }
        }

        void mouseExit (const MouseEvent& /*event*/) override
        {
            outlinedComponent = nullptr;
            if (!selectedComponent)
                return;

            selectedComponent->setMouseCursor(MouseCursor::NormalCursor);
            repaint();
        }

        void mouseUp (const MouseEvent& event) override
        {
            Component::mouseUp (event);
            isDragging = false;
        }

        void mouseEnter (const MouseEvent& event) override
        {
            if (!selectedComponent)
                return;

            selectedComponent->setMouseCursor(MouseCursor::DraggingHandCursor);
            repaint();
        }

        void mouseMove (const MouseEvent& event) override
        {
            if (!selectedComponent)
                return;
            selectedComponent->setMouseCursor(MouseCursor::DraggingHandCursor);
            repaint();
        }

        void startDraggingComponent(const MouseEvent& e){
            //only allow dragging if the mouse is inside the selected component
            if(selectedComponent && selectedComponent->getLocalBounds().contains(e.getEventRelativeTo(selectedComponent).getPosition()))
            {
                componentDragger.startDraggingComponent(selectedComponent, e);
                isDragging = true;
            }
        }

        void dragSelectedComponent (const MouseEvent& e)
        {
            //only allow dragging if the mouse is inside the selected component
            bool isInside = selectedComponent && selectedComponent->getLocalBounds().contains(e.getEventRelativeTo(selectedComponent).getPosition());

            if(isInside || (selectedComponent && isDragging))
            {
                isDragging = true;
                componentDragger.dragComponent (selectedComponent, e, &constrainer);
            }
        }

    private:
        Component::SafePointer<Component> outlinedComponent;
        Rectangle<int> outlinedBounds;

        bool isDragging = false;
        ComponentDragger componentDragger;
        ComponentBoundsConstrainer boundsConstrainer;

        Component::SafePointer<Component> selectedComponent;
        Rectangle<int> selectedBounds;
        Line<float> lineFromTopToParent;
        Line<float> lineFromLeftToParent;

        std::unique_ptr<ResizableBorderComponent> resizable;
        ComponentBoundsConstrainer constrainer;

        Label dimensions;
        Rectangle<int> dimensionsLabelBounds;

        Rectangle<int> getLocalAreaForOutline (Component* component, int borderSize = 2)
        {
            auto boundsPlusOutline = component->getBounds().expanded (borderSize);
            return getLocalArea (component->getParentComponent(), boundsPlusOutline);
        }

        void drawDimensionsLabel()
        {
            int labelWidth = (int) dimensions.getFont().getStringWidthFloat (dimensionsString (selectedBounds)) + 15;
            int labelHeight = 15;
            auto paddingToLabel = 4;
            auto labelCenterX = selectedBounds.getX() + selectedBounds.getWidth() / 2;

            if ((selectedBounds.getBottom() + 20 + paddingToLabel) < getBottom())
            {
                // label on bottom
                dimensionsLabelBounds = Rectangle<int> ((int) (labelCenterX - labelWidth / 2), selectedBounds.getBottom() + paddingToLabel, labelWidth, labelHeight).expanded (2, 1);
            }
            else
            {
                // label on top
                dimensionsLabelBounds = Rectangle<int> ((int) (labelCenterX - labelWidth / 2), selectedBounds.getY() - labelHeight - paddingToLabel, labelWidth, labelHeight).expanded (2, 1);
            }
            dimensions.setText (dimensionsString (selectedBounds), dontSendNotification);
            dimensions.setBounds (dimensionsLabelBounds);
            dimensions.setVisible (true);
        }

        void calculateLinesToParent()
        {
            //Todo(Investigate why this is happening)
            if(selectedComponent == nullptr)
            {
                jassertfalse;
                return;
            }

            auto topOfComponent = selectedComponent->getBoundsInParent().getPosition().translated (selectedBounds.getWidth() / 2, -1);
            auto leftOfComponent = selectedComponent->getBoundsInParent().getPosition().translated (-1, selectedBounds.getHeight() / 2);

            auto localTop = getLocalPoint (selectedComponent->getParentComponent(), topOfComponent);
            auto localParentTop = getLocalPoint (selectedComponent->getParentComponent(), topOfComponent.withY (0));
            auto localLeft = getLocalPoint (selectedComponent->getParentComponent(), leftOfComponent);
            auto localParentLeft = getLocalPoint (selectedComponent->getParentComponent(), leftOfComponent.withX (0));

            lineFromTopToParent = Line<int> (localTop, localParentTop).toFloat();
            lineFromLeftToParent = Line<int> (localLeft, localParentLeft).toFloat();
        }

        void setSelectedAndResizeableBounds (Component* component)
        {
            selectedBounds = getLocalAreaForOutline (component, 1);
            drawDimensionsLabel();
            calculateLinesToParent();
            resizable->setBounds (selectedBounds);
            repaint();
        }

        void deselectComponent()
        {
            dimensions.setVisible (false);

            selectedComponent->removeComponentListener (this);
            selectedComponent->removeMouseListener(this);

            selectedComponent->setMouseCursor(MouseCursor::NormalCursor);

            selectedComponent = nullptr;
            repaint();
        }
    };

    // Unfortunately the DocumentWindow cannot behave as our root component mouse listener
    // without some strange side effects. That's why we are doing the lambda dance...
    class MouseInspector : public MouseListener
    {
    public:
        MouseInspector (Component& c) : root (c)
        {
            // Listen to all mouse movements for all children of the root
            root.addMouseListener (this, true);
        }

        ~MouseInspector() override
        {
            root.removeMouseListener (this);
        }

        void mouseEnter (const MouseEvent& event) override
        {
            outlineComponentCallback (event.originalComponent);
        }

        void mouseUp (const MouseEvent& event) override
        {
            if (event.mods.isLeftButtonDown() && !isDragging)
            {
                selectComponentCallback (event.originalComponent);
            }
            isDragging = false;
        }

        void mouseDown (const MouseEvent& event) override
        {
            if (event.mods.isLeftButtonDown() && event.originalComponent->isMouseOverOrDragging())
            {
                componentStartDraggingCallback (event.originalComponent, event);
            }
        }

        void mouseDrag (const MouseEvent& event) override
        {
            //takes care of small mouse position drift on selection
            if(event.getDistanceFromDragStart() > 3 && event.originalComponent->isMouseOverOrDragging())
            {
                isDragging = true;
                componentDraggedCallback (event.originalComponent, event);
            }
        }

        void mouseExit (const MouseEvent& event) override
        {
            if (event.originalComponent == &root)
            {
                mouseExitCallback();
            }
        }

        std::function<void (Component* c)> outlineComponentCallback;
        std::function<void (Component* c)> selectComponentCallback;
        std::function<void (Component* c, const MouseEvent& e)> componentStartDraggingCallback;
        std::function<void (Component* c, const MouseEvent& e)> componentDraggedCallback;
        std::function<void()> mouseExitCallback;

    private:
        Component& root;

        bool isDragging{false};
    };
}
