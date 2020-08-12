#pragma once

#include <thread>
#include <mutex>
#include <bear/functor.h>
#include <bear/bear_exception.h>
#include <bear/simple_ptr.h>

namespace bear
{
	class thread_block_data
	{
		std::thread::id belong = std::this_thread::get_id();
		bool working = true;
		std::mutex lock;
	public:
		std::thread thread;
		functor<void> work;

		void check_thread()
		{
			if (belong != std::this_thread::get_id())
			{
				throw bear_exception(exception_type::multithread_error,
					"can't access thread_block from another thread!");
			}
		}

		void _wait()
		{
			if (!working) return;
			working = false;
			lock.lock();
		}

		void _run()
		{
			working = true;
			lock.unlock();
		}

		static void vork_routine(shared_smp_ptr<thread_block_data, true> data)
		{
			for(;;)
			{
				data->lock.lock();
				defer df([&]() {
					data->lock.unlock();
				});
				if (!data->work) return;
				data->work();
			}
		}
	};

	class thread_block
	{
		shared_smp_ptr<thread_block_data, true> m_data = make_shared_mt_smp<thread_block_data>();

	public:

		thread_block()
		{
			m_data->_wait();
			m_data->thread = std::thread(thread_block_data::vork_routine, m_data);
		}

		~thread_block()
		{
			m_data->check_thread();

			m_data->_wait();
			m_data->work = functor<void>();
			m_data->_run();
		}

		void wait()
		{
			m_data->check_thread();

			m_data->_wait();
		}

		void run(functor<void>&& work)
		{
			m_data->check_thread();

			if (!work) throw bear_exception(exception_type::multithread_error,
				"work is empty!");

			m_data->_wait();
			m_data->work = std::move(work);
			m_data->_run();
		}

		void run(const functor<void>& work)
		{
			m_data->check_thread();

			if (!work) throw bear_exception(exception_type::multithread_error,
				"work is empty!");

			m_data->_wait();
			m_data->work = work;
			m_data->_run();
		}
	};
}