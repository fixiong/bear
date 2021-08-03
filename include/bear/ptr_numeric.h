#pragma once

#include "ptr_algorism.h"
#include "tensor.h"
#include "image.h"
#include "ptr_traits.h"

namespace bear
{
	template<typename _Bl, typename _Br>
	inline auto add(_Bl&& ls, _Br&& rs
		, typename type_exist<decltype(to_ptr(ls)), decltype(to_ptr(rs))>::type = std::true_type()
	)
	{
		using l_trait = ptr_traits<typename std::decay<decltype(to_ptr(ls))>::type>;
		using r_trait = ptr_traits<typename std::decay<decltype(to_ptr(rs))>::type>;

		using l_elm = typename l_trait::elm_type;
		using r_elm = typename r_trait::elm_type;

		return map_function([](l_elm l, r_elm r) {return l + r; }, ls, rs);
	}

	template<typename _Bl, typename _Br>
	inline auto add(_Bl&& ls, _Br&& rs
		, typename type_exist<decltype(to_ptr(ls)),
		typename ptr_flag<typename std::decay<decltype(rs)>::type>::not_both>
		::type = std::true_type()
	)
	{
		using l_trait = ptr_traits<typename std::decay<decltype(to_ptr(ls))>::type>;
		using l_elm = typename l_trait::elm_type;

		return map_function([rs](l_elm l) {return l + rs; }, ls);
	}

	template<typename _Bl, typename _Br>
	inline auto add(_Bl&& ls, _Br&& rs
		, typename type_exist<decltype(to_ptr(rs)),
		typename ptr_flag<typename std::decay<decltype(ls)>::type>::not_both>
		::type = std::true_type()
	)
	{
		using r_trait = ptr_traits<typename std::decay<decltype(to_ptr(rs))>::type>;
		using r_elm = typename r_trait::elm_type;

		return map_function([ls](r_elm r) {return r + ls; }, rs);
	}


	template<typename _Bl, typename _Br>
	inline decltype(add(____get_t<_Bl>::run(), ____get_t<_Br>::run())) operator + (_Bl&& ls, _Br&& rs)
	{
		return add(ls, rs);
	}



	template<typename _Bl, typename _Br>
	inline _Bl& add_inplace(_Bl&& ls, _Br&& rs
		, typename type_exist<decltype(to_ptr(ls)), decltype(to_ptr(rs))>::type = std::true_type()
	)
	{
		using l_trait = ptr_traits<typename std::decay<decltype(to_ptr(ls))>::type>;
		using r_trait = ptr_traits<typename std::decay<decltype(to_ptr(rs))>::type>;

		using l_elm = typename l_trait::elm_type;
		using r_elm = typename r_trait::elm_type;

		map_function([](l_elm& l, r_elm r) {l += r; }, ls, rs);

		return ls;
	}



	template<typename _Bl, typename _Br>
	inline _Bl& add_inplace(_Bl&& ls, _Br&& rs
		, typename type_exist<decltype(to_ptr(ls)),
		typename ptr_flag<typename std::decay<decltype(rs)>::type>::not_both>
		::type = std::true_type()
	)
	{
		using l_trait = ptr_traits<typename std::decay<decltype(to_ptr(ls))>::type>;
		using l_elm = typename l_trait::elm_type;

		map_function([rs](l_elm& l) {l += rs; }, ls);

		return ls;
	}

	template<typename _Bl, typename _Br>
	inline decltype(add_inplace(____get_t<_Bl>::run(), ____get_t<_Br>::run())) operator += (_Bl&& ls, _Br&& rs)
	{
		return add_inplace(ls, rs);
	}

	template<size_t dim>
	struct __dot {};


	template<>
	struct __dot<1>
	{
		template<typename _Bl, typename _Br>
		static auto run(_Bl&& ls, _Br&& rs)
		{
			using l_trait = ptr_traits<typename std::decay<decltype(to_ptr(ls))>::type>;
			using r_trait = ptr_traits<typename std::decay<decltype(to_ptr(rs))>::type>;

			using elm_type = typename std::decay<
				decltype(l_trait::get_elm()* r_trait::get_elm() + l_trait::get_elm() * r_trait::get_elm())>::type;

			using l_elm = typename l_trait::elm_type;
			using r_elm = typename r_trait::elm_type;

			if (ls.size() != rs.size())
				throw bear_exception(exception_type::size_different, "vector size different!");

			elm_type ret(ls[0] * rs[0]);

			zip_to<1>([&ret](l_elm ls, r_elm rs)
				{
					ret += ls * rs;
				}, clip(ls, 1, ls.size()), clip(rs, 1, rs.size()));

			return ret;

		}
	};

	template<>
	struct __dot<2>
	{
		template<typename _Bl, typename _Br>
		static auto run(_Bl&& ls, _Br&& rs)
		{
			using l_trait = ptr_traits<typename std::decay<decltype(to_ptr(ls))>::type>;
			using r_trait = ptr_traits<typename std::decay<decltype(to_ptr(rs))>::type>;

			using elm_type = typename std::decay<
				decltype(l_trait::get_elm()* r_trait::get_elm() + l_trait::get_elm() * r_trait::get_elm())>::type;

			using l_elm = typename l_trait::elm_type;
			using r_elm = typename r_trait::elm_type;

			if (ls.begin()->size() != rs.size())
				throw bear_exception(exception_type::size_different, "matrix size not compatible!");

			tensor<elm_type, 2> ret(ls.size(), rs.begin()->size());


			auto ls1 = ls.begin();
			auto end1 = ret.end();
			for (auto i1 = ret.begin(); i1 < end1; ++i1, ++ls1)
			{
				int c2 = 0;
				auto end2 = i1->end();
				for (auto i2 = i1->begin(); i2 < end2; ++i2, ++c2)
				{
					auto rs1 = rs.begin();
					auto ls_end2 = ls1->end();
					auto ls2 = ls1->begin();

					*i2 = *ls2 * (*rs1)[c2];
					++ls2, ++rs1;

					for (; ls2 < ls_end2; ++ls2, ++rs1)
					{
						*i2 += *ls2 * (*rs1)[c2];
					}
				}
			}

			return ret;

		}
	};

	template<typename _Bl, typename _Br>
	inline auto dot(_Bl&& ls, _Br&& rs
		, typename type_exist<decltype(to_ptr(ls)), decltype(to_ptr(rs))>::type = std::true_type()
	)
	{
		using l_trait = ptr_traits<typename std::decay<decltype(to_ptr(ls))>::type>;
		using r_trait = ptr_traits<typename std::decay<decltype(to_ptr(rs))>::type>;

		static_assert(l_trait::dim == r_trait::dim, "aggument should be the same dimention!");

		return __dot<l_trait::dim>::run(std::forward<_Bl>(ls), std::forward<_Br>(rs));
	}

}

#define __CNT_INNER(a,b) a ## b

#define __CNT(a,b) __CNT_INNER(a,b)

#define _NAME sub
#define _OP -

#include "ptr_numeric_macro.h"

#undef _NAME
#undef _OP


#define _NAME mul
#define _OP *

#include "ptr_numeric_macro.h"

#undef _NAME
#undef _OP


#define _NAME div
#define _OP /

#include "ptr_numeric_macro.h"

#undef _NAME
#undef _OP


#define _NAME bit_or
#define _OP |

#include "ptr_numeric_macro.h"

#undef _NAME
#undef _OP


#define _NAME bit_and
#define _OP &

#include "ptr_numeric_macro.h"

#undef _NAME
#undef _OP


#define _NAME bit_not
#define _OP ^

#include "ptr_numeric_macro.h"

#undef _NAME
#undef _OP


#define _NAME bit_right_sft
#define _OP >>

#include "ptr_numeric_macro.h"

#undef _NAME
#undef _OP


#define _NAME bit_left_sft
#define _OP <<

#include "ptr_numeric_macro.h"

#undef _NAME
#undef _OP
