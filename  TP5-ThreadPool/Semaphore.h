
#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include <condition_variable>
#include <mutex>

class Semaphore {
public:
    explicit Semaphore(int count = 0);
    void signal();
    void wait();

private:
    std::mutex mutex_;
    std::condition_variable condition_;
    int count_;
};

#endif

