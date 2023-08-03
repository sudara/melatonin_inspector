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

        juce::Value widthValue, heightValue, xValue, yValue;
        juce::Value enabledValue, opaqueValue, hasCachedImageValue, accessibilityHandledValue;
        juce::Value visibleValue, wantsFocusValue, interceptsMouseValue, childrenInterceptsMouseValue;
        juce::Value lookAndFeelValue, typeValue, fontValue, alphaValue;

        struct NamedProperty
        {
            NamedProperty() = default;
            NamedProperty (const juce::String& n, const juce::var& v)
                : name (n), value (v)
            {
            }

            juce::String    name;
            juce::Value     value;
        };

        std::vector<NamedProperty> namedProperties;

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

            if (selectedComponent)
            {
                auto boundsInParent = selectedComponent->getBoundsInParent();

                widthValue.setValue (selectedComponent->getWidth());
                heightValue.setValue (selectedComponent->getHeight());

                xValue.setValue (boundsInParent.getX());
                yValue.setValue (boundsInParent.getY());

                visibleValue = selectedComponent->isVisible ();
                wantsFocusValue = selectedComponent->getWantsKeyboardFocus ();
                enabledValue = selectedComponent->isEnabled();
                opaqueValue = selectedComponent->isOpaque();
                hasCachedImageValue = selectedComponent->getCachedComponentImage() != nullptr;
                typeValue = type (*selectedComponent);
                lookAndFeelValue = lnfString (selectedComponent);
                fontValue = componentFontValue (selectedComponent);
                alphaValue = juce::String (selectedComponent->getAlpha());

                accessibilityHandledValue = selectedComponent->isAccessible();

                widthValue.addListener (this);
                heightValue.addListener (this);

                xValue.addListener (this);
                yValue.addListener (this);

                visibleValue.addListener (this);
                wantsFocusValue.addListener (this);
                enabledValue.addListener (this);
                opaqueValue.addListener (this);
                alphaValue.addListener (this);
                accessibilityHandledValue.addListener (this);
                interceptsMouseValue.addListener (this);
                childrenInterceptsMouseValue.addListener (this);

                {
                    bool interceptsMouse = false;
                    bool childrenInterceptsMouse = false;
                    selectedComponent->getInterceptsMouseClicks (interceptsMouse, childrenInterceptsMouse);
                    interceptsMouseValue = interceptsMouse;
                    childrenInterceptsMouseValue = childrenInterceptsMouse;
                }

                {
                    auto& properties = selectedComponent->getProperties();
                    for (auto nv : properties)
                        namedProperties.push_back ({nv.name.toString(), nv.value});

                    for (auto& nv : namedProperties)
                        nv.value.addListener (this);
                }
            }

            listenerList.call ([this] (Listener& listener) {
                listener.componentModelChanged (*this);
            });
        }

        void removeListeners()
        {
            widthValue.removeListener (this);
            heightValue.removeListener (this);
            xValue.removeListener (this);
            yValue.removeListener (this);
            enabledValue.removeListener (this);
            opaqueValue.removeListener (this);
            alphaValue.removeListener (this);
            visibleValue.removeListener (this);
            wantsFocusValue.removeListener (this);
            accessibilityHandledValue.removeListener (this);
            interceptsMouseValue.removeListener (this);
            childrenInterceptsMouseValue.removeListener (this);

            for (auto& np : namedProperties)
                np.value.removeListener (this);
            namedProperties.clear();

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
                else if (value.refersToSameSourceAs (xValue) || value.refersToSameSourceAs (yValue))
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
                else if (value.refersToSameSourceAs (visibleValue))
                {
                    selectedComponent->setVisible (visibleValue.getValue());
                }
                else if (value.refersToSameSourceAs (wantsFocusValue))
                {
                    selectedComponent->setWantsKeyboardFocus (wantsFocusValue.getValue());
                }
                else if (value.refersToSameSourceAs (enabledValue))
                {
                    selectedComponent->setEnabled (enabledValue.getValue());
                }
                else if (value.refersToSameSourceAs (alphaValue))
                {
                    selectedComponent->setAlpha ((float) alphaValue.getValue());
                }
                else if (value.refersToSameSourceAs (opaqueValue))
                {
                    selectedComponent->setOpaque (opaqueValue.getValue());
                }
                else if (value.refersToSameSourceAs (accessibilityHandledValue))
                {
                    selectedComponent->setAccessible (accessibilityHandledValue.getValue());
                }
                else if (value.refersToSameSourceAs (interceptsMouseValue) || value.refersToSameSourceAs (childrenInterceptsMouseValue))
                {
                    selectedComponent->setInterceptsMouseClicks (interceptsMouseValue.getValue(), childrenInterceptsMouseValue.getValue());
                }
                else
                {
                    for (auto& nv : namedProperties)
                    {
                        if (value.refersToSameSourceAs (nv.value))
                        {
                            selectedComponent->getProperties().set (nv.name, nv.value.getValue());
                            selectedComponent->repaint();
                            break;
                        }
                    }
                }
            }
            else
            {
                jassertfalse;
            }
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
