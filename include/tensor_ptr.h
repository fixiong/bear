#pragma once

#include "array_ptr.h"
#include "array_iterator.h"

namespace bear
{

	template<size_t _Sz>
	inline tensor_size<_Sz - 1> &sub_size(tensor_size<_Sz> &sz)
	{
		static_assert(_Sz > 1, "size too small!");
		return *(tensor_size<_Sz - 1> *)&sz[1];
	}

	template<size_t _Sz>
	inline const tensor_size<_Sz - 1> &sub_size(const tensor_size<_Sz> &sz)
	{
		static_assert(_Sz > 1, "size too small!");
		return *(tensor_size<_Sz - 1> *)&sz[1];
	}

	template<size_t _Sz>
	inline size_t total_size(const tensor_size<_Sz> &sz)
	{
		size_t ret = sz[0];

		for (int i = 1; i < _Sz; ++i)
		{
			ret *= sz[i];
		}

		return ret;
	}

	template<size_t _Sz>
	inline bool operator == (const tensor_size<_Sz> &ls, const tensor_size<_Sz> &rs)
	{
		for (int i = 0; i < _Sz; ++i)
		{
			if (ls[i] != rs[i])return false;
		}
		return true;
	}

	template<size_t _Sz>
	inline bool operator != (const tensor_size<_Sz> &ls, const tensor_size<_Sz> &rs)
	{
		for (int i = 0; i < _Sz; ++i)
		{
			if (ls[i] != rs[i])return true;
		}
		return false;
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

	inline void __pack_tensor_size(tensor_size<1> &pk, size_t sz)
	{
		pk[0] = sz;
	}

	template<typename ... _T, size_t _Dim>
	inline void __pack_tensor_size(tensor_size<_Dim> &pk, size_t sz, _T ... sizes)
	{
		pk[0] = sz;
		__pack_tensor_size(sub_size(pk), sizes ...);
	}

	template<typename ... _T>
	inline auto make_tensor_size(_T ... sizes)
	{
		tensor_size<__ps_count_arg<_T ...>::value> ret;
		__pack_tensor_size(ret, sizes ...);
		return ret;
	}




	template<typename _Base>
	class base_tensor_ptr
	{
	public:
		using value_type = _Base;

		using normal_self = base_tensor_ptr<typename value_type::normal_self>;
		using const_self = base_tensor_ptr<typename value_type::const_self>;

		using elm_type = typename value_type::elm_type;
		using elm_pointer = elm_type * ;

		using array_type = typename value_type::array_type;

		using size_type = typename value_type::size_type;
		//using reference = value_type & ;
		//using pointer = value_type * ;
		using difference_type = typename value_type::difference_type;

		using iterator = array_iterator<value_type>;
		using const_iterator = array_iterator<typename value_type::const_self>;

		using reverse_iterator = reverse_array_iterator<value_type>;
		using const_reverse_iterator = reverse_array_iterator<typename value_type::const_self>;

		static constexpr size_t dim = value_type::dim + 1;

		using deep_size_type = tensor_size<dim>;

	private:

		iterator _pointer;
		size_t _size;

	public:

		base_tensor_ptr() = default;
		base_tensor_ptr(const base_tensor_ptr &oth) = default;

		template<typename ... _T>
		base_tensor_ptr(elm_pointer _elm, size_t _sz, _T ... _sizes) :
			_pointer(value_type(_elm, _sizes ...)),
			_size(_sz) {}

		base_tensor_ptr(elm_pointer _elm, deep_size_type _size) :
			_pointer(value_type(_elm, sub_size(_size))),
			_size(_size[0]) {}

		base_tensor_ptr(const value_type &base, size_t size) :
			_pointer(base),
			_size(size) {}

		base_tensor_ptr(const value_type &base, size_t size, difference_type _step) :
			_pointer(base, _step),
			_size(size){}

		template<typename _Oe_>
		base_tensor_ptr(const base_tensor_ptr<_Oe_> &oth) :
			_pointer(oth.begin()),
			_size(oth.size())
		{
			static_assert(
				is_memory_compatible<elm_type, typename _Oe_::elm_type>::value,
				"element type not compatible!");
		}


		size_t size() const
		{
			return _size;
		}

		size_t total_size() const
		{
			return _size * _pointer->total_size();
		}

		bool empty() const
		{
			return !_size;
		}


		const iterator &begin() const
		{
			return _pointer;
		}

		iterator end() const
		{
			return begin() + (difference_type)size();
		}

		reverse_iterator rbegin() const
		{
			return reverse_iterator(end());
		}

		reverse_iterator rend() const
		{
			return reverse_iterator(begin());
		}

		value_type at(size_t i) const
		{
#ifdef _DEBUG
			assert(i < size());
#endif
			return *(begin() + (difference_type)i);
		}

		value_type operator[](size_t i) const
		{
			return at(i);
		}

		const value_type &front() const
		{
			return *_pointer;
		}

		value_type back() const
		{
			return *(begin() + (_size - 1));
		}

		difference_type move_step() const
		{
			return _pointer.move_step();
		}

		void move_pointer(difference_type _step)
		{
			_pointer.move_pointer(_step);
		}

		auto data() const
		{
			return _pointer->data();
		}


		bool is_plan() const
		{
			return _pointer.is_plan();
		}

		auto plan() const
		{
			if (!is_plan())return array_ptr<elm_type>();
			return make_array(_pointer->data(), total_size());
		}

		template<typename _Fn>
		void for_each(_Fn && _fn) const
		{
			auto ary = plan();
			if (!ary.empty())
			{
				ary.for_each(std::forward<_Fn>(_fn));
				return;
			}

			for (auto i = begin(); i < end(); ++i)
			{
				i->for_each(std::forward<_Fn>(_fn));
			}
		}

		void fill(elm_type _value) const
		{
			for_each([_value](elm_type &v)
			{
				v = _value;
			});
		}

		inline auto clip(size_t start, size_t end) const
		{
			if (end > size())end = size();
			if (start > end)start = end;
			return base_tensor_ptr(at(start), end - start);
		}

		template<typename ... _T>
		inline auto clip(size_t start, size_t end, _T ... arg) const
		{
#ifdef _DEBUG
			assert(end <= size() && start <= end);
#endif
			return base_tensor_ptr(_pointer->clip(arg ...),
				end - start, _pointer.move_step());

		}

		template<typename _Oe>
		inline void copy(const base_tensor_ptr<_Oe> &oth) const
		{
			if (size() != oth.size())
				__on_bear_exception(
					exception_type::size_different,
					"copy source should be the same size!"
				);

			auto e = end();
			auto s = oth.begin();
			for (auto b = begin(); b < e; ++b, ++s)
			{
				b->copy(*s);
			}
		}
	};


	template<size_t _Dim>
	struct _tensor_clip_at
	{
		template<typename _T>
		static auto _run(const _T & _oth, size_t _start, size_t _end)
		{
			return _T(_tensor_clip_at<_Dim - 1>::_run(_oth.front(), _start, _end),
				_oth.size(), _oth.move_step());
		}
	};

	template<>
	struct _tensor_clip_at<0>
	{
		template<typename _T>
		static auto _run(const _T & _oth, size_t _start, size_t _end)
		{
			return _oth.clip(_start, _end);
		}
	};

	template<size_t _Dim, typename _T>
	inline auto clip_at(_T & _oth, size_t _start, size_t _end)
	{
		return _tensor_clip_at<_Dim>::_run(to_ptr(_oth), _start, _end);
	}


	template<size_t _Dim, typename _T>
	inline auto clip_at(const _T & _oth, size_t _start, size_t _end)
	{
		return _tensor_clip_at<_Dim>::_run(to_ptr(_oth), _start, _end);
	}




	template<typename _Elm>
	static void __get_size_array(tensor_size<1> & sz, const array_ptr<_Elm> & t)
	{
		sz[0] = t.size();
	}

	template<typename _Base, size_t _D>
	static void __get_size_array(tensor_size<_D> & sz, const base_tensor_ptr<_Base> & t)
	{
		sz[0] = t.size();
		__get_size_array(sub_size(sz), t.front());
	}

	template<typename _Base>
	inline tensor_size<base_tensor_ptr<_Base>::dim> size(const base_tensor_ptr<_Base> & t)
	{
		tensor_size<base_tensor_ptr<_Base>::dim> ret;

		__get_size_array(ret, t);

		return ret;
	}


	template<typename _Elm>
	inline auto make_tensor(_Elm * _start, size_t _size)
	{
		return array_ptr<_Elm>(_start, _size);
	}

	template<typename _T>
	inline auto make_tensor(const array_ptr<_T> & oth, size_t _size)
	{
		return base_tensor_ptr<array_ptr<_T>>(oth, _size);
	}

	template<typename _T>
	inline auto make_tensor(const base_tensor_ptr<_T> & oth, size_t _size)
	{
		return base_tensor_ptr<base_tensor_ptr<_T>>(oth, _size);
	}

	template<typename _T>
	inline auto make_tensor(const array_ptr<_T> & oth, size_t _size, size_t _step)
	{
		return base_tensor_ptr<array_ptr<_T>>(oth, _size, _step);
	}

	template<typename _T>
	inline auto make_tensor(const base_tensor_ptr<_T> & oth, size_t _size, size_t _step)
	{
		return base_tensor_ptr<base_tensor_ptr<_T>>(oth, _size, _step);
	}



	template<typename _T1>
	inline bool is_plan(
		const _T1 &t1)
	{
		return t1.is_plan();
	}

	template<typename _T1, typename ... _Ts>
	bool is_plan(
		const _T1 &t1,
		const _Ts & ... ts)
	{
		return t1.is_plan() && is_plan(ts ...);
	}

	template<typename _Elm>
	inline auto __enlage(const array_ptr<_Elm> &oth, size_t sz, size_t up_step)
	{

//#ifdef _DEBUG
//		assert(up_step == oth.size() * oth.move_step());
//#endif // _DEBUG

		return array_ptr<_Elm>(oth.data(), sz * oth.size());
	}

	template<typename _Elm>
	inline auto __enlage(const base_tensor_ptr<_Elm> &oth, size_t sz, size_t up_step)
	{
		if (up_step == oth.size() * oth.move_step())
		{
			return base_tensor_ptr<_Elm>(oth.front(), sz * oth.size(), oth.move_step());
		}

//#ifdef _DEBUG
//		assert(1 == oth.size());
//#endif // _DEBUG

		return base_tensor_ptr<_Elm>(oth.front(), sz, up_step);
	}


	template<typename _Elm>
	inline auto __ensmall(const array_ptr<_Elm> &oth, size_t sz)
	{
		return array_ptr<_Elm>(oth.data(), oth.size() / sz);
	}

	template<typename _Elm>
	inline auto __ensmall(const base_tensor_ptr<_Elm> &oth, size_t sz)
	{
		return base_tensor_ptr<_Elm>(oth.front(), oth.size() / sz, oth.move_step());
	}

	template<typename _Elm>
	inline array_ptr<_Elm> _shrink(const array_ptr<_Elm> &oth)
	{
		return oth;
	}

	template<typename _Elm>
	inline base_tensor_ptr<_Elm> _shrink(const base_tensor_ptr<_Elm> &oth)
	{
		if (oth.move_step() == oth.front().move_step() * oth.front().size())
		{
			return base_tensor_ptr<_Elm>(
				_shrink(__enlage(oth.front(), oth.size(), oth.move_step())), 1,
				oth.move_step() * oth.size());
		}

		auto sub = _shrink(oth.front());

		if (sub.size() == 1)
		{
			return base_tensor_ptr<_Elm>(__enlage(sub, oth.size(), oth.move_step()), 1, oth.move_step() * oth.size());
		}

		return base_tensor_ptr<_Elm>(sub, oth.size(), oth.move_step());
	}

	template<typename _Elm>
	inline base_tensor_ptr<_Elm> shrink(const base_tensor_ptr<_Elm> &oth)
	{
		if (oth.is_plan())
		{
			tensor_size<base_tensor_ptr<_Elm>::dim> sizes;

			to_ptr(sizes).fill(1);
			sizes.back() = oth.total_size();

			return base_tensor_ptr<_Elm>(
				oth.data(),
				sizes
			);
		}

		return _shrink(oth);
	}

	template<typename _Elm>
	inline const array_ptr<_Elm> &_reshape(const array_ptr<_Elm> &oth, const tensor_size<1> &sizes)
	{
		return oth;
	}

	template<typename _Elm, size_t _Dim>
	inline tensor_ptr<_Elm, _Dim> _reshape(const array_ptr<_Elm> &oth, const tensor_size<_Dim> &sizes)
	{
		return tensor_ptr<_Elm, _Dim>(oth.data(), sizes);
	}

	template<typename _Elm>
	inline array_ptr<_Elm> _reshape(const base_tensor_ptr<_Elm> &oth, const tensor_size<1> &sizes)
	{
		return oth.plan();
	}


	template<typename _Elm>
	inline auto _reshape(const base_tensor_ptr<_Elm> &oth, const tensor_size<2> &sizes)
	{
		using return_type = base_tensor_ptr<array_ptr<typename base_tensor_ptr<_Elm>::elm_type>>;

		if (oth.size() == sizes[0])return return_type(
			oth.front().plan(),
			oth.size(),
			oth.move_step());

		if (1 == oth.size())
		{
			return _reshape(oth.front(), sizes);
		}

		if (oth.size() % sizes[0])return return_type();

		return return_type(
			__ensmall(oth, sizes[0]).plan(),
			sizes[0],
			oth.move_step() * oth.size() / sizes[0]);
	}

	template<typename _Elm, size_t _Dim>
	inline auto _reshape(const base_tensor_ptr<_Elm> &oth, const tensor_size<_Dim> &sizes)
	{
		using return_type = tensor_ptr<typename base_tensor_ptr<_Elm>::elm_type, _Dim>;

		if (oth.size() == sizes[0])return return_type(
			_reshape(oth.front(), sub_size(sizes)),
			oth.size(),
			oth.move_step());

		if (1 == oth.size())
		{
			return _reshape(oth.front(), sizes);
		}

		if (oth.size() % sizes[0])return return_type();

		return return_type(
			_reshape(__ensmall(oth,sizes[0]), sub_size(sizes)),
			sizes[0],
			oth.move_step() * oth.size() / sizes[0]);
	}

	template<typename _T, size_t _Dim>
	inline auto reshape(_T &&_oth, const tensor_size<_Dim> &sizes)
	{
		auto oth = to_ptr(_oth);
		if (oth.total_size() != total_size(sizes))
			__on_bear_exception(
				exception_type::size_different,
				"can't reshape a memory to a diffrernce size!");

		auto ary = oth.plan();
		if (!ary.empty())return _reshape(ary, sizes);

		return _reshape(_shrink(oth), sizes);
	}

	template<typename _T, typename ... _Sz>
	inline auto reshape(_T &&oth, size_t sz, _Sz ... _sizes)
	{
		auto sizes = make_tensor_size(sz, _sizes ...);
		return reshape(oth, sizes);
	}



	template<typename _Stm, typename _Elm>
	inline _Stm && operator << (_Stm && stm, const base_tensor_ptr<_Elm> &arr)
	{
		std::forward<_Stm>(stm) << '{' << arr[0];
		auto e = arr.end();
		for (auto b = arr.begin() + 1; b < e; ++b)
		{
			std::forward<_Stm>(stm) << ',' << std::endl << *b;
		}
		std::forward<_Stm>(stm) << '}' << std::endl;
		return std::forward<_Stm>(stm);
	}
}
