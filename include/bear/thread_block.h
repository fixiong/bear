#pragma once

#include <thread>
#include <mutex>
#include <bear/functor.h>
#include <bear/bear_exception.h>
#include <bear/simple_ptr.h>
#include <iostream>

namespace bear
{
	class thread_block_data
	{
		std::thread::id belong = std::this_thread::get_id();
		std::mutex lock;
		std::mutex rev_lock;
		bool inner_working = false;
		std::thread thread;
		functor<void> work = []() {};

		bool closed = false;

	public:

		bool outer_working = true;

		template<typename ..._T>
		explicit thread_block_data(_T&& ... t)
		{
			rev_lock.lock();
			thread = std::thread(std::forward<_T>(t) ...);
			_wait();
		}

		~thread_block_data()
		{
			try
			{
				check_thread();

				closed = true;

				_wait();
				_run([]() {});
			}
			catch (const bear::bear_exception& e)
			{
				std::cout << e.what();
			}
		}

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
			while (!inner_working) std::this_thread::sleep_for(std::chrono::milliseconds(1));
			rev_lock.unlock();
			lock.lock();
			outer_working = false;
			rev_lock.lock();
		}

		template<typename _T>
		void _run(_T&& t)
		{
			work = std::forward<_T>(t);
			outer_working = true;
			lock.unlock();
		}

		boid _pick()
		{
			lock.lock();
			while (!closed)
			{
				work();
				rev_lock.lock();
				inner_working = false;
				lock.unlock();
				rev_lock.unlock();
				while(outer_working) std::this_thread::sleep_for(std::chrono::milliseconds(1));
				lock.lock();
				inner_working = true;
			}
			lock.unlock();
		}

		static void vork_routine(shared_smp_ptr<thread_block_data, true> data)
		{
			data->_pick();
		}


		template<typename _Mak, typename _Res>
		static void vork_routine_res_maker(shared_smp_ptr<thread_block_data, true> data, _Mak maker, _Res** _res_ptr)
		{
			_Res res = maker();
			*_res_ptr = &res;
			data->_pick();
		}

		template<typename _Res>
		static void vork_routine_res_create(shared_smp_ptr<thread_block_data, true> data, _Res** _res_ptr)
		{
			_Res res;
			*_res_ptr = &res;
			data->_pick();
		}
	};

	class thread_block
	{
		shared_smp_ptr<thread_block_data, true> m_data;
		thread_block(int, int)
		{
		}
	public:

		template<typename _Res>
		static std::pair<thread_block, _Res* > create()
		{
			_Res* ptr;
			thread_block block(0,0);
			block.m_data.create(
				thread_block_data::vork_routine_res_create<_Res>,
				block.m_data,
				&ptr);

			return std::make_pair(std::move(block), ptr);
		}

		template<typename _Maker>
		static auto create(_Maker&& mak)
		{
			using res_t = typename std::decay<decltype(mak())>::type;
			res_t* ptr;
			thread_block block(0, 0);
			block.m_data.create(
				thread_block_data::vork_routine_res_maker<_Maker&&, res_t>,
				block.m_data,
				std::forward<_Maker>(mak)
				&ptr);

			return std::make_pair(std::move(block), ptr);
		}

		thread_block()
		{
			m_data.create(thread_block_data::vork_routine, m_data);
		}

		bool running()
		{
			return m_data->outer_working;
		}

		void wait()
		{
			m_data->check_thread();

			m_data->_wait();
		}

		template<typename _T>
		void run(_T&& work)
		{
			m_data->check_thread();

			m_data->_wait();
			m_data->_run(std::forward<_T>(work));
		}
	};
}