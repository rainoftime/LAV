#ifndef THREADPOOL_H
#define THREADPOOL_H

// Project headers
#include "CancelableThread.h"
#include "Event.h"
#include "SignalingThread.h"
#include "FixedQueue.h"

// STL includes
#include <thread>
#include <vector>
#include <functional>

using namespace ThreadSafe;

namespace Utils
{
class ThreadPool
{
public:

    enum Outcome {
        Unsat = 1,
        Finished = 2,
        Canceled = 3
    };

    ThreadPool(FixedQueue<std::function<int()>>&& tasks, int num = std::thread::hardware_concurrency() - 1);
    ThreadPool(ThreadPool&& t);
    ThreadPool& operator=(ThreadPool&& oth);
    ~ThreadPool();



    // API
    bool IsActive();
    void ShutDown();
    void Cancel();
    //void Add(const typename FixedQueue<std::function<int()>>::ValueType& newValue);

private:
   FixedQueue<std::function<int()>> m_tasks;
   std::vector<SignalingThread> m_threads;
   std::thread m_control_thread;
   Event::Pointer m_cancel_event;
   bool m_active;

};

}

#endif // THREADPOOL_H