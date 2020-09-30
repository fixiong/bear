#pragma once

#include "bear_exception.h"
#include <sstream>

namespace bear
{
	inline void __string_format(std::ostringstream& os, const_string_ptr fmt)
	{
		auto f = fmt._split('%');
		if (!f.second.empty())
			throw bear_exception(exception_type::size_different, literal_u8("too more %!"));

		os << f.first;
	}

	template<typename _Fst, typename ... _Types>
	inline void __string_format(std::ostringstream &os, const_string_ptr fmt, _Fst fst, _Types&& ... _args)
	{
		auto f = fmt._split('%');
		if (f.second.empty())
			throw bear_exception(exception_type::size_different, literal_u8("too more argument!"));

		os << f.first << fst;
		__string_format(os, f.second.clip(1,f.second.size()), std::forward<_Types>(_args) ...);
	}

	template<typename ... _Types>
	inline std::string format(const_string_ptr fmt, _Types&& ... _args)
	{
		std::ostringstream os;
		__string_format(os, fmt, std::forward<_Types>(_args) ...);
		return os.str();
	}
}