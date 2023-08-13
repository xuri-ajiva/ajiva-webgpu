//
// Created by XuriAjiva on 13.08.2023.
//

#pragma once

#include "defines.h"

//todo move to defines?? is this enough space?
#define MAX_MESSAGE_CODES 16384

#include <functional>

//#define AJ_EVENT_REGISTER(eventSystemToRegister, eventCodeToRegister, listenerToRegister) (eventSystemToRegister).RegisterEvent(eventCodeToRegister, listenerToRegister, [](Ajiva::Core::EventCode eventCode, void *sender, void *listener, const Ajiva::Core::EventContext &context) {
#define AJ_EVENT_REGISTER(eventSystemToRegister, eventCodeToRegister, listenerToRegisterCast, ...) \
(eventSystemToRegister).RegisterEvent(eventCodeToRegister, this,                 \
[](Ajiva::Core::EventCode eventCode, void *sender, void *listener, const Ajiva::Core::EventContext &event) { \
auto that = static_cast<listenerToRegisterCast *>(listener); \
__VA_ARGS__ return false; })

namespace Ajiva::Core {
    enum EventCode : u16 {
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
    union EventContext {
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
        union {
            struct {
                f64 xOffset;
                f64 yOffset;
            } wheel;
            struct {
                u32 x;
                u32 y;
                i32 button;
                i32 mods;
            } click;
            struct {
                u32 x;
                u32 y;
            } move;
        } mouse;
        struct {
            i32 x;
            i32 y;
            u32 width;
            u32 height;
        } windowRect;
        struct {
            u32 width;
            u32 height;
        } framebufferSize;

        struct { //todo add enum for keycode
            i32 key;
            i32 scancode;
            i32 action;
            i32 mods;
        } key;
    };
    static_assert(sizeof(EventContext) == 16);


    typedef std::function<bool(EventCode EventCode, void *sender, void *listener,
                               const EventContext &context)> EventCallback;
    struct RegisteredEvent {
        void *listener;
        EventCallback callback;
    };


    class EventSystem {
    public:
        EventSystem() = default;

        ~EventSystem();

        bool RegisterEvent(EventCode eventCode, void *listener, const EventCallback &callback);

        bool UnregisterEvent(EventCode eventCode, void *listener, const EventCallback &callback);

        bool FireEvent(EventCode eventCode, void *sender, const EventContext &context);

    private:
        std::vector<RegisteredEvent> registered[MAX_MESSAGE_CODES];

    };


}
