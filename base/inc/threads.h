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
	inline ThreadPool(int n_thread)
	{
		for (int i = 0; i < n_thread; ++i)
		{
			workers.emplace_back
			(
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

	/// Enqueue task and get future.
	template<class F, class... Args>
	inline std::future<typename std::result_of<F(Args...)>::type> enqueue(F&& f, Args&&... args)
	{
		using return_type = typename std::result_of<F(Args...)>::type;

		auto task = std::make_shared<std::packaged_task<return_type()>>(std::bind(std::forward<F>(f), std::forward<Args>(args)...));
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

	bool stop = false;
	std::mutex queue_mutex;
	std::condition_variable condition;
};
