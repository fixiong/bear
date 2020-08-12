#pragma once

#include <thread>
#include <mutex>
#include <bear/functor.h>
#include <bear/bear_exception.h>
#include <bear/simple_ptr.h>

namespace bear
{
	struct thread_block_data
	{
		functor<void> work;
		std::mutex lock;
	};

	class thread_block
	{
		using data_ptr = shared_smp_ptr<thread_block_data, true>;

		std::thread::id m_belong = std::this_thread::get_id();

		data_ptr m_data = make_shared_mt_smp<thread_block_data>();

		bool m_working = true;

		std::thread m_thread;

		static void vork_routine(data_ptr data)
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

		void check_thread()
		{
			if (m_belong != std::this_thread::get_id())
			{
				throw bear_exception(exception_type::multithread_error,
					"can't access thread_block from another thread!");
			}
		}

		void _wait()
		{
			if (!m_working) return;
			m_working = false;
			m_data->lock.lock();
		}

		void _run()
		{
			m_working = true;
			m_data->lock.unlock();
		}

	public:

		thread_block()
		{
			_wait();
			m_thread = std::thread(vork_routine, m_data);
		}

		~thread_block()
		{
			check_thread();

			_wait();
			m_data->work = functor<void>();
			_run();
		}

		void wait()
		{
			check_thread();

			_wait();
		}

		void run(functor<void>&& work)
		{
			check_thread();

			if (!work) throw bear_exception(exception_type::multithread_error,
				"work is empty!");

			_wait();
			m_data->work = std::move(work);
			_run();
		}

		void run(const functor<void>& work)
		{
			check_thread();

			if (!work) throw bear_exception(exception_type::multithread_error,
				"work is empty!");

			_wait();
			m_data->work = work;
			_run();
		}
	};
}