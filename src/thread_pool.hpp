#pragma once

#include <vector>
#include <thread>
#include <mutex>
#include <queue>
#include <memory>

namespace thread_pool
{
	using mutex_type = std::mutex;
	using lock_type = std::unique_lock<std::mutex>;
	using condition_variable_type = std::condition_variable;

	class task
	{
	protected:
		task();
	public:
		virtual void run() = 0;
	};



	class thread_pool
	{
	public:
		thread_pool(const unsigned count);
		
		void add_task(std::shared_ptr<task> new_task);

		size_t queue_size() const;

		void stop();
		void join_all();

	private:
		void work();
		std::vector<std::thread> threads;

		bool running = true;

		std::queue<std::shared_ptr<task>> queue;
		mutable mutex_type mutex;
		condition_variable_type cv;
	};
}
