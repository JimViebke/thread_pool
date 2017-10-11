
#include <sstream>

#include "util.hpp"
#include "thread_pool.hpp"

thread_pool::task::task() {}



thread_pool::thread_pool::thread_pool(const unsigned count)
{
	threads.reserve(count);
	for (unsigned i = 0; i < count; ++i)
		threads.push_back(std::move(std::thread(&thread_pool::work, this)));
}

void thread_pool::thread_pool::add_task(std::shared_ptr<task> new_task)
{
	lock_type lock(mutex);

	// if the queue is still running
	if (running)
	{
		// add a task to the queue
		queue.push(new_task);
		// wake a sleeping thread
		cv.notify_one();
	}
}

size_t thread_pool::thread_pool::queue_size() const
{
	lock_type lock(mutex);
	return queue.size();
}

void thread_pool::thread_pool::stop()
{
	lock_type lock(mutex);
	// stop running tasks
	running = false;
	// wake all sleeping threads
	cv.notify_all();
}
void thread_pool::thread_pool::join_all()
{
	if (running) stop();

	// wait for all workers to finish
	for (auto & t : threads)
		t.join();
}

void thread_pool::thread_pool::work()
{
	while (true)
	{
		// create a shared pointer to a task
		std::shared_ptr<task> task;
		{
			lock_type lock(mutex);

			// if the queue is empty but still running, sleep
			while (queue.empty() && running)
				cv.wait(lock);

			// destroy thread if the thread pool has finished
			if (queue.empty() && !running) return;

			// get the next task
			task = queue.front();
			queue.pop();

			// debug
			std::stringstream ss;
			ss << std::this_thread::get_id() << "\ttook a task from the queue. Size = " << queue.size();
			util::print(ss.str());
		}

		try
		{
			// run the next task
			task->run();
		}
		catch (std::exception & ex)
		{
			std::stringstream ss;
			ss << "An unknown exception occurred while running a task: " << ex.what() << std::endl;
			util::print(ss.str());
		}
	}
}
