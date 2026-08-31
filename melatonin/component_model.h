#pragma once

#include <utility>
#include "helpers/component_helpers.h"
#include "helpers/paint_diagnostics.h"
#include "juce_gui_basics/juce_gui_basics.h"

namespace melatonin
{
    class ComponentModel : private juce::Value::Listener,
                           private juce::ComponentListener
#if MELATONIN_HAS_PAINT_DIAGNOSTICS
                         , private juce::AsyncUpdater
#endif
    {
    public:
        class Listener
        {
        public:
            virtual ~Listener() = default;
            virtual void componentModelChanged (ComponentModel& model) = 0;

            // Lightweight refresh fired when the paint history ticks over, so
            // Preview can redraw timings without rebuilding the whole model.
            // Default no-op.
            virtual void componentModelPaintHistoryUpdated (ComponentModel&) {}
        };

        juce::Value nameValue;
        juce::Value widthValue, heightValue, xValue, yValue;
        juce::Value enabledValue, opaqueValue, hasCachedImageValue, accessibilityHandledValue;
        juce::Value visibleValue, wantsFocusValue, interceptsMouseValue, childrenInterceptsMouseValue;
        juce::Value lookAndFeelValue, typeValue, fontValue, alphaValue;
        juce::Value pickedColor;

        juce::Value isToggleable, toggleState, clickTogglesState, radioGroupId;
        juce::Value sliderStyleValue, sliderTextBoxPositionValue, buttonStyleValue, comboBoxTextEditableValue;

        struct AccessiblityDetail
        {
            juce::Value title, value, role, handlerType;
        } accessiblityDetail;

        ComponentModel() = default;

        ~ComponentModel() override
        {
            selectComponent (nullptr);
        }

        void selectComponent (juce::Component* component)
        {
            TRACE_COMPONENT();

            // add component listener to component and unsubscribe from previous component
            if (selectedComponent)
                selectedComponent->removeComponentListener (this);

            selectedComponent = component;

#if MELATONIN_HAS_PAINT_DIAGNOSTICS
            // paint history is per-component — drop the previous component's
            paintHistory.clear();
            cancelPendingUpdate();
#endif

            if (selectedComponent)
                selectedComponent->addComponentListener (this);

            updateModel();
        }

        void deselectComponent()
        {
            TRACE_COMPONENT();

            if (selectedComponent)
                selectedComponent->removeComponentListener (this);

            selectedComponent = nullptr;
#if MELATONIN_HAS_PAINT_DIAGNOSTICS
            paintHistory.clear();
            cancelPendingUpdate();
#endif
            updateModel();
        }

        struct NamedProperty
        {
            NamedProperty() = default;
            NamedProperty (juce::String n, const juce::var& v)
                : name (std::move (n)), value (v)
            {
            }

            juce::String name;
            juce::Value value;
        };

        enum class StyleType
        {
            SliderStyle,
            SliderTextPosition,
            ButtonStyle,
            ComboBoxEditable
        };

        struct StyleProperty
        {
            StyleProperty() = default;
            StyleProperty(StyleType t, const juce::var& v, juce::StringArray opts = {})
                : type(t), value(v), options(std::move(opts))
            {
            }

            StyleType type;
            juce::Value value;
            juce::StringArray options;  // For choice/enum properties

            // Helper to get display name
            [[nodiscard]] juce::String getDisplayName() const
            {
                switch (type)
                {
                    case StyleType::SliderStyle: return "Slider Style";
                    case StyleType::SliderTextPosition: return "Text Position";
                    case StyleType::ButtonStyle: return "Button Style";
                    case StyleType::ComboBoxEditable: return "Text Editable";
                    default: return "Unknown";
                }
            }

            // Helper to get options for the style type
            static juce::StringArray getOptionsForType(StyleType t)
            {
                switch (t)
                {
                    case StyleType::SliderStyle:
                        return {"LinearHorizontal", "LinearVertical", "LinearBar",
                            "Rotary", "RotaryHorizontalDrag", "RotaryVerticalDrag",
                            "IncDecButtons", "TwoValueHorizontal", "TwoValueVertical"};

                    case StyleType::SliderTextPosition:
                        return {"NoTextBox", "TextBoxLeft", "TextBoxRight",
                            "TextBoxAbove", "TextBoxBelow"};

                    case StyleType::ButtonStyle:
                        return {"TextButton", "DrawableButton", "ImageButton",
                            "ArrowButton", "HyperlinkButton"};

                    case StyleType::ComboBoxEditable:
                        return {"NotEditable", "Editable"};

                    default:
                        return {};
                }
            }
        };

        std::vector<NamedProperty> namedProperties;
        std::vector<NamedProperty> colors;
        std::vector<StyleProperty> styles;

        void refresh()
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

#if MELATONIN_HAS_PAINT_DIAGNOSTICS
        [[nodiscard]] bool hasPaintHistory() const noexcept
        {
            return !paintHistory.empty();
        }

        [[nodiscard]] const PaintDiagnosticsHistory& getPaintHistory() const noexcept
        {
            return paintHistory;
        }

        // Reset the rolling history. Used by Preview's double-click handler.
        void clearPaintHistory()
        {
            paintHistory.clear();
            cancelPendingUpdate();
            notifyPaintHistoryUpdated();
        }
#endif

    private:
        juce::ListenerList<Listener> listenerList;
        juce::Component::SafePointer<juce::Component> selectedComponent;
#if MELATONIN_HAS_PAINT_DIAGNOSTICS
        PaintDiagnosticsHistory paintHistory;
#endif

        void updateModel()
        {
            TRACE_COMPONENT();

            removeListeners();

            // always show picked color, even with no component selected
            if (!pickedColor.getValue().isVoid())
                colors.emplace_back ("Last Picked", pickedColor);

            if (!selectedComponent)
            {
                notifyListeners();
                return;
            }

            nameValue = selectedComponent->getName();
            lookAndFeelValue = lnfString (selectedComponent);
            visibleValue = selectedComponent->isVisible();
            enabledValue = selectedComponent->isEnabled();
            alphaValue = juce::String (selectedComponent->getAlpha());
            opaqueValue = selectedComponent->isOpaque();
            wantsFocusValue = selectedComponent->getWantsKeyboardFocus();
            fontValue = componentFontValue (selectedComponent);
            hasCachedImageValue = selectedComponent->getCachedComponentImage() != nullptr;
            typeValue = type (*selectedComponent);
            accessibilityHandledValue = selectedComponent->isAccessible();

            updateStyleProperties();

            if (auto button = dynamic_cast<juce::Button*> (selectedComponent.getComponent()))
            {
                isToggleable = button->isToggleable();
                toggleState = button->getToggleState();
                clickTogglesState = button->getClickingTogglesState();
                radioGroupId = button->getRadioGroupId();
            }

            nameValue.addListener(this);
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

            isToggleable.addListener (this);
            toggleState.addListener (this);
            clickTogglesState.addListener (this);
            radioGroupId.addListener (this);

            sliderStyleValue.addListener (this);
            sliderTextBoxPositionValue.addListener (this);
            buttonStyleValue.addListener (this);
            comboBoxTextEditableValue.addListener (this);

            if (selectedComponent->isAccessible() && selectedComponent->getAccessibilityHandler())
            {
                auto* accH = selectedComponent->getAccessibilityHandler();
                accessiblityDetail.handlerType = type (*accH);
                if (accH->getValueInterface())
                {
                    accessiblityDetail.value = accH->getValueInterface()->getCurrentValueAsString();
                }
                else
                {
                    accessiblityDetail.value = "no value interface";
                }
                accessiblityDetail.title = accH->getTitle();
                auto role = accH->getRole();
                switch (role)
                {
                    // Amazingly juce doesn' thave a display name fn for these
#define DN(x)                          \
    case juce::AccessibilityRole::x:   \
        accessiblityDetail.role = #x; \
        break;
                    DN (button)
                    DN (toggleButton)
                    DN (radioButton)
                    DN (comboBox)
                    DN (image)
                    DN (slider)
                    DN (label)
                    DN (staticText)
                    DN (editableText)
                    DN (menuItem)
                    DN (menuBar)
                    DN (popupMenu)
                    DN (table)
                    DN (tableHeader)
                    DN (column)
                    DN (row)
                    DN (cell)
                    DN (hyperlink)
                    DN (list)
                    DN (listItem)
                    DN (tree)
                    DN (treeItem)
                    DN (progressBar)
                    DN (group)
                    DN (dialogWindow)
                    DN (window)
                    DN (scrollBar)
                    DN (tooltip)
                    DN (splashScreen)
                    DN (ignored)
                    DN (unspecified)
#undef DN
                    default:
                        accessiblityDetail.role = juce::String ("Unknown ") + juce::String ((int) role);
                        break;
                }
            }

            {
                bool interceptsMouse = false;
                bool childrenInterceptsMouse = false;
                selectedComponent->getInterceptsMouseClicks (interceptsMouse, childrenInterceptsMouse);
                interceptsMouseValue = interceptsMouse;
                childrenInterceptsMouseValue = childrenInterceptsMouse;
            }

            {
                auto& properties = selectedComponent->getProperties();
                for (const auto& nv : properties)
                {
                    if (nv.name.toString().startsWith ("jcclr_"))
                        colors.emplace_back (nv.name.toString(), nv.value);
                    else
                        namedProperties.emplace_back (nv.name.toString(), nv.value);
                }

                for (auto& nv : namedProperties)
                    nv.value.addListener (this);

                for (auto& nv : colors)
                    nv.value.addListener (this);

                for (auto& style : styles)
                    style.value.addListener (this);
            }
            notifyListeners();
        }

        void updateStyleProperties()
        {
            if (auto* slider = dynamic_cast<juce::Slider*>(selectedComponent.getComponent()))
            {
                styles.emplace_back(
                    StyleType::SliderStyle,
                    static_cast<int>(slider->getSliderStyle()),
                    StyleProperty::getOptionsForType(StyleType::SliderStyle));

                styles.emplace_back(
                    StyleType::SliderTextPosition,
                    static_cast<int>(slider->getTextBoxPosition()),
                    StyleProperty::getOptionsForType(StyleType::SliderTextPosition));
            }
            else if (auto* button = dynamic_cast<juce::Button*>(selectedComponent.getComponent()))
            {
                styles.emplace_back(
                    StyleType::ButtonStyle,
                    button->getButtonText().isEmpty() ? 1 : 0,
                    StyleProperty::getOptionsForType(StyleType::ButtonStyle));
            }
            else if (auto* comboBox = dynamic_cast<juce::ComboBox*>(selectedComponent.getComponent()))
            {
                styles.emplace_back(
                    StyleType::ComboBoxEditable,
                    comboBox->isTextEditable());
            }
        }

        void removeListeners()
        {
            TRACE_COMPONENT();

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

            isToggleable.removeListener (this);
            toggleState.removeListener (this);
            clickTogglesState.removeListener (this);
            radioGroupId.removeListener (this);

            sliderStyleValue.removeListener (this);
            sliderTextBoxPositionValue.removeListener (this);
            buttonStyleValue.removeListener (this);
            comboBoxTextEditableValue.removeListener (this);

            for (auto& np : namedProperties)
                np.value.removeListener (this);

            for (auto& np : colors)
                np.value.removeListener (this);

            for (auto& style : styles)
                style.value.removeListener (this);

            colors.clear();
            namedProperties.clear();
            styles.clear();
        }

        // allows properties to be set from our properties
        void valueChanged (juce::Value& value) override
        {
            TRACE_COMPONENT();

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
                else if (value.refersToSameSourceAs (isToggleable))
                {
                    if (auto button = dynamic_cast<juce::Button*> (selectedComponent.getComponent()))
                        button->setToggleable (isToggleable.getValue());
                }
                else if (value.refersToSameSourceAs (toggleState))
                {
                    if (auto button = dynamic_cast<juce::Button*> (selectedComponent.getComponent()))
                        button->setToggleState (toggleState.getValue(), juce::dontSendNotification);
                }
                else if (value.refersToSameSourceAs (clickTogglesState))
                {
                    if (auto button = dynamic_cast<juce::Button*> (selectedComponent.getComponent()))
                        button->setClickingTogglesState (clickTogglesState.getValue());
                }
                else if (value.refersToSameSourceAs (radioGroupId))
                {
                    if (auto button = dynamic_cast<juce::Button*> (selectedComponent.getComponent()))
                        button->setRadioGroupId (radioGroupId.getValue());
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

                    for (auto& nv : colors)
                    {
                        if (value.refersToSameSourceAs (nv.value))
                        {
                            selectedComponent->getProperties().set (nv.name, nv.value.getValue());
                            selectedComponent->repaint();
                            break;
                        }
                    }

                    for (auto& style : styles)
                    {
                        if (value.refersToSameSourceAs(style.value))
                        {
                            switch (style.type)
                            {
                                case StyleType::SliderStyle:
                                    if (auto* slider = dynamic_cast<juce::Slider*>(selectedComponent.getComponent()))
                                        slider->setSliderStyle(static_cast<juce::Slider::SliderStyle>(int(style.value.getValue())));
                                    break;

                                case StyleType::SliderTextPosition:
                                    if (auto* slider = dynamic_cast<juce::Slider*>(selectedComponent.getComponent()))
                                        slider->setTextBoxStyle(
                                            static_cast<juce::Slider::TextEntryBoxPosition>(int(style.value.getValue())),
                                            false,
                                            slider->getTextBoxWidth(),
                                            slider->getTextBoxHeight());
                                    break;

                                case StyleType::ButtonStyle:
                                    if (auto* button = dynamic_cast<juce::Button*>(selectedComponent.getComponent()))
                                    {
                                        // Handle button style changes
                                        selectedComponent->repaint();
                                    }
                                    break;

                                case StyleType::ComboBoxEditable:
                                    if (auto* comboBox = dynamic_cast<juce::ComboBox*>(selectedComponent.getComponent()))
                                        comboBox->setEditableText(style.value.getValue());
                                    break;
                            }
                            return; // Style was handled, exit early
                        }
                    }
                }
            }
        }

        void componentMovedOrResized (juce::Component&, bool wasMoved, bool wasResized) override
        {
            TRACE_COMPONENT();

            if (wasResized || wasMoved)
            {
                updateModel();
            }
        }

#if MELATONIN_HAS_PAINT_DIAGNOSTICS
        void componentPainted (juce::Component&, const juce::ComponentPaintDiagnostics& d) override
        {
            // This callback fires inside the paint cycle, so we keep it as
            // tight as possible — JUCE explicitly warns that time spent here
            // counts against the parent component's measured paint time.
            paintHistory.capture (d);
            triggerAsyncUpdate();
        }

        void handleAsyncUpdate() override
        {
            notifyPaintHistoryUpdated();
        }

        void notifyPaintHistoryUpdated()
        {
            listenerList.call ([this] (Listener& listener) {
                listener.componentModelPaintHistoryUpdated (*this);
            });
        }
#endif

        void notifyListeners()
        {
            listenerList.call ([this] (Listener& listener) {
                listener.componentModelChanged (*this);
            });
        }
    };
}
