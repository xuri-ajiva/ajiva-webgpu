//
// Created by XuriAjiva on 13.08.2023.
//

#include "EventSystem.h"
#include "Core/Logger.h"

namespace Ajiva::Core {
    bool EventSystem::RegisterEvent(EventCode eventCode, void *listener, const EventCallback &callback) {
        if (eventCode >= MAX_MESSAGE_CODES) {
            AJ_FAIL("EventCode >= MAX_MESSAGE_CODES, Increase MAX_MESSAGE_CODES!");
        }

        for (auto &registeredEvent: registered[eventCode]) {
            if (registeredEvent.listener == listener && &registeredEvent.callback == &callback) {
                PLOG_WARNING << "Event " << eventCode << " already registered for listener " << listener
                             << " and callback " << &callback << "!";
                return false;
            }
        }

        //todo check if this works without constructing the vector, should be possible
        registered[eventCode].emplace_back(listener, callback);

        return true;
    }

    bool EventSystem::UnregisterEvent(EventCode eventCode, void *listener, const EventCallback &callback) {
        if (eventCode >= MAX_MESSAGE_CODES) {
            AJ_FAIL("EventCode >= MAX_MESSAGE_CODES, Increase MAX_MESSAGE_CODES!");
        }

        for (auto it = registered[eventCode].begin(); it != registered[eventCode].end(); ++it) {
            if (it->listener == listener && &it->callback == &callback) {
                registered[eventCode].erase(it);
                return true;
            }
        }

        PLOG_WARNING << "Event " << eventCode << " not registered for listener " << listener << " and callback "
                     << &callback << "!";
        return false;
    }

    bool EventSystem::FireEvent(EventCode eventCode, void *sender, const EventContext &context) {
        if (eventCode >= MAX_MESSAGE_CODES) {
            AJ_FAIL("EventCode >= MAX_MESSAGE_CODES, Increase MAX_MESSAGE_CODES!");
        }
        //PLOG_VERBOSE << "Firing event " << eventCode << " from " << sender << " with context " << &context << "!";
        bool handled = false;
        for (auto &registeredEvent: registered[eventCode]) {
            if (registeredEvent.callback(eventCode, sender, registeredEvent.listener, context)) {
                return true;
            }
            handled = true;
        }

        return
                handled;
    }

    EventSystem::~EventSystem() {
        for (auto &registeredEvent: registered) {
            if (!registeredEvent.empty()) {
                PLOG_WARNING << "Event " << &registeredEvent << " not unregistered!";
            }
            registeredEvent.clear();
        }
    }

}