
namespace bear
{
	template<typename _Bl, typename _Br>
	inline auto _NAME(_Bl && ls, _Br && rs
		, typename type_exist<decltype(to_ptr(ls)), decltype(to_ptr(rs))>::type = std::true_type()
	)
	{
		using l_trait = ptr_traits<typename std::decay<decltype(to_ptr(ls))>::type>;
		using r_trait = ptr_traits<typename std::decay<decltype(to_ptr(rs))>::type>;

		using l_elm = typename l_trait::elm_type;
		using r_elm = typename r_trait::elm_type;

		return map_function([](l_elm l, r_elm r) {return l _OP r; }, ls, rs);
	}

	template<typename _Bl, typename _Br>
	inline auto _NAME(_Bl && ls, _Br && rs
		, typename type_exist<decltype(to_ptr(ls)),
		typename ptr_flag<typename std::decay<decltype(rs)>::type>::not_both>
		::type = std::true_type()
	)
	{
		using l_trait = ptr_traits<typename std::decay<decltype(to_ptr(ls))>::type>;
		using l_elm = typename l_trait::elm_type;

		return map_function([rs](l_elm l) {return l _OP rs; }, ls);
	}

	template<typename _Bl, typename _Br>
	inline auto _NAME(_Bl && ls, _Br && rs
		, typename type_exist<decltype(to_ptr(rs)),
		typename ptr_flag<typename std::decay<decltype(ls)>::type>::not_both>
		::type = std::true_type()
	)
	{
		using r_trait = ptr_traits<typename std::decay<decltype(to_ptr(rs))>::type>;
		using r_elm = typename r_trait::elm_type;

		return map_function([ls](r_elm r) {return r _OP ls; }, rs);
	}


	template<typename _Bl, typename _Br>
	inline decltype(_NAME(____get_t<_Bl>::run(), ____get_t<_Br>::run())) operator _OP (_Bl && ls, _Br && rs)
	{
		return _NAME(ls, rs);
	}



	template<typename _Bl, typename _Br>
	inline _Bl & __CNT(_NAME, _inplace)(_Bl && ls, _Br && rs
		, typename type_exist<decltype(to_ptr(ls)), decltype(to_ptr(rs))>::type = std::true_type()
	)
	{
		using l_trait = ptr_traits<typename std::decay<decltype(to_ptr(ls))>::type>;
		using r_trait = ptr_traits<typename std::decay<decltype(to_ptr(rs))>::type>;

		using l_elm = typename l_trait::elm_type;
		using r_elm = typename r_trait::elm_type;

		map_function([](l_elm &l, r_elm r) {l __CNT(_OP, =) r; }, ls, rs);

		return ls;
	}

	template<typename _Bl, typename _Br>
	inline _Bl & __CNT(_NAME, _inplace)(_Bl && ls, _Br && rs
		, typename type_exist<decltype(to_ptr(ls)),
		typename ptr_flag<typename std::decay<decltype(rs)>::type>::not_both>
		::type = std::true_type()
	)
	{
		using l_trait = ptr_traits<typename std::decay<decltype(to_ptr(ls))>::type>;
		using l_elm = typename l_trait::elm_type;

		map_function([rs](l_elm &l) {l __CNT(_OP, =) rs; }, ls);

		return ls;
	}

	template<typename _Bl, typename _Br>
	inline decltype(__CNT(_NAME, _inplace)(____get_t<_Bl>::run(), ____get_t<_Br>::run())) operator __CNT(_OP, =) (_Bl && ls, _Br && rs)
	{
		return __CNT(_NAME, _inplace)(ls, rs);
	}

}