#pragma once
#include <queue>
#include <vector>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <atomic>

class TaskQueue
{
private:
	std::queue<std::function<void(void)>> tasks;
	std::mutex m;
	std::atomic<bool> continue_on;
	std::vector<std::thread> pool;
	std::condition_variable taskAdded;
	std::vector<std::thread> thread_pool;

public:
	TaskQueue(int num_threads) : pool(), tasks()
	{
		continue_on = true;

		for (int i = 0; i < num_threads; i++)
		{
			pool.emplace_back([&]() {
				while (continue_on)
				{
					std::function<void(void)> func;
					{
						std::unique_lock<std::mutex> lock(m);
						while (tasks.empty())
						{
							taskAdded.wait(lock);
							if (!continue_on)
							{
								return;
							}
						}
						func = tasks.front();
						tasks.pop();
					}
					func();
				}
			});
		}
	}

	~TaskQueue()
	{
		stop();
		for (int i = 0; i < pool.size(); i++)
		{
			if (pool[i].joinable())
			{
				pool[i].join();
			}
		}
	}

	void stop()
	{
		continue_on = false;
		taskAdded.notify_all();
	}

	void add_task(std::function<void()> task)
	{
		std::lock_guard<std::mutex> l(m);
		tasks.push(task);
		taskAdded.notify_one();
	}
};
