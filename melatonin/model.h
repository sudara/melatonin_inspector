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
            virtual void componentChanged (ComponentModel& model) = 0;
        };

        ComponentModel() = default;

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

            resetModel();
        }

        juce::Value widthValue, heightValue, xValue, yValue;
        bool opaqueValue{}, hasCachedImageValue{}, accessibilityHandled{}, focused{};
        juce::String lookAndFeel { "" }, fontValue, alphaValue;

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

        //this may return nullptr if no component is selected
        juce::Component* getSelectedComponent()
        {
            return selectedComponent;
        }

    private:
        juce::ListenerList<Listener> listenerList;
        juce::Component::SafePointer<juce::Component> selectedComponent;

        void updateModel()
        {
            widthValue.removeListener (this);
            heightValue.removeListener (this);

            xValue.removeListener (this);
            yValue.removeListener (this);

            if (selectedComponent)
            {
                auto boundsInParent = selectedComponent->getBoundsInParent();

                widthValue.setValue (selectedComponent->getWidth());
                heightValue.setValue (selectedComponent->getHeight());

                xValue.setValue (boundsInParent.getX());
                yValue.setValue (boundsInParent.getY());

                opaqueValue = selectedComponent->isOpaque();
                hasCachedImageValue = selectedComponent->getCachedComponentImage() != nullptr;
                lookAndFeel = lnfString (selectedComponent);
                fontValue = componentFontValue (selectedComponent);
                alphaValue = juce::String (selectedComponent->getAlpha());

                accessibilityHandled = selectedComponent->isAccessible();

                widthValue.addListener (this);
                heightValue.addListener (this);

                xValue.addListener (this);
                yValue.addListener (this);
            }

            listenerList.call ([this] (Listener& vml) {
                vml.componentChanged (*this);
            });
        }

        void resetModel()
        {
            widthValue.removeListener (this);
            heightValue.removeListener (this);

            xValue.removeListener (this);
            yValue.removeListener (this);

            listenerList.call ([this] (Listener& vml) {
                vml.componentChanged (*this);
            });
        }

        void valueChanged (juce::Value& value) override
        {
            if (selectedComponent)
            {
                if (value == widthValue || value == heightValue)
                {
                    selectedComponent->setSize ((int) widthValue.getValue(), (int) heightValue.getValue());
                }
                if (value == xValue || value == yValue)
                {
                    int leftVal = xValue.getValue();
                    int topVal = yValue.getValue();

                    selectedComponent->setTopLeftPosition (leftVal, topVal);
                    selectedComponent->setSize ((int) widthValue.getValue(),
                        (int) heightValue.getValue());
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
