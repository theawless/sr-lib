#pragma once

#include <condition_variable>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <thread>
#include <queue>
#include <vector>

/// Thanks to https://github.com/progschj/ThreadPool
class ThreadPool {
private:
	std::vector<std::thread> workers;
	std::queue<std::function<void()>> tasks;

	bool stop;
	std::mutex queue_mutex;
	std::condition_variable condition;

public:
	/// Constructor.
	inline ThreadPool(size_t);

	/// Enqueue task and get future.
	template<class F, class... Args>
	inline std::future<typename std::result_of<F(Args...)>::type> enqueue(F&& f, Args&&... args);

	/// Join all threads.
	inline ~ThreadPool();
};

ThreadPool::ThreadPool(size_t threads) : stop(false)
{
	for (size_t i = 0; i < threads; ++i)
	{
		workers.emplace_back(
			[this]
		{
			while (true)
			{
				std::function<void()> task;
				{
					std::unique_lock<std::mutex> lock(this->queue_mutex);
					this->condition.wait(lock, [this] { return this->stop || !this->tasks.empty(); });
					if (this->stop && this->tasks.empty())
					{
						return;
					}
					task = std::move(this->tasks.front());
					this->tasks.pop();
				}

				task();
			}
		}
		);
	}
}

ThreadPool::~ThreadPool()
{
	{
		std::unique_lock<std::mutex> lock(queue_mutex);
		stop = true;
	}
	condition.notify_all();
	for (std::thread &worker : workers)
	{
		worker.join();
	}
}

template<class F, class... Args>
std::future<typename std::result_of<F(Args...)>::type> ThreadPool::enqueue(F&& f, Args&&... args)
{
	using return_type = typename std::result_of<F(Args...)>::type;

	auto task = std::make_shared<std::packaged_task<return_type()>>(std::bind(std::forward<F>(f), std::forward<Args>(args)...));

	std::future<return_type> res = task->get_future();
	{
		std::unique_lock<std::mutex> lock(queue_mutex);
		if (stop)
		{
			throw std::runtime_error("enqueue on stopped ThreadPool");
		}
		tasks.emplace([task]() { (*task)(); });
	}
	condition.notify_one();

	return res;
}
