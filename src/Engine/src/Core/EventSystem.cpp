//
// Created by XuriAjiva on 13.08.2023.
//

#include "EventSystem.h"
#include "Core/Logger.h"
#include "magic_enum.hpp"

namespace Ajiva::Core
{
    bool EventSystem::FireEvent(EventCode eventCode, void* sender, const EventContext& context)
    {
        if (eventCode >= MAX_MESSAGE_CODES)
        {
            AJ_FAIL("EventCode >= MAX_MESSAGE_CODES, Increase MAX_MESSAGE_CODES!");
        }
        bool handled = false;
        try
        {
            for (auto it = registered[eventCode].begin(); it != registered[eventCode].end();)
            {
                if (it->expired())
                {
                    PLOG_WARNING << "Event " << magic_enum::enum_name<>(eventCode)
                                 << " has expired listener! Removing...";
                    it = registered[eventCode].erase(it);
                    continue;
                }
                if ((*it->lock())(eventCode, sender, context))
                {
                    return true;
                }
                else
                {
                    handled = true;
                }
                ++it;
            }
        }
        catch (std::exception& e)
        {
            PLOG_ERROR << "Exception while firing event " << magic_enum::enum_name<>(eventCode) << " from " << sender
                       << " with context "
                       << &context << "!";
            PLOG_ERROR << e.what();
            return false;
        }
        return handled;
    }

    EventSystem::~EventSystem()
    {
        for (auto& registeredEvent : registered)
        {
            if (!registeredEvent.empty())
            {
                PLOG_WARNING << "Event " << &registeredEvent << " not unregistered!";
            }
            registeredEvent.clear();
        }
    }
}
