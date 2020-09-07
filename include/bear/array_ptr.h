#pragma once

#include "ptr_types.h"
#include <algorithm>
#include <assert.h>

namespace bear
{
	template<typename _Elm>
	class array_ptr
	{
	public:

		using normal_elm_type = typename std::remove_const<_Elm>::type;
		using normal_self = array_ptr<normal_elm_type>;
		using const_self = array_ptr<const _Elm>;

		using array_type = array_ptr;
		using size_type = size_t;
		using elm_type = _Elm;
		using elm_pointer = elm_type *;
		using value_type = _Elm;
		using pointer = value_type * ;
		using reference = value_type & ;
		using iterator = pointer;
		using reverse_iterator = std::reverse_iterator<iterator>;
		using difference_type = ptrdiff_t;

		static constexpr unsigned int dim = 1;

		using raw_type = typename std::conditional<
			std::is_const<_Elm>::value,
			const char,
			char >::type;

	private:


		pointer _pointer = 0;
		size_t _size = 0;

	public:

		template<typename _Alc>
		array_ptr(const std::vector<normal_elm_type, _Alc> & _ctn) :
			_pointer(_ctn.empty() ? 0 : &_ctn[0]),
			_size(_ctn.size()) {}

		template<typename _Alc>
		array_ptr(std::vector<normal_elm_type, _Alc> & _ctn) :
			_pointer(_ctn.empty() ? 0 : &_ctn[0]),
			_size(_ctn.size()) {}

		template<size_t Size>
		array_ptr(const std::array<normal_elm_type, Size> & _ctn) :
			_pointer(_ctn.empty() ? 0 : &_ctn[0]),
			_size(_ctn.size()) {}

		template<size_t Size>
		array_ptr(std::array<normal_elm_type, Size> & _ctn) :
			_pointer(_ctn.empty() ? 0 : &_ctn[0]),
			_size(_ctn.size()) {}

		template<typename _Trt_>
		array_ptr(const std::basic_string<normal_elm_type, _Trt_> & _ctn) :
			_pointer(_ctn.empty() ? 0 : &_ctn[0]),
			_size(_ctn.size()) {}

		template<typename _Trt_>
		array_ptr(std::basic_string<normal_elm_type, _Trt_> & _ctn) :
			_pointer(_ctn.empty() ? 0 : &_ctn[0]),
			_size(_ctn.size()) {}

		array_ptr() = default;
		array_ptr(const array_ptr &oth) = default;

		template<typename _Oe_>
		array_ptr(const array_ptr<_Oe_>& oth) :
			_pointer(oth.data()),
			_size(oth.size())
		{
			static_assert(
				is_memory_compatible<elm_type, _Oe_>::value,
				"element type not compatible!");
		}

		template<typename _Iter>
		array_ptr(_Iter _begin, _Iter _end) :
			_pointer(&*_begin),
			_size(_end - _begin) {}

		array_ptr(_Elm * _begin, size_t size) :
			_pointer(_begin),
			_size(size) {}

		array_ptr(_Elm * _begin, tensor_size<1> size) :
			_pointer(_begin),
			_size(size[0]) {}

		array_ptr<raw_type> to_memory() const
		{
			return array_ptr<raw_type>((raw_type *)_pointer, _size * sizeof(_Elm));
		}

		size_t size() const
		{
			return _size;
		}

		size_t total_size() const
		{
			return _size;
		}

		bool empty() const
		{
			return !_size;
		}

		iterator begin() const
		{
			return _pointer;
		}

		iterator end() const
		{
			return begin() + size();
		}

		reverse_iterator rbegin() const
		{
			return reverse_iterator(end());
		}

		reverse_iterator rend() const
		{
			return reverse_iterator(begin());
		}

		value_type & at(size_t i) const
		{
#ifdef _DEBUG
			assert(i < size());
#endif
			return *(_pointer + i);
		}

		value_type & operator[](size_t i) const
		{
			return at(i);
		}

		value_type & front() const
		{
			return *begin();
		}

		value_type & back() const
		{
			return *(_pointer + _size - 1);
		}

		constexpr difference_type move_step() const
		{
			return sizeof(value_type);
		}

		void move_pointer(difference_type step)
		{
			_pointer = pointer((char *)_pointer + step);
		}

		pointer data() const
		{
			return (pointer)_pointer;
		}

		constexpr bool is_plan() const
		{
			return true;
		}

		const array_ptr<elm_type> &plan() const
		{
			return *this;
		}

		template<typename _Fn>
		void for_each(_Fn && _fn) const
		{
			std::for_each(
				begin(),
				end(),
				std::forward<_Fn>(_fn));
		}

		void fill(elm_type _value) const
		{
			for_each([_value](value_type &v)
			{
				v = _value;
			});
		}

		inline auto clip(size_t start, size_t end) const
		{
#ifdef _DEBUG
			assert(end <= size() && start <= end);
#endif
			return array_ptr(data() + start, end - start);
		}

		template<typename _Oe>
		void copy(array_ptr<_Oe> oth) const
		{
			if (size() != oth.size())
				__on_bear_exception(
					exception_type::size_different,
					"copy source should be the same size!");

			auto e = end();
			auto s = oth.begin();
			for (auto b = begin(); b < e; ++b,++s)
			{
				*b = *s;
			}
		}

	private:
	};

	template<typename _Elm>
	inline size_t size(const array_ptr<_Elm>& arr)
	{
		return arr.size();
	}



	template<typename _Elm>
	inline std::vector<typename std::decay<_Elm>::type> 
		make_container(const array_ptr<_Elm> &arr)
	{
		return std::vector<typename std::decay<_Elm>::type>(arr.begin(), arr.end());
	}

	template<typename _Elm>
	inline auto make_array_ptr(_Elm * _start, size_t _size)
	{
		return array_ptr<_Elm>(_start, _size);
	}

	template<typename _Elm, typename _Oe_>
	inline auto ptr_cast(const array_ptr<_Oe_>& oth)
	{
		static_assert(
			is_memory_compatible<_Elm, _Oe_>::value,
			"element type not compatible!");

		return array_ptr<_Elm>((_Elm*)&oth[0], oth.size());
	}

	template<typename _T1, typename ... _T>
	struct __ps_count_arg
	{
		static constexpr size_t value = __ps_count_arg<_T...>::value + 1;
	};

	template<typename _T1>
	struct __ps_count_arg<_T1>
	{
		static constexpr size_t value = 1;
	};

	template<typename _T1, typename ... _Type>
	inline auto make_array(_T1&& t1, _Type&& ... t)
	{
		using _Elm = typename std::decay<_T1>::type;
		using _RType = std::array<_Elm, __ps_count_arg<_T1, _Type ...>::value>;
		return _RType{ std::forward<_T1>(t1), std::forward<_Type>(t) ... };
	}


	template<typename _T1, typename ... _Type>
	inline auto make_array_t(_T1&& t1, _Type&& ... t)
	{
		using _Elm = typename std::decay<_T1>::type;
		using _RType = std::array<_Elm, __ps_count_arg<_T1, _Type ...>::value>;
		return _RType{std::move(t1), std::forward<_Type>(t) ... };
	}

	template<typename _T1, typename ... _Type>
	inline auto make_array_t(const _T1& t1, _Type&& ... t)
	{
		using _Elm = typename std::decay<_T1>::type;
		using _RType = std::array<_Elm, __ps_count_arg<_T1, _Type ...>::value>;
		return _RType{ t1, std::forward<_Type>(t) ... };
	}


	template<typename _Elm, typename _Alc>
	inline array_ptr<_Elm> to_ptr(std::vector<_Elm, _Alc> & _ctn)
	{
		if (_ctn.empty())return array_ptr<_Elm>();
		return make_array_ptr(&_ctn[0], _ctn.size());
	}

	template<typename _Elm, typename _Alc>
	inline const_array_ptr<_Elm> to_ptr(const std::vector<_Elm, _Alc> & _ctn)
	{
		if (_ctn.empty())return const_array_ptr<_Elm>();
		return make_array_ptr(&_ctn[0], _ctn.size());
	}

	template<typename _Elm, size_t Size>
	inline array_ptr<_Elm> to_ptr(std::array<_Elm, Size> & _ctn)
	{
		if (_ctn.empty())return array_ptr<_Elm>();
		return make_array_ptr(&_ctn[0], _ctn.size());
	}

	template<typename _Elm, size_t Size>
	inline const_array_ptr<_Elm> to_ptr(const std::array<_Elm, Size> &_ctn)
	{
		if (_ctn.empty())return const_array_ptr<_Elm>();
		return make_array_ptr(&_ctn[0], _ctn.size());
	}


	template<typename _Stm, typename _Elm_>
	inline _Stm && operator >> (_Stm && stm, array_ptr<_Elm_> arr)
	{
		auto e = arr.end();
		for (auto b = arr.begin(); b < e; ++b)
		{
			std::forward<_Stm>(stm) >> *b;
		}
		return std::forward<_Stm>(stm);
	}


	template<typename _Stm, typename _Elm_>
	inline _Stm && operator << (_Stm && stm, array_ptr<_Elm_> arr)
	{
		std::forward<_Stm>(stm) << '{' << arr[0];
		auto e = arr.end();
		for (auto b = arr.begin() + 1; b < e; ++b)
		{
			std::forward<_Stm>(stm) << ',' << *b;
		}
		std::forward<_Stm>(stm) << '}';
		return std::forward<_Stm>(stm);
	}
}
