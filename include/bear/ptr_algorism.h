#pragma once
#include "ptr_types.h"
#include "bear_exception.h"
#include "ptr_traits.h"

namespace bear
{
	template<typename _T1>
	inline bool zip_check_size(
		size_t& size,
		const _T1& t1)
	{
		size = t1.size();
		return true;
	}

	template<typename _T1, typename _T2, typename ... _Ts>
	inline bool zip_check_size(
		size_t& size,
		const _T1& t1,
		const _T2& t2,
		const _Ts & ... ts)
	{
		auto s1 = t1.size();
		auto s2 = t2.size();
		if (s1 != s2)return false;
		return zip_check_size(size, t2, ts ...);
	}

	template<typename ... _Ts>
	inline bool zip_check_size(
		std::array<size_t, 1>& size,
		const _Ts & ... ts)
	{
		return zip_check_size(size[0], ts ...);
	}

	template<size_t _Lv, typename ... _Ts>
	inline bool zip_check_size(
		std::array<size_t, _Lv>& size,
		const _Ts & ... ts)
	{
		static_assert(_Lv >= 1, "dim can't be zero!");
		if (!zip_check_size(size[0], ts ...))return false;
		return zip_check_size(*(std::array<size_t, _Lv - 1> *) & size[1], *ts.begin() ...);
	}


	template<typename _T1>
	inline void _zip_increase(_T1& t1)
	{
		++t1;
	}

	template<typename _T1, typename ... _Ts>
	inline void _zip_increase(
		_T1& t1,
		_Ts & ... ts)
	{
		++t1;
		_zip_increase(ts ...);
	}



	template<typename _Fn, typename ... _Ts>
	inline void zip(
		_Fn&& fn,
		size_t size,
		_Ts ... ts)
	{

		for (size_t i = size; i; _zip_increase(ts ...), --i)
		{
			std::forward<_Fn>(fn)(*ts ...);
		}
	}


	template<typename _Fn, typename ... _Ts>
	inline void zip(
		_Fn&& fn,
		const std::array<size_t, 1>& size,
		_Ts ... ts)
	{

		for (size_t i = size[0]; i; _zip_increase(ts ...), --i)
		{
			std::forward<_Fn>(fn)(*ts ...);
		}
	}


	template<size_t _Lv, typename _Fn, typename ... _Ts>
	inline void zip(
		_Fn&& fn,
		const std::array<size_t, _Lv>& size,
		_Ts ... ts)
	{

		for (size_t i = size[0]; i; _zip_increase(ts ...), --i)
		{
			zip(
				std::forward<_Fn>(fn),
				*(const std::array<size_t, _Lv - 1> *) & size[1],
				ts->begin() ...);
		}
	}

	template<size_t _Lv, typename _Fn, typename ... _Ts>
	inline void zip_to(
		_Fn&& fn,
		_Ts && ... ts)
	{

		std::array<size_t, _Lv> sizes;

#ifdef _DEBUG
		assert(zip_check_size(sizes, ts ...));
#else
		zip_check_size(sizes, ts ...);
#endif

		zip(std::forward<_Fn>(fn), sizes, std::forward<_Ts>(ts).begin() ...);
	}



	template<typename _Fn, typename _Tr, typename ... _Ts>
	inline void zip_r(
		_Fn&& fn,
		size_t size,
		_Tr tr,
		_Ts ... ts)
	{

		for (size_t i = size; i; _zip_increase(tr, ts ...), --i)
		{
			*tr = std::forward<_Fn>(fn)(*ts ...);
		}
	}


	template<typename _Fn, typename _Tr, typename ... _Ts>
	inline void zip_r(
		_Fn&& fn,
		const std::array<size_t, 1>& size,
		_Tr tr,
		_Ts ... ts)
	{

		for (size_t i = size[0]; i; _zip_increase(tr, ts ...), --i)
		{
			*tr = std::forward<_Fn>(fn)(*ts ...);
		}
	}


	template<size_t _Lv, typename _Fn, typename ... _Ts>
	inline void zip_r(
		_Fn&& fn,
		const std::array<size_t, _Lv>& size,
		_Ts ... ts)
	{

		for (size_t i = size[0]; i; _zip_increase(ts ...), --i)
		{
			zip_r(
				std::forward<_Fn>(fn),
				*(const std::array<size_t, _Lv - 1> *) & size[1],
				ts->begin() ...);
		}
	}

	template<size_t _Lv, typename _Fn, typename ... _Ts>
	inline void zip_r_to(
		_Fn&& fn,
		_Ts && ... ts)
	{

		std::array<size_t, _Lv> sizes;

#ifdef _DEBUG
		assert(zip_check_size(sizes, ts ...));
#else
		zip_check_size(sizes, ts ...);
#endif

		zip_r(std::forward<_Fn>(fn), sizes, std::forward<_Ts>(ts).begin() ...);
	}


	template<typename _T1, typename ... _Ts>
	struct is_same_dim : public std::integral_constant<bool,
		ptr_traits<_T1>::dim == is_same_dim<_Ts...>::dim
	>
	{
		static constexpr size_t dim = ptr_traits<_T1>::dim;
	};

	template<typename _T1>
	struct is_same_dim<_T1> : public std::integral_constant<bool, true>
	{
		static constexpr size_t dim = ptr_traits<_T1>::dim;
	};


	template<typename _Re, size_t dim>
	struct __map_function
	{
		template<typename _Fun, typename _T1, typename ... _Ts>
		static auto run(_Fun&& fn, const _T1& t1, const _Ts &... ts)
		{
			using result_elm = typename std::decay<_Re>::type;

			using ctn_type = typename
				ptr_traits<
				typename ptr_change_elm<typename std::decay<decltype(t1)>::type, result_elm>::type
				>::container_type;

			ctn_type ret(size(t1));

			zip_r_to<dim>(std::forward<_Fun>(fn), ret, t1, ts ...);

			return ret;
		}
	};

	template<size_t dim>
	struct __map_function<void, dim>
	{
		template<typename _Fun, typename ... _Ts>
		static void run(_Fun&& fn, const _Ts &... ts)
		{
			zip_to<dim>(std::forward<_Fun>(fn), ts ...);
		}
	};


	template<typename _Fun, typename ... _Ts>
	inline auto map_function(_Fun&& fn, _Ts && ... ts)
	{
		using dim_test = is_same_dim<typename std::decay<decltype(to_ptr(ts))>::type ...>;

		static_assert(dim_test::value, "not same dim.");

		using result_elm_type = decltype(
			std::forward<_Fun>(fn)(
				ptr_traits<typename std::decay<decltype(to_ptr(ts))>::type>::get_elm() ...)
			);

		return __map_function<result_elm_type, dim_test::dim>::run(
			std::forward<_Fun>(fn), to_ptr(ts) ...);
	}






	template<typename _T1>
	inline bool _is_same_size(
		const _T1& t1)
	{
		return true;
	}

	template<typename _T1, typename _T2, typename ... _Ts>
	inline bool _is_same_size(
		const _T1& t1,
		const _T2& t2,
		const _Ts & ... ts)
	{
		if (t1.size() != t2.size())return false;
		return _is_same_size(t2, ts ...);
	}

	template<size_t _Dim>
	struct is_same_size_to
	{
		template<typename ... _Ts>
		static bool run(const _Ts & ... ts)
		{
			if (!_is_same_size(ts ...))return false;
			return is_same_size_to<_Dim - 1>::run(*(ts.begin()) ...);
		}
	};

	template<>
	struct is_same_size_to<0>
	{
		template<typename ... _Ts>
		static bool run(const _Ts & ... ts)
		{
			return true;
		}
	};

	template<typename ... _Ts>
	inline bool is_same_size(const _Ts & ... ts)
	{
		using dim_test = is_same_dim<typename std::decay<decltype(to_ptr(ts))>::type ...>;

		static_assert(dim_test::value, "not same dim.");

		return is_same_size_to<dim_test::dim>::run(ts ...);
	}






	template<typename _Tl, typename _Tr>
	inline int compare(_Tl&& p, _Tr&& q

		, typename type_exist<
		typename ptr_flag<typename std::decay<_Tl>::type>::not_both,
		typename ptr_flag<typename std::decay<_Tr>::type>::not_both
		>::type _ = std::true_type()

	)
	{
		return p > q ? 1 : (p < q ? -1 : 0);
	}


	template<typename _Tl, typename _Tr>
	inline int compare(_Tl&& p, _Tr&& q

		, typename type_exist<
		typename ptr_flag<typename std::decay<_Tl>::type>::is_one,
		typename ptr_flag<typename std::decay<_Tr>::type>::is_one
		>::type _ = std::true_type()

	)
	{
		size_t ms;
		int sf;

		if (p.size() > q.size())
		{
			ms = q.size();
			sf = 1;
		}
		else if (p.size() < q.size())
		{
			ms = p.size();
			sf = -1;
		}
		else
		{
			ms = p.size();
			sf = 0;
		}

		auto _p = p.begin();
		auto _q = q.begin();

		for (auto i = ms; i; --i)
		{
			int r = compare(*_p, *_q);
			if (r)return r;

			++_p;
			++_q;
		}

		return sf;
	}


	template<typename _T>
	inline auto clip(_T&& oth, size_t start, size_t end

		, typename ptr_flag<typename std::decay<_T>::type>::is_1d _ = std::true_type()

	)
	{
		return to_ptr(oth).clip(start, end);
	}

	template<typename T>
	inline void _____dummy(T)
	{
	}

	template<typename _T1, typename _T2>
	inline void copy(_T1&& dst, _T2&& src

		, typename type_exist<
		typename ptr_flag<typename std::decay<_T1>::type>::is_one,
		typename ptr_flag<typename std::decay<_T2>::type>::is_one
		>::type _ = std::true_type()

	)
	{
		_____dummy(_);
		using d_trait = ptr_traits<typename std::decay<decltype(to_ptr(dst))>::type>;
		using s_trait = ptr_traits<typename std::decay<decltype(to_ptr(src))>::type>;

		using d_elm = typename d_trait::elm_type;
		using s_elm = typename s_trait::elm_type;

		map_function([](d_elm& d, const s_elm& s) {d = s; }, dst, src);
	}


	template<typename _T, typename _Fun>
	inline void for_each(_T&& dst, _Fun&& fun

		, typename ptr_flag<typename std::decay<_T>::type>::is_one
		_ = std::true_type()
	)
	{
		map_function(std::forward<_Fun>(fun), dst);
	}


	template<typename _T, typename _V>
	inline void fill(_T&& dst, _V&& vl

		, typename ptr_flag<typename std::decay<_T>::type>::is_one
		_ = std::true_type()
	)
	{
		using d_trait = ptr_traits<typename std::decay<decltype(to_ptr(dst))>::type>;
		using d_elm = typename d_trait::elm_type;

		map_function([vl](d_elm& v) {v = vl; }, dst);
	}

	template<typename _T>
	inline auto sum(_T&& ctn)
	{
		typename std::decay<decltype(*(ctn.begin()) + *(ctn.begin()))>::type
			ret = 0;

		auto e = ctn.end();
		for (auto b = ctn.begin(); b < e; ++b)
		{
			ret += *b;
		}

		return ret;
	}


	template<size_t i>
	struct _tensor_get
	{
		template<typename _T>
		static auto _get_size(const _T& t)
		{
			return _tensor_get<i - 1>::_get_size(t.front());
		}


		template<typename _T>
		static auto _get_step(const _T& t)
		{
			return _tensor_get<i - 1>::_get_step(t.front());
		}
	};

	template<>
	struct _tensor_get<0>
	{
		template<typename _T>
		static auto _get_size(const _T& t)
		{
			return t.size();
		}

		template<typename _T>
		static auto _get_step(const _T& t)
		{
			return t.move_step();
		}
	};


	template<size_t _Dim, typename _T>
	size_t size_at(_T&& ts)
	{
		return _tensor_get<_Dim>::_get_size(ts);
	}

	template<size_t _Dim, typename _T>
	size_t move_step_at(_T&& ts)
	{
		return _tensor_get<_Dim>::_get_step(ts);
	}

	template<typename _Elm, typename _T>
	inline auto element_cast(const _T& ts)
	{
		using l_trait = ptr_traits<typename std::decay<decltype(to_ptr(ts))>::type>;

		using l_elm = typename l_trait::elm_type;

		return map_function([](l_elm v) { return static_cast<_Elm>(v); }, ts);
	}

}
