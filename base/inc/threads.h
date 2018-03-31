#pragma once

#include <condition_variable>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <thread>
#include <queue>
#include <vector>

/// Thanks to https://github.com/progschj/ThreadPool
class ThreadPool
{
public:
	/// Constructor.
	inline ThreadPool(int n_thread) :
		workers(), tasks(), stop(), queue_mutex(), condition()
	{
		for (int i = 0; i < n_thread; ++i)
		{
			workers.push_back(std::move(std::thread(&ThreadPool::worker, this)));
		}
	}

	/// Enqueue task and get future.
	template<class F, class... Args>
	inline std::future<typename std::result_of<F(Args...)>::type> enqueue(F&& f, Args&&... args)
	{
		using return_type = typename std::result_of<F(Args...)>::type;
		std::shared_ptr<std::packaged_task<return_type()>> task(new std::packaged_task<return_type()>(std::bind(std::forward<F>(f), std::forward<Args>(args)...)));
		std::future<return_type> res = task->get_future();
		{
			std::unique_lock<std::mutex> lock(queue_mutex);
			tasks.emplace([task]() { (*task)(); });
		}
		condition.notify_one();

		return res;
	}

	/// Join all threads.
	inline ~ThreadPool()
	{
		{
			std::unique_lock<std::mutex> lock(queue_mutex);
			stop = true;
		}
		condition.notify_all();
		for (int i = 0; i < workers.size(); ++i)
		{
			workers[i].join();
		}
	}

private:
	std::vector<std::thread> workers;
	std::queue<std::function<void()>> tasks;

	bool stop;
	std::mutex queue_mutex;
	std::condition_variable condition;

	/// Indefinitely fetch tasks from the queue.
	void worker()
	{
		while (true)
		{
			std::function<void()> task;
			{
				std::unique_lock<std::mutex> lock(queue_mutex);
				const std::function<bool()> pred = [&]()
				{
					return stop || !tasks.empty();
				};
				condition.wait(lock, pred);
				if (stop && tasks.empty())
				{
					return;
				}

				task = std::move(tasks.front());
				tasks.pop();
			}

			task();
		}
	}
};
