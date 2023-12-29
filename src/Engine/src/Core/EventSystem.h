//
// Created by XuriAjiva on 13.08.2023.
//

#pragma once

#include "defines.h"

//todo move to defines?? is this enough space?
#define MAX_MESSAGE_CODES 16384
#define AJ_EVENT_PARAMETERS Ajiva::Core::EventCode code, void *sender, const Ajiva::Core::EventContext &event
#define AJ_EVENT_PARAMETERS_CALL code, sender, event

#include <functional>


namespace Ajiva::Core
{
    enum EventCode : u16
    {
        None = 0,
        Quit = 1,
        KeyDown = 2,
        KeyUp = 3,
        MouseMove = 4,
        MouseButtonDown = 5,
        MouseButtonUp = 6,
        MouseScroll = 7,
        FramebufferResize = 8,
        WindowRectChange = 9,

        MaxInternalEvent = 0xFF,
    };

    union EventContext
    {
        i64 i64_[2];
        u64 u64_[2];

        f64 f64_[2];

        i32 i32_[4];
        u32 u32_[4];
        f32 f32_[4];

        i16 i16_[8];

        u16 u16_[8];

        i8 i8_[16];
        u8 u8_[16];

        char c_[16];

        union
        {
            struct
            {
                f64 xOffset;
                f64 yOffset;
            } wheel;

            struct
            {
                i32 x;
                i32 y;
                i32 button;
                i32 mods;
            } click;

            struct
            {
                i32 x;
                i32 y;
                i32 dx;
                i32 dy;
            } move;
        } mouse;

        struct
        {
            i32 x;
            i32 y;
            u32 width;
            u32 height;
        } windowRect;

        struct
        {
            u64 padding; // align with mouse/windowRect
            u32 width;
            u32 height;
        } framebufferSize;

        struct
        {
            //todo add enum for keycode
            i32 key;
            i32 scancode;
            i32 action;
            i32 mods;
        } key;
    };

    static_assert(sizeof(EventContext) == 16);

    class IListener
    {
    public:
        virtual ~IListener() = default;

        virtual bool operator()(AJ_EVENT_PARAMETERS) = 0;

        //encorce equality operator
        bool operator==(const IListener& other) const
        {
            return this == &other;
        }
    };

    template <typename D, typename R>
    class Listener : public IListener
    {
    public:
        typedef R (D::*Callback)(AJ_EVENT_PARAMETERS);

        bool operator()(AJ_EVENT_PARAMETERS) override
        {
            static_assert(std::is_same_v<R, bool> || std::is_same_v<R, void>,
                          "Return type must be bool or void.");

            if constexpr (std::is_same_v<R, bool>)
            {
                // Handle bool return type
                return (_instance->*callback)(AJ_EVENT_PARAMETERS_CALL);
            }
            else
            {
                (_instance->*callback)(AJ_EVENT_PARAMETERS_CALL);
                return false;
            }
        }

        Listener(D* instance, Callback wmFuncPtr)
        {
            _instance = instance;
            callback = wmFuncPtr;
        }

        ~Listener() override = default;

    private:
        D* _instance;
        Callback callback;
    };


    class EventSystem
    {
    public:
        EventSystem() = default;

        ~EventSystem();

        template <typename D>
        Ref<Listener<D, bool>> Add(EventCode eventCode, D* instance, Listener<D, bool>::Callback callback)
        {
            return Add<D, bool>(eventCode, instance, callback);
        }

        template <typename D>
        Ref<Listener<D, void>> Add(EventCode eventCode, D* instance, Listener<D, void>::Callback callback)
        {
            return Add<D, void>(eventCode, instance, callback);
        }

        bool FireEvent(EventCode eventCode, void* sender, const EventContext& context);

    private:
        std::vector<std::weak_ptr<IListener>> registered[MAX_MESSAGE_CODES];


        template <typename D, typename R>
        bool Remove(EventCode eventCode, Listener<D, R>* listener)
        {
            for (auto it = registered[eventCode].begin(); it != registered[eventCode].end(); ++it)
            {
                if (it->expired())
                {
                    registered[eventCode].erase(it);
                    return true;
                }
                else
                {
                    if (it->lock().get() == listener)
                    {
                        registered[eventCode].erase(it);
                        return true;
                    }
                }
            }
            return false;
        }

        template <typename D, typename R>
        Ref<Listener<D, R>> Add(EventCode eventCode, D* instance, Listener<D, R>::Callback callback)
        {
            std::shared_ptr<Listener<D, R>> listener(new Listener<D, R>(instance, callback),
                                                     [this, eventCode](Listener<D, R>* pi)
                                                     {
                                                         PLOG_DEBUG << "Listener " << pi << " unregistered!";
                                                         this->Remove<D, R>(eventCode, pi);
                                                         delete pi;
                                                     });
            registered[eventCode].push_back(listener);
            return listener;
        }
    };
}
