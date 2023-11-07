#pragma once

#include <functional>
#include <queue>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>

class ThreadPool
{
public:
    ThreadPool(size_t numThreads) : stop(false)
    {
        for (size_t i = 0; i < numThreads; ++i)
        {
            workers.emplace_back(&ThreadPool::WorkerThread, this);
        }
    }
    ~ThreadPool()
    {
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            stop = true;
        }
        condition.notify_all();
        for (std::thread& worker : workers)
        {
            if (worker.joinable())
            {
                worker.join();
            }
        }
    }

    void EnqueueJob(std::function<void()> job)
    {
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            jobs.push(job);
        }
        condition.notify_one();
    }

private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> jobs;
    std::mutex queueMutex;
    std::condition_variable condition;
    bool stop;

    void WorkerThread()
    {
        while (true)
        {
            std::function<void()> job;
            {
                std::unique_lock<std::mutex> lock(queueMutex);
                condition.wait(lock, [this] { return stop || !jobs.empty(); });
                if (stop && jobs.empty())
                {
                    return;
                }
                job = jobs.front();
                jobs.pop();
            }
            job();
        }
    }
};