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
