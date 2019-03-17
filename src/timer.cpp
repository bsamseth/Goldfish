#include "search.hpp"

namespace goldfish
{
Search::Timer::Timer(bool&        timer_stopped,
                     bool&        do_time_management,
                     Depth&       current_depth,
                     const Depth& initial_depth,
                     bool&        abort)
    : timer_stopped(timer_stopped)
    , do_time_management(do_time_management)
    , current_depth(current_depth)
    , initial_depth(initial_depth)
    , abort(abort)
{
}

void Search::Timer::run(uint64_t search_time)
{
    std::unique_lock<std::mutex> lock(mutex);
    if (condition.wait_for(lock, std::chrono::milliseconds(search_time))
        == std::cv_status::timeout)
    {
        timer_stopped = true;

        // If we finished the first iteration, we should have a result.
        // In this case abort the search.
        if (!do_time_management || current_depth > initial_depth)
        {
            abort = true;
        }
    }
}

void Search::Timer::start(uint64_t search_time)
{
    thread = std::thread(&Search::Timer::run, this, search_time);
}

void Search::Timer::stop()
{
    condition.notify_all();
    thread.join();
}

}  // namespace goldfish
