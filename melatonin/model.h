#pragma once

#include "juce_gui_basics/juce_gui_basics.h"
#include "helpers.h"


namespace melatonin{
    class ComponentModel : private juce::Value::Listener{
    public:
        ComponentModel(){

        }

        void selectComponent(juce::Component* component){
            selectedComponent = component;
            updateModel();
        }

        void deselectComponent(){
            selectedComponent = nullptr;

            resetModel();
        }

        juce::Value widthValue, heightValue, xValue, yValue;
        bool opaqueValue, hasCachedImageValue, accessibilityHandled, focused;
        juce::String lookAndFeel{""}, fontValue, alphaValue;

    private:
        juce::Component* selectedComponent;


        void updateModel()
        {
            widthValue.removeListener(this);
            heightValue.removeListener(this);

            xValue.removeListener(this);
            yValue.removeListener(this);

            if(selectedComponent){
                auto boundsInParent = selectedComponent->getBoundsInParent();

                widthValue.setValue(selectedComponent->getWidth());
                heightValue.setValue(selectedComponent->getHeight());

                xValue.setValue(boundsInParent.getX());
                yValue.setValue(boundsInParent.getY());

                opaqueValue = selectedComponent->isOpaque();
                hasCachedImageValue = selectedComponent->getCachedComponentImage() != nullptr;
                lookAndFeel = lnfString(selectedComponent);
                fontValue = componentFontValue(selectedComponent);
                alphaValue = juce::String(selectedComponent->getAlpha());

                accessibilityHandled = selectedComponent->isAccessible();
                focused = selectedComponent->isFocusContainer();

                widthValue.addListener(this);
                heightValue.addListener(this);

                xValue.addListener(this);
                yValue.addListener(this);
            }
        }

        void resetModel()
        {

            widthValue.removeListener(this);
            heightValue.removeListener(this);

            xValue.removeListener(this);
            yValue.removeListener(this);
        }

        void valueChanged (juce::Value& value) override
        {
            if(selectedComponent)
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
                    selectedComponent->setSize((int) widthValue.getValue(),
                        (int) heightValue.getValue());

                }
            }
            else
                jassertfalse;
        }
    };
}
