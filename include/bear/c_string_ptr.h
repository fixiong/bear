#pragma once

#include "simple_ptr.h"


namespace bear
{
	template<typename _Elm, typename _Trt = std::char_traits<typename std::decay<_Elm>::type>>
	struct basic_c_string_ptr
	{
	private:

		static int _cmp(_Elm * p, _Elm * q)
		{
			int r = (int)(*p - *q);
			for (; !r && *p; ++p, ++q, r = (int)(*p - *q));
			return r;
		}

	public:


		using traits_type = _Trt;

		_Elm * value = 0;

		basic_c_string_ptr() = default;
		basic_c_string_ptr(const basic_c_string_ptr &oth) = default;

		template<typename _Oe>
		basic_c_string_ptr(_Oe * _cs) : value(_cs)
		{
			static_assert(std::is_convertible<_Oe *, _Elm *>::value, "element type not compatible!");
		}

		operator basic_string_ptr<_Elm, _Trt>()
		{
			return basic_string_ptr<_Elm, _Trt>(value);
		}

		operator std::basic_string<typename std::decay<_Elm>::type, _Trt>()
		{
			return value;
		}

		friend bool operator == (const basic_c_string_ptr &lr, const basic_c_string_ptr &rr)
		{
			return !_cmp(lr.value, rr.value);
		}

		friend bool operator != (const basic_c_string_ptr &lr, const basic_c_string_ptr &rr)
		{
			return _cmp(lr.value, rr.value);
		}

		friend bool operator > (const basic_c_string_ptr &lr, const basic_c_string_ptr &rr)
		{
			return _cmp(lr.value, rr.value) > 0;
		}

		friend bool operator < (const basic_c_string_ptr &lr, const basic_c_string_ptr &rr)
		{
			return _cmp(lr.value, rr.value) < 0;
		}

		friend bool operator >= (const basic_c_string_ptr &lr, const basic_c_string_ptr &rr)
		{
			return _cmp(lr.value, rr.value) >= 0;
		}

		friend bool operator <= (const basic_c_string_ptr &lr, const basic_c_string_ptr &rr)
		{
			return _cmp(lr.value, rr.value) <= 0;
		}
	};

	using c_string_ptr = basic_c_string_ptr<char, std::char_traits<char>>;
	using c_wstring_ptr = basic_c_string_ptr<wchar_t, std::char_traits<wchar_t>>;
	using c_u16string_ptr = basic_c_string_ptr<char16_t, std::char_traits<char16_t>>;
	using c_u32string_ptr = basic_c_string_ptr<char32_t, std::char_traits<char32_t>>;


	using const_c_string_ptr = basic_c_string_ptr<const char, std::char_traits<char>>;
	using const_c_wstring_ptr = basic_c_string_ptr<const wchar_t, std::char_traits<wchar_t>>;
	using const_c_u16string_ptr = basic_c_string_ptr<const char16_t, std::char_traits<char16_t>>;
	using const_c_u32string_ptr = basic_c_string_ptr<const char32_t, std::char_traits<char32_t>>;
}