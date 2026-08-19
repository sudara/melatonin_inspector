#pragma once

#include <iostream>

namespace melatonin
{
    inline void IpcConnection::messageReceived (const juce::MemoryBlock& message)
    {
        juce::String jsonString = message.toString();
        std::cout << "Received IPC message: " << jsonString.toStdString() << std::endl;
        auto json = juce::JSON::parse (jsonString);

        if (json.isObject())
        {
            std::cout << "Parsed JSON successfully, dispatching..." << std::endl;
            juce::MessageManager::callAsync ([this, json]() {
                std::cout << "On Message Thread!" << std::endl;
                handleMessageOnMessageThread (json);
            });
        }
        else
        {
            std::cout << "Failed to parse JSON object" << std::endl;
        }
    }

    inline void IpcConnection::sendMessage (const juce::String& text)
    {
        juce::MemoryBlock mb (text.toRawUTF8(), text.getNumBytesAsUTF8());
        juce::InterprocessConnection::sendMessage (mb);
    }

    inline juce::var IpcConnection::serializeComponentTree (juce::Component* c)
    {
        if (c == nullptr)
            return {};

        juce::DynamicObject::Ptr obj (new juce::DynamicObject());
        obj->setProperty ("id", juce::String::toHexString ((juce::pointer_sized_int) c));
        obj->setProperty ("name", c->getName());
        obj->setProperty ("type", type (*c));

        auto bounds = c->getBounds();
        juce::DynamicObject::Ptr b (new juce::DynamicObject());
        b->setProperty ("x", bounds.getX());
        b->setProperty ("y", bounds.getY());
        b->setProperty ("w", bounds.getWidth());
        b->setProperty ("h", bounds.getHeight());
        obj->setProperty ("bounds", juce::var (b.get()));
        obj->setProperty ("visible", c->isVisible());

        juce::Array<juce::var> children;
        for (auto* child : c->getChildren())
        {
            children.add (serializeComponentTree (child));
        }
        if (!children.isEmpty())
            obj->setProperty ("children", children);

        return juce::var (obj.get());
    }

    inline juce::Component* IpcConnection::findComponentByPointerString (juce::Component* parent, const juce::String& ptrString)
    {
        if (!parent)
            return nullptr;
        if (juce::String::toHexString ((juce::pointer_sized_int) parent) == ptrString)
            return parent;

        for (auto* child : parent->getChildren())
        {
            if (auto* found = findComponentByPointerString (child, ptrString))
                return found;
        }
        return nullptr;
    }

    inline juce::DynamicObject::Ptr IpcConnection::getDetails (juce::Component* c)
    {
        juce::DynamicObject::Ptr details (new juce::DynamicObject());
        if (!c)
            return details;

        melatonin::ComponentModel model;
        model.selectComponent (c);

        details->setProperty ("name", model.nameValue.getValue().toString());
        details->setProperty ("type", model.typeValue.getValue().toString());
        details->setProperty ("lookAndFeel", model.lookAndFeelValue.getValue().toString());
        details->setProperty ("font", model.fontValue.getValue().toString());
        details->setProperty ("x", model.xValue.getValue());
        details->setProperty ("y", model.yValue.getValue());
        details->setProperty ("width", model.widthValue.getValue());
        details->setProperty ("height", model.heightValue.getValue());
        details->setProperty ("visible", model.visibleValue.getValue());
        details->setProperty ("enabled", model.enabledValue.getValue());
        details->setProperty ("opaque", model.opaqueValue.getValue());
        details->setProperty ("alpha", model.alphaValue.getValue());

        juce::DynamicObject::Ptr props (new juce::DynamicObject());
        for (auto& prop : model.namedProperties)
            props->setProperty (prop.name, prop.value.getValue());
        details->setProperty ("properties", juce::var (props.get()));

        juce::DynamicObject::Ptr colorsObj (new juce::DynamicObject());
        for (auto& col : model.colors)
            colorsObj->setProperty (col.name, col.value.getValue());
        details->setProperty ("colors", juce::var (colorsObj.get()));

        model.deselectComponent();
        return details;
    }

    inline void IpcConnection::handleMessageOnMessageThread (const juce::var& json)
    {
        juce::String action = json.getProperty ("action", "").toString();
        juce::String msgId = json.getProperty ("msg_id", "").toString();

        juce::DynamicObject::Ptr response (new juce::DynamicObject());
        response->setProperty ("msg_id", msgId);

        if (action == "ping")
        {
            response->setProperty ("status", "ok");
            response->setProperty ("data", "pong");
        }
        else if (action == "get_tree")
        {
            auto* root = inspector.getRootComponent();
            response->setProperty ("status", "ok");
            response->setProperty ("data", serializeComponentTree (root));
        }
        else if (action == "quit")
        {
            if (auto* app = juce::JUCEApplication::getInstance())
            {
                app->systemRequestedQuit();
                response->setProperty ("status", "ok");
                response->setProperty ("data", "Application exiting...");
            }
            else
            {
                response->setProperty ("status", "error");
                response->setProperty ("data", "No JUCEApplication instance found");
            }
        }
        else if (action == "get_details" || action == "highlight" || action == "click" || action == "screenshot" || action == "set_bounds")
        {
            auto* root = inspector.getRootComponent();
            juce::var args = json.getProperty ("args", juce::var());
            juce::String targetId = args.getProperty ("id", "").toString();
            auto* c = findComponentByPointerString (root, targetId);

            if (c)
            {
                if (action == "get_details")
                {
                    response->setProperty ("status", "ok");
                    response->setProperty ("data", juce::var (getDetails (c).get()));
                }
                else if (action == "highlight")
                {
                    inspector.outlineComponent (c);
                    response->setProperty ("status", "ok");
                }
                else if (action == "click")
                {
                    if (auto* btn = dynamic_cast<juce::Button*> (c))
                    {
                        btn->triggerClick();
                        response->setProperty ("status", "ok");
                    }
                    else
                    {
                        response->setProperty ("status", "error");
                        response->setProperty ("data", "Component is not a button");
                    }
                }
                else if (action == "screenshot")
                {
                    auto img = c->createComponentSnapshot (c->getLocalBounds());
                    juce::MemoryOutputStream mos;
                    juce::PNGImageFormat png;
                    png.writeImageToStream (img, mos);

                    auto b64 = juce::Base64::toBase64 (mos.getData(), mos.getDataSize());
                    response->setProperty ("status", "ok");
                    response->setProperty ("data", b64);
                }
                else if (action == "set_bounds")
                {
                    if (args.hasProperty ("x") && args.hasProperty ("y") && args.hasProperty ("w") && args.hasProperty ("h"))
                    {
                        c->setBounds ((int) args.getProperty ("x", 0), (int) args.getProperty ("y", 0), (int) args.getProperty ("w", 0), (int) args.getProperty ("h", 0));
                        response->setProperty ("status", "ok");
                    }
                    else
                    {
                        response->setProperty ("status", "error");
                        response->setProperty ("data", "Missing bounds arguments");
                    }
                }
            }
            else
            {
                response->setProperty ("status", "error");
                response->setProperty ("data", "Component not found");
            }
        }
        else
        {
            response->setProperty ("status", "error");
            response->setProperty ("data", "Unknown action: " + action);
        }

        sendMessage (juce::JSON::toString (response.get()));
    }
}
