#include "thread-pool.h"
#include <stdexcept>

ThreadPool::ThreadPool(size_t numThreads) {
    for (size_t i = 0; i < numThreads; ++i) {
        workers.push_back( Worker{ std::thread(&ThreadPool::workerLoop, this, int(i)) } );
    }
}

void ThreadPool::schedule(const std::function<void(void)>& task) {
    if (!task)
        throw std::invalid_argument("Cannot schedule empty task");

    std::unique_lock<std::mutex> lock(mtx);
    if (done)
        throw std::runtime_error("ThreadPool is stopped");

    tasks.push(task);
    ++activeTasks;
    cvTask.notify_one();  
}

void ThreadPool::workerLoop(int /*id*/) {
    while (true) {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(mtx);
            cvTask.wait(lock, [&] { return done || !tasks.empty(); });
            if (done && tasks.empty())
                return;
            task = std::move(tasks.front());
            tasks.pop();
        }

        task();

        {
            std::unique_lock<std::mutex> lock(mtx);
            --activeTasks;
            if (activeTasks == 0)
                cvDone.notify_all(); 
        }
    }
}

void ThreadPool::wait() {
    std::unique_lock<std::mutex> lock(mtx);
    cvDone.wait(lock, [&] { return activeTasks == 0; });
}

ThreadPool::~ThreadPool() {
    {
        std::unique_lock<std::mutex> lock(mtx);
        done = true;
        cvTask.notify_all();
        cvDone.notify_all();
    }
    for (auto& w : workers) {
        if (w.th.joinable())
            w.th.join();
    }
}

