#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

class ThreadPool {
    struct Worker { std::thread th; };

public:
    explicit ThreadPool(size_t numThreads);
    ~ThreadPool();

    void schedule(const std::function<void(void)>& task);

    void wait();

private:
    void workerLoop(int id);

    bool done{false};                      
    size_t activeTasks{0};               

    std::mutex mtx;
    std::condition_variable cvTask;
    std::condition_variable cvDone;

    std::queue<std::function<void(void)>> tasks;
    std::vector<Worker> workers;
};

#endif 

