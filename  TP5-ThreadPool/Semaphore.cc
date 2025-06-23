#include "Semaphore.h"

Semaphore::Semaphore(int count) : count_(count) {}

void Semaphore::signal() {
    std::unique_lock<std::mutex> lock(mutex_);
    ++count_;
    condition_.notify_one();
}

void Semaphore::wait() {
    std::unique_lock<std::mutex> lock(mutex_);
    condition_.wait(lock, [this]() { return count_ > 0; });
    --count_;
}
