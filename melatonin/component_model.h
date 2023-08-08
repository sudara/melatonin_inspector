#pragma once

#include "juce_gui_basics/juce_gui_basics.h"

namespace melatonin
{
    class ComponentModel : private juce::Value::Listener, private juce::ComponentListener
    {
    public:
        class Listener
        {
        public:
            virtual ~Listener() = default;
            virtual void componentModelChanged (ComponentModel& model) = 0;
        };

        juce::Value widthValue, heightValue, xValue, yValue;
        juce::Value enabledValue, opaqueValue, hasCachedImageValue, accessibilityHandledValue, focusedValue, interceptsMouseValue, childrenInterceptsMouseValue;
        juce::Value lookAndFeelValue, typeValue, fontValue, alphaValue;
        juce::Value timing1, timing2, timing3, hasChildren;
        double timingWithChildren1, timingWithChildren2, timingWithChildren3;
        std::vector<juce::Value> customKeys;
        std::vector<juce::Value> customValues;

        ComponentModel() = default;

        ~ComponentModel() override
        {
            selectComponent (nullptr);
        }

        void selectComponent (juce::Component* component)
        {
            // add component listener to component and unsubscribe from previous component
            if (selectedComponent)
                selectedComponent->removeComponentListener (this);

            selectedComponent = component;

            if (selectedComponent)
                selectedComponent->addComponentListener (this);

            updateModel();
        }

        void deselectComponent()
        {
            if (selectedComponent)
                selectedComponent->removeComponentListener (this);

            selectedComponent = nullptr;

            removeListeners();
        }

        void displayComponent (juce::Component*)
        {
            updateModel();
        }

        void removeListener (Listener& listener)
        {
            listenerList.remove (&listener);
        }

        void addListener (Listener& listener)
        {
            listenerList.add (&listener);
        }

        // this may return nullptr if no component is selected
        juce::Component* getSelectedComponent()
        {
            return selectedComponent;
        }

    private:
        juce::ListenerList<Listener> listenerList;
        juce::Component::SafePointer<juce::Component> selectedComponent;

        void updateModel()
        {
            removeListeners();

            if (!selectedComponent)
                return;

            auto boundsInParent = selectedComponent->getBoundsInParent();

            widthValue.setValue (selectedComponent->getWidth());
            heightValue.setValue (selectedComponent->getHeight());

            xValue.setValue (boundsInParent.getX());
            yValue.setValue (boundsInParent.getY());

            enabledValue = selectedComponent->isEnabled();
            opaqueValue = selectedComponent->isOpaque();
            hasCachedImageValue = selectedComponent->getCachedComponentImage() != nullptr;
            typeValue = type (*selectedComponent);
            lookAndFeelValue = lnfString (selectedComponent);
            fontValue = componentFontValue (selectedComponent);
            alphaValue = juce::String (selectedComponent->getAlpha());

            focusedValue = selectedComponent->hasKeyboardFocus (true);
            accessibilityHandledValue = selectedComponent->isAccessible();

            widthValue.addListener (this);
            heightValue.addListener (this);

            xValue.addListener (this);
            yValue.addListener (this);

            {
                bool interceptsMouse = false;
                bool childrenInterceptsMouse = false;
                selectedComponent->getInterceptsMouseClicks (interceptsMouse, childrenInterceptsMouse);
                interceptsMouseValue = interceptsMouse;
                childrenInterceptsMouseValue = childrenInterceptsMouse;
            }

            hasChildren.setValue (selectedComponent->getNumChildComponents() > 0);
            populatePerformanceData (selectedComponent->getProperties());

            listenerList.call ([this] (Listener& listener) {
                listener.componentModelChanged (*this);
            });
        }

        void populatePerformanceData (const juce::NamedValueSet& props)
        {
            if (props.contains ("timing1"))
            {
                // assume they are all there
                timing1 = props["timing1"];
                timing2 = props["timing2"];
                timing3 = props["timing3"];

                timingWithChildren1 = timing1.getValue();
                timingWithChildren2 = timing2.getValue();
                timingWithChildren3 = timing3.getValue();
                getTimingWithChildren (selectedComponent);
            }
        }

        double getTimingWithChildren (juce::Component* component)
        {
            for (auto child : component->getChildren())
            {
                if (child->getProperties().contains ("timing1"))
                {
                    timingWithChildren1 += (double) child->getProperties()["timing1"];
                    timingWithChildren2 += (double) child->getProperties()["timing2"];
                    timingWithChildren3 += (double) child->getProperties()["timing3"];
                    getTimingWithChildren (child);
                }
            }
        }

        void removeListeners()
        {
            widthValue.removeListener (this);
            heightValue.removeListener (this);
            xValue.removeListener (this);
            yValue.removeListener (this);

            listenerList.call ([this] (Listener& listener) {
                listener.componentModelChanged (*this);
            });
        }

        // allows properties to be set from our properties
        void valueChanged (juce::Value& value) override
        {
            if (selectedComponent)
            {
                if (value.refersToSameSourceAs (widthValue) || value.refersToSameSourceAs (heightValue))
                {
                    selectedComponent->setSize ((int) widthValue.getValue(), (int) heightValue.getValue());
                }
                if (value.refersToSameSourceAs (xValue) || value.refersToSameSourceAs (yValue))
                {
                    int leftVal = xValue.getValue();
                    int topVal = yValue.getValue();

                    // in cases where components are animated or moved via AffineTransforms
                    // we can get a feedback loop, as the left/top values are no longer
                    // the actual position in the component
                    // so first remove any transform present
                    selectedComponent->setTransform (juce::AffineTransform());
                    selectedComponent->setTopLeftPosition (leftVal, topVal);
                }
            }
            else
                jassertfalse;
        }

        void componentMovedOrResized (juce::Component&, bool wasMoved, bool wasResized) override
        {
            if (wasResized || wasMoved)
            {
                updateModel();
            }
        }
    };
}
