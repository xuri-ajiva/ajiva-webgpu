//
// Created by XuriAjiva on 20.08.2023.
//

#pragma once

#include "defines.h"

#include <functional>
#include <memory>
#include <thread>
#include <vector>
#include <future>
#include <queue>
#include <condition_variable>
#include <mutex>

#ifndef AJ_THREAD_POOL_LENGTH
#define AJ_THREAD_POOL_LENGTH 1024
#endif

namespace Ajiva::Core
{
    class AJ_API IThreadPool
    {
    public:
        virtual void QueueWork(const std::function<void()>& func, const std::function<void()>& callback = nullptr) = 0;

        AJ_INLINE virtual bool IsWorking() = 0;

        AJ_INLINE virtual bool IsFull() = 0;

        AJ_INLINE virtual bool IsEmpty() = 0;
    };

    template <u64 N = AJ_THREAD_POOL_LENGTH, u64 T = 8>
    class AJ_API ThreadPool : public IThreadPool
    {
        struct Worker
        {
            ThreadPool* pool;
            std::thread thread;
            u64 index = 0;
            bool working = false;
        };

        struct Work
        {
            std::function<void()> func;
            std::function<void()> callback;
        };

    public:
        ThreadPool() : ThreadPool(true)
        {
        }

        explicit ThreadPool(bool start)
        {
            head = 0;
            tail = 0;
            shutdown = false;
            started = false;
            waitFunc = [this]() { return !IsEmpty() || shutdown; };
            if (start)
            {
                Start();
            }
        }

        void Start()
        {
            if (started) return;
            started = true;
            for (u64 i = 0; i < T; ++i)
            {
                auto state = &states[i];
                state->pool = this;
                state->index = i;
                state->working = false;
                state->thread = std::thread([state]()
                {
                    state->pool->WorkerLoop(state);
                });
            }
        }

        ~ThreadPool()
        {
            Shutdown();
        }

        void Shutdown()
        {
            if (shutdown) return;
            {
                std::unique_lock<std::mutex> lock(mutex);
                shutdown = true;
            }
            cv.notify_all();
            for (u64 i = 0; i < T; ++i)
            {
                auto state = &states[i];
                if (state->thread.joinable())
                {
                    state->thread.join();
                }
            }
        }

        void WorkerLoop(ThreadPool::Worker* state)
        {
            PLOG_DEBUG << "WorkerLoop: " << state->index;
            while (!shutdown)
            {
                Work work;
                {
                    if (shutdown) break;
                    std::unique_lock<std::mutex> lock(mutex);
                    cv.wait(lock, waitFunc);
                    if (shutdown) break;
                    auto i = this->tail++;
                    work = std::move(works[i]);
                    state->working = true;
                }
                work.func();
                state->working = false;
                if (work.callback)
                {
                    work.callback();
                }
            }
            PLOG_DEBUG << "WorkerLoop: " << state->index << " end";
        }

        void QueueWork(const std::function<void()>& func, const std::function<void()>& callback = nullptr) override
        {
            if (shutdown) return;
            std::unique_lock<std::mutex> lock(mutex);
            if (shutdown) return;
            while (IsFull())
            {
                cv.wait(lock, [this]() { return !IsFull() || shutdown; });
            }
            auto i = this->head++;
            works[i].func = func;
            works[i].callback = callback;
            PLOG_DEBUG << "QueueWork: " << i << " " << head << " " << tail;
            cv.notify_one();
        }

        AJ_INLINE bool IsWorking() override
        {
            return tail < head;
        }

        AJ_INLINE bool IsFull() override
        {
            return tail - head >= N;
        }

        AJ_INLINE bool IsEmpty() override
        {
            return tail == head;
        }

    private:
        Worker states[T];
        Work works[N];

        std::atomic<u64> head; // last added index
        std::atomic<u64> tail; // next to be processed index

        bool shutdown = false;
        bool started = false;

        std::function<bool()> waitFunc;
        std::mutex mutex;
        std::condition_variable cv;
    };
} // Ajiva
// Core
