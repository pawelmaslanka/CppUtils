#pragma once

#include <iostream>
#include <vector>
#include <functional>
#include <utility>
#include <string>

namespace Utils {

/**
 * @class DeferQueue
 * @brief Manages deferred tasks to be executed LIFO at the end of the function scope.
 *
 * This class implements a Go-style defer mechanism. When the DeferQueue object is 
 * destroyed at the end of its scope, it executes all active tasks in a Last-In, 
 * First-Out (LIFO) order, which is essential for safely unwinding dependency chains.
 *
 * How to use:
 * 1. Initialize: Place the `DEFER_SCOPE;` macro at the very top of your function.
 * 2. Add tasks: Queue a task using `auto myTask = defer_task(myFunction, arg1, arg2);`
 * or using a lambda `auto myTask = defer_task([&](){ myFunction(arg1); });`
 * 3. Early Invocation: You can explicitly trigger the task early by calling 
 * the handle as a function: `myTask();`. Doing this will run the task 
 * immediately and cancel its scheduled execution from the queue, 
 * preventing it from running twice when the function ends.
 */
class DeferQueue {
private:
    struct Task {
        std::function<void()> mFunc;
        bool mActive = true;
    };
    
    std::vector<Task> mTasks;

public:
    ~DeferQueue() {
        for (auto it = mTasks.rbegin(); it != mTasks.rend(); ++it) {
            if (it->mActive && it->mFunc) {
                it->mFunc();
            }
        }
    }

    // Accepts any callable and any number of arguments, binding them together.
    template<typename F, typename... Args>
    size_t add(F&& f, Args&&... args) {
        auto boundTask = [fn = std::forward<F>(f), 
                          ...boundArgs = std::forward<Args>(args)]() mutable {
            // Execute the function with the stored arguments
            std::invoke(fn, boundArgs...); 
        };
        
        mTasks.push_back({std::move(boundTask), true});
        return mTasks.size() - 1;
    }

    void invoke(const size_t id) {
        if (id < mTasks.size() && mTasks[id].mActive) {
            mTasks[id].mFunc();
            mTasks[id].mActive = false; 
        }
    }

    void cancel(const size_t id) {
        if (id < mTasks.size()) {
            mTasks[id].mActive = false;
        }
    }
};

class DeferHandle {
public:
    DeferHandle(DeferQueue& q, const size_t id) : mQueue(&q), mId(id) {}
    
    void operator()() { mQueue->invoke(mId); }
    void cancel() { mQueue->cancel(mId); }
    
private:
    DeferQueue* mQueue;
    size_t mId;
};

#define DEFER_SCOPE DeferQueue _fn_scope_tracker
#define DEFER_TASK(...) DeferHandle(_fn_scope_tracker, _fn_scope_tracker.add(__VA_ARGS__))

} // namespace Utils
