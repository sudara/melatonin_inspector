#pragma once

#include "juce_core/juce_core.h"
#include "juce_data_structures/juce_data_structures.h"
#include "juce_events/juce_events.h"
#include "juce_graphics/juce_graphics.h"
#include "juce_gui_basics/juce_gui_basics.h"

#include "../component_model.h"
#include "component_helpers.h"

namespace melatonin
{
    class Inspector;

    class IpcConnection : public juce::InterprocessConnection
    {
    public:
        explicit IpcConnection (Inspector& inspectorRef)
            : juce::InterprocessConnection (false, 0x2172746a), inspector (inspectorRef) {}

        void connectionMade() override {}
        void connectionLost() override {}

        void messageReceived (const juce::MemoryBlock& message) override;
        void handleMessageOnMessageThread (const juce::var& json);
        void sendMessage (const juce::String& text);

    private:
        Inspector& inspector;

        juce::var serializeComponentTree (juce::Component* c);
        juce::Component* findComponentByPointerString (juce::Component* parent, const juce::String& ptrString);
        juce::DynamicObject::Ptr getDetails (juce::Component* c);
    };

    class IpcServer : public juce::InterprocessConnectionServer
    {
    public:
        explicit IpcServer (Inspector& inspectorRef) : inspector (inspectorRef) {}

        ~IpcServer() override
        {
            stop();
            activeConnections.clear();
        }

        juce::InterprocessConnection* createConnectionObject() override
        {
            auto* connection = new IpcConnection (inspector);
            activeConnections.add (connection);
            return connection;
        }

        void start (int port = 8484)
        {
            beginWaitingForSocket (port);
        }

    private:
        Inspector& inspector;
        juce::OwnedArray<IpcConnection> activeConnections;
    };
}
