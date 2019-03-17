#include "search.hpp"

namespace goldfish
{
Search::Semaphore::Semaphore(int permits) : permits(permits) {}

void Search::Semaphore::acquire()
{
    std::unique_lock<std::mutex> lock(mutex);
    while (permits == 0)
    {
        condition.wait(lock);
    }
    permits--;
}

void Search::Semaphore::release()
{
    std::unique_lock<std::mutex> lock(mutex);
    permits++;
    condition.notify_one();
}

void Search::Semaphore::drain_permits()
{
    std::unique_lock<std::mutex> lock(mutex);
    permits = 0;
}

}  // namespace goldfish
