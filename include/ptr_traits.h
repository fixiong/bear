#pragma once

#include "ptr_types.h"

#include <tuple>

namespace bear
{
	template<typename _T>
	_T __get_t();

	template<typename _T>
	struct ____get_t
	{
		static typename std::decay<_T>::type &run();
	};

	template<typename ... _Ts>
	struct tuple_push_back {};

	template<typename ... _Ts, typename _T>
	struct tuple_push_back<std::tuple<_Ts ...>, _T>
	{
		using type = std::tuple<_Ts ..., _T>;
	};

	template<unsigned int _sz>
	struct make_type_index
	{
		using type = typename tuple_push_back<
			typename make_type_index<_sz - 1>::type,
			std::integral_constant<size_t,_sz - 1>>::type;
	};

	template<>
	struct make_type_index<1>
	{
		using type = std::integral_constant<size_t, 0> ;
	};

	template<typename _Fn, typename ... _T, typename ... _Idx>
	inline void __unpack_arg(
		_Fn && fn,
		const std::tuple<_T ...> &arg,
		std::tuple<_Idx ...>)
	{
		std::forward<_Fn>(fn)(*std::get<_Idx::value>(arg) ...);
	}


	template<typename _Fn, typename _Tp>
	inline void unpack_tuple_arg(
		_Fn && fn,
		const _Tp &arg)
	{
		__unpack_arg(std::forward<_Fn>(fn), arg, make_type_index<std::tuple_size<_Tp>::value>());
	}

	template<size_t I, typename _T1, typename ... _T>
	struct pack_get_type
	{
		using type = typename pack_get_type<I - 1, _T ...>::type;
	};


	template<typename _T1, typename ... _T>
	struct pack_get_type<0,_T1,_T ...>
	{
		using type = _T1;
	};


	template<typename _T1, typename ... _T>
	struct pack_get_size : public std::integral_constant<size_t, pack_get_size<_T ...>::value + 1> {};


	template<typename _T1>
	struct pack_get_size<_T1> :public std::integral_constant<size_t,1> {};


	template<typename _T1, size_t _Sz>
	void __pack_to_array(const std::array<_T1, _Sz> & ret, size_t p) {}

	template<typename _T1, size_t _Sz, typename ... _T>
	void __pack_to_array(const std::array<_T1,_Sz> & ret, size_t p, _T1 v1, _T ... vs)
	{
		ret[p] = v1;
		__pack_to_array(ret, p + 1, vs ...);
	}

	template<typename _T1, typename ... _T>
	auto pack_to_array(_T1 v1, _T ... vs)
	{
		std::array<_T1, pack_get_size<_T ...>::value + 1> ret;
		__pack_to_array(ret, 0, v1, vs ...);
	}


	template<typename _T, size_t _I>
	struct ptr_type_at
	{
		using type = typename ptr_type_at<
			typename std::decay<decltype(__get_t<_T>().front())>::type,
			_I - 1>::type;
	};


	template<typename _T>
	struct ptr_type_at<_T,0>
	{
		using type = _T;
	};

	template<typename _Ptr, typename _Elm>
	struct ptr_change_elm
	{
		using type = _Elm;
	};

	template<typename _Oe, size_t _Sz, typename _Elm>
	struct ptr_change_elm<std::array<_Oe,_Sz>,_Elm>
	{
		using type = std::array<
			typename ptr_change_elm<_Oe, _Elm>::type, _Sz>;
	};

	template<typename _Oe, size_t _Sz, typename _Elm>
	struct ptr_change_elm<const std::array<_Oe, _Sz>, _Elm>
	{
		using _type = std::array<typename ptr_change_elm<_Oe,
			typename std::remove_const<_Elm>::type>::type, _Sz>;

		using type = typename std::conditional<
			std::is_const<_Elm>::value,
			const _type,
			_type>::type;
	};

	template<typename _Elm, typename _Trt, typename _Nelm>
	struct ptr_change_elm<basic_string_ptr<_Elm, _Trt>, _Nelm>
	{
		using type = basic_string_ptr<typename ptr_change_elm<_Elm, _Nelm>::type, _Trt>;
	};

	template<typename _Oe, typename _Elm>
	struct ptr_change_elm<array_ptr<_Oe>, _Elm>
	{
		using type = array_ptr<typename ptr_change_elm<_Oe, _Elm>::type>;
	};

	template<typename _Bs, typename _Elm>
	struct ptr_change_elm<base_tensor_ptr<_Bs>, _Elm>
	{
		using type = base_tensor_ptr<typename ptr_change_elm<_Bs, _Elm>::type>;
	};


	template<typename _Oe, size_t _Ch, typename _Elm>
	struct ptr_change_elm<image_ptr<_Oe,_Ch>, _Elm>
	{
		using type = image_ptr<typename ptr_change_elm<_Oe, _Elm>::type,_Ch>;
	};



	template<typename _Ptr>
	struct ptr_get_elm
	{
		using type = _Ptr;
	};

	template<typename _Oe, size_t _Sz>
	struct ptr_get_elm<std::array<_Oe, _Sz>>
	{
		using type = typename ptr_get_elm<_Oe>::type;
	};

	template<typename _Oe>
	struct ptr_get_elm<array_ptr<_Oe>>
	{
		using type = typename ptr_get_elm<_Oe>::type;
	};

	template<typename _Bs>
	struct ptr_get_elm<base_tensor_ptr<_Bs>>
	{
		using type = typename ptr_get_elm<_Bs>::type;
	};


	template<typename _Oe, size_t _Ch>
	struct ptr_get_elm<image_ptr<_Oe, _Ch>>
	{
		using type = typename ptr_get_elm<_Oe>::type;
	};

	template<typename _Ptr>
	auto &to_base_ptr(const _Ptr &p)
	{
		return p;
	}

	template<typename _Elm, size_t _Ch>
	const typename image_ptr<_Elm, _Ch>::tensor_type &
		to_base_ptr(const image_ptr<_Elm,_Ch> &p)
	{
		return p;
	}

	template<typename _Elm, typename _Trt>
	const array_ptr<_Elm> &to_base_ptr(const basic_string_ptr<_Elm, _Trt> &p)
	{
		return p;
	}

	template<typename _Ptr>
	struct ptr_traits
	{
		using type = _Ptr;
		using sub_type = _Ptr;
		using elm_type = _Ptr;
		static constexpr size_t dim = 0;
		static elm_type &get_elm();
		using not_ptr = std::true_type;
	};


	template<typename _Ptr>
	struct _ptr_traits
	{
		using elm_type = _Ptr;
		static constexpr size_t dim = 0;
	};

	template<typename _Elm, size_t _Sz>
	struct _ptr_traits<std::array<_Elm, _Sz>>
	{
		using elm_type = typename _ptr_traits<_Elm>::elm_type;
		static constexpr size_t dim = _ptr_traits<_Elm>::dim + 1;
	};

	template<typename _Elm, size_t _Sz>
	struct _ptr_traits<const std::array<_Elm, _Sz>>
	{
		using elm_type = typename _ptr_traits<_Elm>::elm_type;
		static constexpr size_t dim = _ptr_traits<_Elm>::dim + 1;
	};

	template<typename _Elm, size_t _Sz>
	struct ptr_traits<std::array<_Elm, _Sz>>
	{
		using type = std::array<_Elm, _Sz>;
		using sub_type = _Elm;
		using elm_type = typename _ptr_traits<_Elm>::elm_type;
		static constexpr size_t dim = _ptr_traits<_Elm>::dim + 1;
		static elm_type &get_elm();
		using container_type = std::array<_Elm, _Sz>;
		using is_ptr = std::true_type;
	};

	template<typename _Elm, size_t _Sz>
	struct ptr_traits<const std::array<_Elm, _Sz>>
	{
		using type = std::array<_Elm, _Sz>;
		using sub_type = _Elm;
		using elm_type = const typename _ptr_traits<_Elm>::elm_type;
		static constexpr size_t dim = _ptr_traits<_Elm>::dim + 1;
		static elm_type &get_elm();
		using container_type = std::array<_Elm, _Sz>;
		using is_ptr = std::true_type;
	};

	template<typename _Elm>
	struct ptr_traits<array_ptr<_Elm>>
	{
		using type = array_ptr<_Elm>;
		using sub_type = _Elm;
		using elm_type = typename _ptr_traits<_Elm>::elm_type;
		static constexpr size_t dim = _ptr_traits<_Elm>::dim + 1;
		static elm_type &get_elm();
		using container_type = std::vector<_Elm>;
		using is_ptr = std::true_type;
	};

	template<typename _Elm, typename _Trt>
	struct ptr_traits<basic_string_ptr<_Elm, _Trt>>
	{
		using type = basic_string_ptr<_Elm, _Trt>;
		using sub_type = _Elm;
		using elm_type = typename _ptr_traits<_Elm>::elm_type;
		static constexpr size_t dim = _ptr_traits<_Elm>::dim + 1;
		static elm_type &get_elm();
		using container_type = std::basic_string<_Elm, _Trt>;
		using is_ptr = std::true_type;
	};

	template<typename _Elm>
	struct ptr_traits<base_tensor_ptr<_Elm>>
	{
		using type = base_tensor_ptr<_Elm>;
		using sub_type = _Elm;
		using elm_type = typename ptr_traits<_Elm>::elm_type;
		static constexpr size_t dim = ptr_traits<_Elm>::dim + 1;
		static elm_type &get_elm();
		using container_type = tensor<typename base_tensor_ptr<_Elm>::elm_type, base_tensor_ptr<_Elm>::dim>;
		using is_ptr = std::true_type;
	};

	template<typename _Elm, size_t _Ch>
	struct ptr_traits<image_ptr<_Elm ,_Ch>>
	{
		using type = image_ptr<_Elm, _Ch>;
		using sub_type = _Elm;
		using image_elm_type = typename image_ptr<_Elm, _Ch>::elm_type;
		using elm_type = typename _ptr_traits<_Elm>::elm_type;
		static constexpr size_t dim = _ptr_traits<image_elm_type>::dim + 2;
		static elm_type &get_elm();
		using container_type = image<_Elm, _Ch>;
		using is_ptr = std::true_type;
	};

	template<typename _Cnt>
	struct container_traits : 
		public ptr_traits<typename std::decay<decltype(to_ptr(__get_t<_Cnt>()))>::type>
	{
	};

	template<typename _T1, typename ... _Ts>
	struct type_exist
	{
		using type = typename type_exist<_Ts ...>::type;
	};


	template<typename _T1>
	struct type_exist<_T1>
	{
		using type = std::true_type;
	};

	template<typename _T>
	struct _is_true_type {};

	template<>
	struct _is_true_type<std::true_type>
	{
		using type = std::true_type;
	};



	template<typename _Ptr>
	struct ptr_flag
	{
		using not_ptr = std::true_type;
		using not_container = std::true_type;
		using not_both = std::true_type;
	};

	template<typename _Elm>
	struct ptr_flag<array_ptr<_Elm>>
	{
		using is_ptr = std::true_type;
		using not_container = std::true_type;
		using is_one = std::true_type;
		using is_1d = std::true_type;
	};

	template<typename _Elm, typename _Trt>
	struct ptr_flag<basic_string_ptr<_Elm, _Trt>>
	{
		using is_ptr = std::true_type;
		using not_container = std::true_type;
		using is_one = std::true_type;
		using is_1d = std::true_type;
	};

	template<typename _Elm>
	struct ptr_flag<base_tensor_ptr<_Elm>>
	{
		using is_ptr = std::true_type;
		using not_container = std::true_type;
		using is_one = std::true_type;
	};

	template<typename _Elm, size_t _Ch>
	struct ptr_flag<image_ptr<_Elm, _Ch>>
	{
		using is_ptr = std::true_type;
		using not_container = std::true_type;
		using is_one = std::true_type;
	};

	template<typename _Elm, size_t _Sz>
	struct ptr_flag<std::array<_Elm, _Sz>>
	{
		using not_ptr = std::true_type;
		using is_container = std::true_type;
		using is_one = std::true_type;
		using is_1d = std::true_type;
	};

	template<typename _Elm, typename _Al>
	struct ptr_flag<std::vector<_Elm, _Al>>
	{
		using not_ptr = std::true_type;
		using is_container = std::true_type;
		using is_one = std::true_type;
		using is_1d = std::true_type;
	};

	template<typename _Elm, typename _Al>
	struct ptr_flag<std::basic_string<_Elm, _Al>>
	{
		using not_ptr = std::true_type;
		using is_container = std::true_type;
		using is_one = std::true_type;
		using is_1d = std::true_type;
	};


	template<typename _Elm, size_t _Ch>
	struct ptr_flag<image<_Elm, _Ch>>
	{
		using not_ptr = std::true_type;
		using is_container = std::true_type;
		using is_one = std::true_type;
	};

	template<typename _Elm, size_t _Sz>
	struct ptr_flag<tensor<_Elm, _Sz>>
	{
		using not_ptr = std::true_type;
		using is_container = std::true_type;
		using is_one = std::true_type;
	};

}