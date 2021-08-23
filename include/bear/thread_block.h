#pragma once

#include <thread>
#include <condition_variable>
#include <bear/functor.h>
#include <bear/bear_exception.h>
#include <bear/simple_ptr.h>
#include <iostream>

namespace bear
{
	class thread_block_data
	{
		std::mutex mtx;
		std::condition_variable inner_signal;
		bear::functor<void> work;

		bool inited = false;
		bool closed = false;
		bool inner_working = false;
		bool outer_working = false;

	public:
		std::thread thread;

		bool initialized()
		{
			return inited;
		}

		bool working = false;

		template<typename ..._T>
		void init(_T&& ... t)
		{
			thread = std::thread(std::forward<_T>(t) ...);

			std::unique_lock<std::mutex> lock(mtx);
			inner_signal.wait(lock, [this]()
				{
					return inited;
				});
		}

		~thread_block_data()
		{
			_wait();
			closed = true;
			_run([]() {});
		}

		template<typename _T>
		void _run(_T&& t)
		{
			work = std::forward<_T>(t);
			working = true;

			std::unique_lock<std::mutex> lock(mtx);
			inner_signal.wait(lock, [this]()
				{
					return !inner_working;
				});
			outer_working = true;
			inner_signal.notify_one();
		}

		void _wait()
		{
			if (!outer_working) return;

			std::unique_lock<std::mutex> lock(mtx);
			inner_signal.wait(lock, [this]()
				{
					return inner_working;
				});
			outer_working = false;
			inner_signal.notify_one();
		}


		void _pick()
		{
			std::unique_lock<std::mutex> lock(mtx);
			inited = true;
			inner_signal.notify_one();
			inner_signal.wait(lock);
			while (!closed)
			{
				inner_working = true;
				work();
				working = false;
				inner_signal.notify_one();
				inner_signal.wait(lock, [this]()
					{
						return !outer_working;
					});
				inner_working = false;
				inner_signal.notify_one();
				inner_signal.wait(lock, [this]()
					{
						return outer_working;
					});
			}
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

		void _check()
		{
			if (!m_data->initialized())
			{
				m_data->init(thread_block_data::vork_routine, m_data);
			}
		}
	public:

		enum priority
		{
			priority_default,
			priority_normal,
			priority_high,
			priority_low
		};

		template<typename _Res>
		static std::pair<thread_block, _Res* > create()
		{
			_Res* ptr;
			thread_block block;
			block.m_data->init(
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
			thread_block block;
			block.m_data->init(
				thread_block_data::vork_routine_res_maker<_Maker&&, res_t>,
				block.m_data,
				std::forward<_Maker>(mak),
				&ptr);

			return std::make_pair(std::move(block), ptr);
		}

		static void set_native_priority(std::thread::native_handle_type handle, thread_block::priority p);

		thread_block(priority p = priority_default)
		{
			m_data = m_data.create();

#if defined(THREAD_BLOCK_PRIORITY)
			set_native_priority(m_data->thread.native_handle(), p);
#endif
		}

		bool running()
		{
			_check();
			return m_data->working;
		}

		void wait()
		{
			_check();
			m_data->_wait();
		}

		template<typename _T>
		void run(_T&& work)
		{
			_check();
			m_data->_wait();
			m_data->_run(std::forward<_T>(work));
		}
	};

	inline void run_all(array_ptr<thread_block> threads)
	{
	}

	template<typename _Fun, typename ... _OF>
	inline void run_all(array_ptr<thread_block> threads, _Fun&& fun, _OF&& ... of)
	{
		auto& p = threads.front();

		p.run(std::forward<_Fun>(fun));

		run_all(threads.clip(1, threads.size()), std::forward<_OF>(of) ...);

		p.wait();
	}
}
