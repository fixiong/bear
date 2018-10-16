#pragma once

#include "string_ptr.h"

namespace bear
{
	class bear_exception
	{
	public:


	private:

		void _init(size_t count)
		{
			_size = count;
		}

		template<typename ... _Ts>
		void _init(size_t count, const_string_ptr s, _Ts && ... arg)
		{
			auto e = s.end();
			for (auto b = s.begin(); b < e && count < sizeof(_what); ++b, ++count)
			{
				_what[count] = *b;
			}

			if (count < sizeof(_what))_init(count, std::forward<_Ts>(arg) ...);
			else _size = count;
		}

	public:

		template<typename ... _T>
		bear_exception(unsigned int type, _T && ...arg) :_type(type)
		{
			_init(0, std::forward<_T>(arg) ...);
		}

		bear_exception() :_size(0), _type(0) {}

		const_string_ptr what() const noexcept
		{
			return const_string_ptr(_what, _size);
		}

		unsigned int error_number() const noexcept
		{
			return _type;
		}

	protected:

		size_t _size;
		char _what[256];
		unsigned int _type;
	};


	inline void __on_bear_exception(unsigned int t, const char * e)
	{
		throw bear_exception(t, e);
	}
}