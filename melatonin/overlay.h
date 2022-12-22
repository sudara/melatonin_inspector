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

            juce::Label* labels[4] = { &distanceToTopHoveredLabel, &distanceToLeftHoveredLabel, &distanceToBottomHoveredLabel, &distanceToRightHoveredLabel};

            for(auto* l: labels){
                l->setFont (13.f);
                l->setJustificationType (Justification::centred);
                l->setColour (juce::Label::textColourId, color::white);

                addAndMakeVisible(l);
            }

            dimensions.setFont (13.f);
            dimensions.setJustificationType (Justification::centred);
            dimensions.setColour (juce::Label::textColourId, color::white);
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

            if (!hoveredBounds.isEmpty())
            {
                g.setColour (color::redLineColor);
                g.drawRect (hoveredBounds.reduced (1.0f));
                g.drawRect (selectedBounds.reduced (1.0f));

                const float dashes[] { 2.0f, 2.0f };
                g.drawLine(lineToTopHoveredComponent, 2);
                g.drawLine(lineToLeftHoveredComponent, 2);
                g.drawLine(lineToRightHoveredComponent, 2);
                g.drawLine(lineToBottomHoveredComponent, 2);

                g.drawDashedLine (horConnectingLineToComponent, dashes, 2, 2.0f);
                g.drawDashedLine (vertConnectingLineToComponent, dashes, 2, 2.0f);

                // text doesn't vertically center very nicely without manual offset
                if(distanceToTopHoveredLabel.isVisible()) g.fillRoundedRectangle (distanceToTopLabelBounds.toFloat().withBottom (distanceToTopLabelBounds.getBottom()), 2.0f);
                if(distanceToBottomHoveredLabel.isVisible()) g.fillRoundedRectangle (distanceToBottomLabelBounds.toFloat().withBottom (distanceToBottomLabelBounds.getBottom()), 2.0f);
                if(distanceToLeftHoveredLabel.isVisible()) g.fillRoundedRectangle (distanceToLeftLabelBounds.toFloat().withBottom (distanceToLeftLabelBounds.getBottom()), 2.0f);
                if(distanceToRightHoveredLabel.isVisible()) g.fillRoundedRectangle (distanceToRightLabelBounds.toFloat().withBottom (distanceToRightLabelBounds.getBottom()), 2.0f);
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

        void resetDistanceLinesToHovered()
        {
            lineToTopHoveredComponent = Line<float>();
            lineToLeftHoveredComponent = Line<float>();
            lineToRightHoveredComponent = Line<float>();
            lineToBottomHoveredComponent = Line<float>();

            horConnectingLineToComponent = Line<float>();
            vertConnectingLineToComponent = Line<float>();

            distanceToTopHoveredLabel.setVisible(false);
            distanceToBottomHoveredLabel.setVisible(false);
            distanceToLeftHoveredLabel.setVisible(false);
            distanceToRightHoveredLabel.setVisible(false);
        }
        //draws a disatances line when component is selected shows distance to parent or hovered element
        void outlineDistanceCallback (Component* hovComponent)
        {
            hoveredComponent = hovComponent;
            if (hovComponent != selectedComponent && hovComponent != nullptr)
            {
                hoveredBounds = getLocalAreaForOutline (hovComponent);
                calculateDistanceLinesToHovered();
                drawDistanceLabel();
            }
            else
            {
                hoveredBounds = Rectangle<int>();

                resetDistanceLinesToHovered();
            }

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
                if (isSameComponent)
                    return;
            }

            // listen for those sweet resize calls
            component->addComponentListener (this);
            component->addMouseListener (this, false);

            // take over the outline from the hover
            outlinedComponent = nullptr;
            selectedComponent = component;
            resizable = std::make_unique<ResizableBorderComponent> (component, &constrainer);
            resizable->setBorderThickness (BorderSize<int> (6));
            addAndMakeVisible (*resizable);
            setSelectedAndResizeableBounds (component);
            repaint();

            if (selectedComponent)
            {
                constrainer.setMinimumOnscreenAmounts (selectedComponent->getHeight(), selectedComponent->getWidth(), selectedComponent->getHeight(), selectedComponent->getWidth());
                //reset previous selection and update mouse cursor
                selectedComponent->setMouseCursor (MouseCursor::DraggingHandCursor);
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

            selectedComponent->setMouseCursor (MouseCursor::NormalCursor);
            repaint();
        }

        void mouseUp (const MouseEvent& event) override
        {
            Component::mouseUp (event);
            isDragging = false;
        }

        void mouseEnter (const MouseEvent&) override
        {
            if (!selectedComponent)
                return;

            selectedComponent->setMouseCursor (MouseCursor::DraggingHandCursor);
            repaint();
        }

        void mouseMove (const MouseEvent&) override
        {
            if (!selectedComponent)
                return;
            selectedComponent->setMouseCursor (MouseCursor::DraggingHandCursor);
            repaint();
        }

        void startDraggingComponent (const MouseEvent& e)
        {
            //only allow dragging if the mouse is inside the selected component
            if (selectedComponent && selectedComponent->getLocalBounds().contains (e.getEventRelativeTo (selectedComponent).getPosition()))
            {
                componentDragger.startDraggingComponent (selectedComponent, e);
                isDragging = true;
            }
        }

        void dragSelectedComponent (const MouseEvent& e)
        {
            //only allow dragging if the mouse is inside the selected component
            bool isInside = selectedComponent && selectedComponent->getLocalBounds().contains (e.getEventRelativeTo (selectedComponent).getPosition());

            if (isInside || (selectedComponent && isDragging))
            {
                isDragging = true;
                componentDragger.dragComponent (selectedComponent, e, &constrainer);
            }
        }

    private:
        Component::SafePointer<Component> outlinedComponent;
        Component::SafePointer<Component> hoveredComponent;
        Rectangle<int> outlinedBounds;

        bool isDragging = false;
        ComponentDragger componentDragger;
        ComponentBoundsConstrainer boundsConstrainer;

        Component::SafePointer<Component> selectedComponent;
        Rectangle<int> selectedBounds;
        Line<float> lineFromTopToParent;
        Line<float> lineFromLeftToParent;

        Rectangle<int> hoveredBounds;
        juce::Line<float> lineToTopHoveredComponent,
            lineToLeftHoveredComponent,
            lineToRightHoveredComponent,
            lineToBottomHoveredComponent,
            horConnectingLineToComponent,
            vertConnectingLineToComponent;

        Label distanceToTopHoveredLabel,
            distanceToBottomHoveredLabel,
            distanceToLeftHoveredLabel,
            distanceToRightHoveredLabel;
        Rectangle<int> distanceToTopLabelBounds,
            distanceToLeftLabelBounds,
            distanceToRightLabelBounds,
            distanceToBottomLabelBounds;

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
            if (selectedComponent == nullptr)
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

        void drawDistanceLabel()
        {
            //todo avoid overlapping of labels with dimensions label
            if (selectedComponent && hoveredComponent)
            {
                int labelHeight = 15;
                auto paddingToLabel = 4;

                //top
                if(lineToTopHoveredComponent.getLength() > 0)
                {
                    int labelWidth = (int) distanceToTopHoveredLabel.getFont().getStringWidthFloat (distanceString (lineToTopHoveredComponent)) + 15;

                    //todo draw on left or right side of line
                    auto labelCenterY = lineToTopHoveredComponent.getPointAlongLineProportionally (0.5f).getY();
                    distanceToTopLabelBounds = Rectangle<int> ((int) (lineToTopHoveredComponent.getStartX() + paddingToLabel),
                            (int) labelCenterY - labelHeight / 2,
                            labelWidth,
                            labelHeight).expanded (2, 1);
                    distanceToTopHoveredLabel.setText (distanceString (lineToTopHoveredComponent), dontSendNotification);
                    distanceToTopHoveredLabel.setBounds (distanceToTopLabelBounds);
                }


                //bottom
                if(lineToBottomHoveredComponent.getLength() > 0){
                    int labelWidth = (int) distanceToBottomHoveredLabel.getFont().getStringWidthFloat (distanceString (lineToBottomHoveredComponent)) + 15;

                    //todo draw on left or right side of line
                    auto labelCenterY = lineToBottomHoveredComponent.getPointAlongLineProportionally (0.5f).getY();
                    distanceToBottomLabelBounds = Rectangle<int> ((int) (lineToBottomHoveredComponent.getStartX() + paddingToLabel),
                        (int) labelCenterY - labelHeight / 2,
                        labelWidth,
                        labelHeight).expanded (2, 1);
                    distanceToBottomHoveredLabel.setText (distanceString (lineToBottomHoveredComponent), dontSendNotification);
                    distanceToBottomHoveredLabel.setBounds (distanceToBottomLabelBounds);
                }

                //right
                if(lineToRightHoveredComponent.getLength() > 0){
                    int labelWidth = (int) distanceToRightHoveredLabel.getFont().getStringWidthFloat (distanceString (lineToRightHoveredComponent)) + 15;

                    //todo draw on top or bottom side of line
                    auto labelCenterX = lineToRightHoveredComponent.getPointAlongLineProportionally (0.5f).getX();
                    distanceToRightLabelBounds = Rectangle<int> ((int) labelCenterX - labelWidth / 2,
                        (int) (lineToRightHoveredComponent.getStartY() + paddingToLabel),
                        labelWidth,
                        labelHeight).expanded (2, 1);
                    distanceToRightHoveredLabel.setText (distanceString (lineToRightHoveredComponent), dontSendNotification);
                    distanceToRightHoveredLabel.setBounds (distanceToRightLabelBounds);
                }

                //left
                if(lineToLeftHoveredComponent.getLength() > 0){
                    int labelWidth = (int) distanceToLeftHoveredLabel.getFont().getStringWidthFloat (distanceString (lineToLeftHoveredComponent)) + 15;

                    //todo draw on top or bottom side of line
                    auto labelCenterX = lineToLeftHoveredComponent.getPointAlongLineProportionally (0.5f).getX();
                    distanceToLeftLabelBounds = Rectangle<int> ((int) labelCenterX - labelWidth / 2,
                        (int) (lineToLeftHoveredComponent.getStartY() + paddingToLabel),
                        labelWidth,
                        labelHeight).expanded (2, 1);
                    distanceToLeftHoveredLabel.setText (distanceString (lineToLeftHoveredComponent), dontSendNotification);
                    distanceToLeftHoveredLabel.setBounds (distanceToLeftLabelBounds);
                }

            }

            distanceToTopHoveredLabel.setVisible (lineToTopHoveredComponent.getLength() > 0);
            distanceToBottomHoveredLabel.setVisible (lineToBottomHoveredComponent.getLength() > 0);
            distanceToLeftHoveredLabel.setVisible (lineToLeftHoveredComponent.getLength() > 0);
            distanceToRightHoveredLabel.setVisible (lineToRightHoveredComponent.getLength() > 0);
        }
        void calculateDistanceLinesToHovered()
        {
            lineToTopHoveredComponent = Line<float>();
            lineToLeftHoveredComponent = Line<float>();
            lineToRightHoveredComponent = Line<float>();
            lineToBottomHoveredComponent = Line<float>();

            horConnectingLineToComponent = Line<float>();
            vertConnectingLineToComponent = Line<float>();

            if (hoveredComponent && selectedComponent)
            {
                bool hovOnLeft = selectedBounds.getCentreX() > hoveredBounds.getCentreX();
                bool hovOnTop = selectedBounds.getCentreY() > hoveredBounds.getCentreY();

                //if the hovered component is above the selected component
                if (hovOnTop)
                {
                    //if the hovered component is left of the selected component
                    if (hovOnLeft)
                    {
                        auto p1 = selectedBounds.getTopLeft().translated (0, selectedBounds.getHeight() / 2);
                        lineToRightHoveredComponent = Line<int> (p1, p1.withX (hoveredBounds.getRight())).toFloat();
                    }
                    //if the hovered component is right of the selected component
                    else
                    {
                        auto p1 = selectedBounds.getTopRight().translated (0, selectedBounds.getHeight() / 2);
                        lineToLeftHoveredComponent = Line<int> (p1, p1.withX(hoveredBounds.getX())).toFloat();
                    }

                    auto p1 = selectedBounds.getTopLeft().translated (selectedBounds.getWidth() / 2, 0);

                    //avoid drawing of top line if the hovered component is left or right of the selected component
                    //if(selectedBounds.getY() > hoveredBounds.getBottom() || selectedBounds.getBottom() < hoveredBounds.getY())
                    lineToBottomHoveredComponent = Line<int> (p1, p1.withY (hoveredBounds.getBottom())).toFloat();

                    //avoid drawing horizontal line and if line is going into component
                    if (lineToBottomHoveredComponent.isHorizontal() || lineToBottomHoveredComponent.getStartY() < lineToBottomHoveredComponent.getEndY())
                        lineToBottomHoveredComponent = Line<float>();
                    //avoid drawing stricly vertical line lineToLeftHoveredComponent
                    if (lineToLeftHoveredComponent.isVertical() || lineToLeftHoveredComponent.getStartX() > lineToLeftHoveredComponent.getEndX())
                        lineToLeftHoveredComponent = Line<float>();
                    //avoid drawing stricly vertical line lineToRightHoveredComponent
                    if (lineToRightHoveredComponent.isVertical() || lineToRightHoveredComponent.getStartX() < lineToRightHoveredComponent.getEndX())
                        lineToRightHoveredComponent = Line<float>();
                }
                else
                {
                    //if the hovered component is left of the selected component
                    if (hovOnLeft)
                    {
                        auto p1 = selectedBounds.getBottomLeft().translated (0, -selectedBounds.getHeight() / 2);
                        lineToRightHoveredComponent = Line<int> (p1, p1.withX (hoveredBounds.getRight())).toFloat();
                    }
                    else
                    {
                        auto p1 = selectedBounds.getBottomRight().translated (0, -selectedBounds.getHeight() / 2);
                        lineToLeftHoveredComponent = Line<int> (p1, p1.withX (hoveredBounds.getX())).toFloat();
                    }

                    auto p1 = selectedBounds.getBottomLeft().translated (selectedBounds.getWidth() / 2, 0);
                    lineToTopHoveredComponent = Line<int> (p1, p1.withY (hoveredBounds.getY())).toFloat();

                    //avoid drawing horizontal line and if line is going into component
                    if (lineToTopHoveredComponent.isHorizontal() || lineToTopHoveredComponent.getStartY() > lineToTopHoveredComponent.getEndY())
                        lineToTopHoveredComponent = Line<float>();

                    //avoid drawing stricly vertical line lineToLeftHoveredComponent
                    if (lineToLeftHoveredComponent.isVertical() || lineToLeftHoveredComponent.getStartX() > lineToLeftHoveredComponent.getEndX())
                        lineToLeftHoveredComponent = Line<float>();
                    //avoid drawing stricly vertical line lineToRightHoveredComponent
                    if (lineToRightHoveredComponent.isVertical() || lineToRightHoveredComponent.getStartX() < lineToRightHoveredComponent.getEndX())
                        lineToRightHoveredComponent = Line<float>();
                }

                //adding missing lines to connect to hovered component
                if(!hoveredBounds.contains(lineToRightHoveredComponent.getEnd().toInt().translated(-2, 0)) && lineToRightHoveredComponent.getLength() > 0){
                    juce::Point<int> hoveredPoint;
                    if(hovOnTop){
                        hoveredPoint = hovOnLeft ?  hoveredBounds.getBottomRight() : hoveredBounds.getBottomLeft();
                    }
                    else
                        hoveredPoint = hovOnLeft ?  hoveredBounds.getTopRight() : hoveredBounds.getTopLeft();
                    vertConnectingLineToComponent = Line<float>(lineToRightHoveredComponent.getEnd().toFloat(), hoveredPoint.toFloat());
                }
                if(!hoveredBounds.contains(lineToLeftHoveredComponent.getEnd().translated(2, 0).toInt()) && lineToLeftHoveredComponent.getLength() > 0){
                    juce::Point<int> hoveredPoint;
                    if(hovOnTop){
                        hoveredPoint = hovOnLeft ?  hoveredBounds.getBottomRight() : hoveredBounds.getBottomLeft();
                    }
                    else
                        hoveredPoint = hovOnLeft ?  hoveredBounds.getTopRight() : hoveredBounds.getTopLeft();
                    vertConnectingLineToComponent = Line<float>(lineToLeftHoveredComponent.getEnd().toFloat(), hoveredPoint.toFloat());
                }

                if(!hoveredBounds.contains(lineToTopHoveredComponent.getEnd().toInt().translated(0, 2)) && lineToTopHoveredComponent.getLength() > 0){
                    juce::Point<int> hoveredPoint = hovOnLeft ?  hoveredBounds.getTopRight() : hoveredBounds.getTopLeft();
                    horConnectingLineToComponent = Line<float>(lineToTopHoveredComponent.getEnd().toFloat(), hoveredPoint.toFloat());
                }
                if(!hoveredBounds.contains(lineToBottomHoveredComponent.getEnd().toInt().translated(0, -2)) && lineToBottomHoveredComponent.getLength() > 0){
                    auto hoveredPoint = hovOnLeft ?  hoveredBounds.getBottomRight() : hoveredBounds.getBottomLeft();
                    horConnectingLineToComponent = Line<float>(lineToBottomHoveredComponent.getEnd().toFloat(), hoveredPoint.toFloat());
                }
            }
        }

        void deselectComponent()
        {
            dimensions.setVisible (false);

            selectedComponent->removeComponentListener (this);
            selectedComponent->removeMouseListener (this);

            selectedComponent->setMouseCursor (MouseCursor::NormalCursor);

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

        void mouseMove (const MouseEvent& event) override
        {
            if (outlineDistanceCallback && event.mods.isAltDown())
                outlineDistanceCallback (event.originalComponent);
            else
                outlineDistanceCallback (nullptr);
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
            if (event.getDistanceFromDragStart() > 3 && event.originalComponent->isMouseOverOrDragging())
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
        std::function<void (Component* c)> outlineDistanceCallback;
        std::function<void (Component* c)> selectComponentCallback;
        std::function<void (Component* c, const MouseEvent& e)> componentStartDraggingCallback;
        std::function<void (Component* c, const MouseEvent& e)> componentDraggedCallback;
        std::function<void()> mouseExitCallback;

    private:
        Component& root;

        bool isDragging { false };
    };
}
