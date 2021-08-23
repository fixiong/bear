
#if !defined(THREAD_BLOCK_PRIORITY)
#define THREAD_BLOCK_PRIORITY
#endif

#include <bear/thread_block.h>

#if defined(_WIN32)
#include <Windows.h>
#else
#include <pthread.h>
#endif

namespace bear
{
	void thread_block::set_native_priority(std::thread::native_handle_type handle, thread_block::priority p)
	{

#if defined(_WIN32)

		if (priority_normal == p || priority_default == p) return;

		DWORD dwPriorityClass = NORMAL_PRIORITY_CLASS;
		int nPriorityNumber = 8;
		if (priority_high == p)
		{
			dwPriorityClass = HIGH_PRIORITY_CLASS;
			nPriorityNumber = THREAD_PRIORITY_HIGHEST;
		}
		else
		{
			dwPriorityClass = BELOW_NORMAL_PRIORITY_CLASS;
			nPriorityNumber = THREAD_PRIORITY_LOWEST;
		}
		int result = SetPriorityClass(
			reinterpret_cast<HANDLE>(handle),
			dwPriorityClass);
		if (result != 0) {
			throw bear_exception(exception_type::multithread_error, "Setting priority class failed with ");
		}
		result = SetThreadPriority(
			reinterpret_cast<HANDLE>(handle),
			nPriorityNumber);
		if (result != 0) {
			throw bear_exception(exception_type::multithread_error, "Setting priority number failed with ");
		}
#else
		if (priority_default == p) return;

		auto low = sched_get_priority_min(SCHED_FIFO);
		auto high = sched_get_priority_max(SCHED_FIFO);

		sched_param sch_params;

		if (priority_low == p)
		{
			sch_params.sched_priority = sched_get_priority_min(SCHED_FIFO);
		}
		else if (priority_high == p)
		{
			sch_params.sched_priority = sched_get_priority_max(SCHED_FIFO);
		}
		else
		{
			sch_params.sched_priority = (sched_get_priority_min(SCHED_FIFO) + sched_get_priority_max(SCHED_FIFO)) / 2;
		}

		if (pthread_setschedparam(th.native_handle(), SCHED_FIFO, &sch_params)) {
			throw bear_exception(exception_type::multithread_error, "Failed to set Thread scheduling : ", std::strerror(errno));
		}
#endif

	}

}
