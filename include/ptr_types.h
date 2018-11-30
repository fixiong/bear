#pragma once


#include<type_traits>
#include<array>
#include<vector>
#include<string>


namespace bear
{
	using std::size_t;
	using std::ptrdiff_t;


	class bear_exception;

	namespace exception_type
	{
		enum type
		{
			pointer_outof_range = 1,
			size_different,
			memory_not_continuous,
			other_error,
			bear_exception_end,
		};

	}

	inline void __on_bear_exception(unsigned int, const char * e);

	template<typename _Elm>
	class array_ptr;

	template<typename _Elm>
	inline array_ptr<_Elm> & to_ptr(array_ptr<_Elm> & p)
	{
		return p;
	}

	template<typename _Elm>
	inline const array_ptr<_Elm> & to_ptr(const array_ptr<_Elm> & p)
	{
		return p;
	}

	template<typename _Elm>
	inline size_t size(const array_ptr<_Elm> & arr);

	template<typename _Elm>
	using const_array_ptr = array_ptr<const _Elm>;



	template<typename _Elm>
	inline std::vector<typename std::decay<_Elm>::type> make_container(const array_ptr<_Elm> &arr);

	template<typename _Elm, typename _Alc>
	inline size_t size(const std::vector<_Elm, _Alc> & _ctn)
	{
		return _ctn.size();
	}

	template<typename _Elm, size_t _Size>
	inline size_t size(const std::array<_Elm, _Size> & _ctn)
	{
		return _ctn.size();
	}


	template<typename _Elm, typename _Alc>
	inline array_ptr<_Elm> to_ptr(std::vector<_Elm, _Alc> & _ctn);

	template<typename _Elm, typename _Alc>
	inline const_array_ptr<_Elm> to_ptr(const std::vector<_Elm, _Alc> & _ctn);

	template<typename _Elm, size_t Size>
	inline array_ptr<_Elm> to_ptr(std::array<_Elm, Size> & _ctn);

	template<typename _Elm, size_t Size>
	inline const_array_ptr<_Elm> to_ptr(const std::array<_Elm, Size> &_ctn);




	template<typename _Elm, typename _Trt = std::char_traits<typename std::decay<_Elm>::type>>
	class basic_string_ptr;

	template<typename _Elm, typename _Trt>
	inline basic_string_ptr<_Elm, _Trt> & to_ptr(basic_string_ptr<_Elm, _Trt> & p)
	{
		return p;
	}

	template<typename _Elm, typename _Trt>
	inline const basic_string_ptr<_Elm, _Trt> & to_ptr(const basic_string_ptr<_Elm, _Trt> & p)
	{
		return p;
	}

	template<typename _Elm, typename _Trt>
	inline size_t size(const basic_string_ptr<_Elm, _Trt> & str);

	template<typename _Elm, typename _Trt>
	using const_basic_string_ptr = basic_string_ptr<const _Elm, _Trt>;

	template<typename _Elm>
	inline basic_string_ptr<_Elm> to_ptr(_Elm * oth);

	template<typename _Elm, typename _Trt>
	inline basic_string_ptr<_Elm, _Trt> to_ptr(std::basic_string<_Elm, _Trt> &oth);

	template<typename _Elm, typename _Trt>
	inline const_basic_string_ptr<_Elm, _Trt> to_ptr(const std::basic_string<_Elm, _Trt> &oth);

	template<typename _Elm, typename _Trt>
	inline size_t size(const std::basic_string<_Elm, _Trt> &oth)
	{
		return oth.size();
	}

	template<typename _Elm, typename _Trt>
	inline std::basic_string<typename std::decay<_Elm>::type, _Trt>
		make_container(const basic_string_ptr<_Elm, _Trt> &str);




	template<typename _Elm, typename _Trt = std::char_traits<typename std::decay<_Elm>::type>>
	using const_basic_string_ptr = basic_string_ptr<const _Elm, _Trt>;

	using string_ptr = basic_string_ptr<char>;
	using wstring_ptr = basic_string_ptr<wchar_t>;
	using u16string_ptr = basic_string_ptr<char16_t>;
	using u32string_ptr = basic_string_ptr<char32_t>;


	using const_string_ptr = basic_string_ptr<const char>;
	using const_wstring_ptr = basic_string_ptr<const wchar_t>;
	using const_u16string_ptr = basic_string_ptr<const char16_t>;
	using const_u32string_ptr = basic_string_ptr<const char32_t>;

#define literal_u8(s) bear::const_string_ptr(u8 ## s, sizeof(u8 ## s)-1)
#define literal_u16(s) bear::const_wstring_ptr(u ## s, sizeof(u ## s)-1)
#define literal_u32(s) bear::const_u32string_ptr(U ## s, sizeof(U ## s)-1)


	template<size_t _Sz>
	using tensor_size = std::array<size_t,_Sz>;

	template<size_t _Sz>
	inline tensor_size<_Sz - 1> &sub_size(tensor_size<_Sz> &sz);

	template<size_t _Sz>
	inline const tensor_size<_Sz - 1> &sub_size(const tensor_size<_Sz> &sz);

	template<size_t _Sz>
	inline size_t total_size(const tensor_size<_Sz> &sz);

	template<size_t _Sz>
	inline bool operator == (const tensor_size<_Sz> &ls, const tensor_size<_Sz> &rs);

	template<size_t _Sz>
	inline bool operator != (const tensor_size<_Sz> &ls, const tensor_size<_Sz> &rs);

	template<typename _Base>
	class base_tensor_ptr;

	template<typename _Base>
	inline const base_tensor_ptr<_Base> & to_ptr(const base_tensor_ptr<_Base> & p)
	{
		return p;
	}

	template<typename _Base>
	inline base_tensor_ptr<_Base> & to_ptr(base_tensor_ptr<_Base> & p)
	{
		return p;
	}

	template<typename _Base>
	inline tensor_size<base_tensor_ptr<_Base>::dim> size(const base_tensor_ptr<_Base> & t);

	template<typename _Elm, size_t _Dim>
	struct _tensor_traits
	{
		using type = base_tensor_ptr<
			typename _tensor_traits<_Elm, _Dim - 1>::type>;
	};

	template<typename _Elm>
	struct _tensor_traits<_Elm, 1>
	{
		using type = array_ptr<_Elm>;
	};

	template<typename _Elm>
	struct _tensor_traits<_Elm, 0>
	{
		using type = _Elm;
	};

	template<typename _Elm, size_t _Dim>
	using tensor_ptr = typename _tensor_traits<_Elm, _Dim>::type;

	template<typename _Elm, size_t _Dim>
	using const_tensor_ptr = typename _tensor_traits<const _Elm, _Dim>::type;



	class image_size;

	template<typename _Elm, size_t _Ch>
	class image_ptr;

	template<typename _Elm, size_t _Ch>
	inline const image_ptr<_Elm, _Ch> & to_ptr(const image_ptr<_Elm, _Ch> & p)
	{
		return p;
	}

	template<typename _Elm, size_t _Ch>
	inline image_ptr<_Elm, _Ch> & to_ptr(image_ptr<_Elm, _Ch> & p)
	{
		return p;
	}

	template<typename _Elm, size_t _Ch>
	inline image_size size(const image_ptr<_Elm, _Ch> & img);

	template<typename _Elm, size_t _Ch>
	using const_image_ptr = image_ptr<const _Elm, _Ch>;





	template<typename _Elm, size_t _Dim, typename Alloc = std::allocator<_Elm>>
	class tensor;

	template<typename _Elm, size_t _Dim, size_t _Aln>
	class aligned_tensor;

	template<typename _Elm, size_t _Dim, typename Alloc>
	inline tensor_size<_Dim> size(const tensor<_Elm, _Dim, Alloc> & ts);

	template<typename _Elm, typename _Base, typename Alloc>
	inline tensor<
		typename base_tensor_ptr<_Base>::elm_type,
		base_tensor_ptr<_Base>::dim,
		Alloc> bind_container(std::vector<_Elm, Alloc> &&ctn, const base_tensor_ptr<_Base> &ptr);

	template<typename _Base>
	inline tensor<
		typename std::decay<typename base_tensor_ptr<_Base>::elm_type>::type,
		base_tensor_ptr<_Base>::dim, std::allocator<
		typename std::decay<typename base_tensor_ptr<_Base>::elm_type>::type>
	> make_container(const base_tensor_ptr<_Base> & oth);


	template<typename _Stm, typename _Elm>
	inline _Stm && operator << (_Stm && stm, const base_tensor_ptr<_Elm> &arr);

	template<typename _Stm, typename _Elm>
	inline _Stm && operator << (_Stm && stm, array_ptr<_Elm> arr);

	template<typename _Stm, typename _Elm, typename _Trt>
	inline _Stm && operator << (_Stm && stm, basic_string_ptr<_Elm, _Trt> arr);

	template<typename _Elm, size_t _Dim, typename Alloc>
	inline tensor_ptr<_Elm, _Dim> &to_ptr(tensor<_Elm, _Dim, Alloc> &img);

	template<typename _Elm, size_t _Dim, typename Alloc>
	inline const_tensor_ptr<_Elm, _Dim> &to_ptr(const tensor<_Elm, _Dim, Alloc> &img);


	template<typename _Elm, size_t _Ch, class Alloc = std::allocator<_Elm> >
	class image;

	template<typename _Elm, size_t _Ch, size_t _Aln >
	class aligned_image;

	template<typename _Elm, size_t _Ch, typename Alloc>
	inline image_size size(const image<_Elm, _Ch, Alloc> & ts);

	template<typename _Elm, size_t _Ch, typename Alloc = std::allocator<_Elm>>
	inline image<_Elm, _Ch, Alloc> bind_container(std::vector<_Elm, Alloc> &&ctn, const image_ptr<_Elm, _Ch> &ptr);

	template<typename _Elm, size_t _Ch>
	inline image<typename std::decay<_Elm>::type, _Ch,
		std::allocator<typename std::decay<_Elm>::type>>
		make_container(const image_ptr<_Elm, _Ch> &ptr);

	template<typename _Elm, size_t _Dim, typename Alloc>
	inline image_ptr<_Elm, _Dim> &to_ptr(image<_Elm, _Dim, Alloc> &img);

	template<typename _Elm, size_t _Dim, typename Alloc>
	inline const_image_ptr<_Elm, _Dim> &to_ptr(const image<_Elm, _Dim, Alloc> &img);

	class char_bool;

	template<typename _Tl, typename _Tr>
	class is_memory_compatible : public
		std::integral_constant<bool,
		//std::is_assignable<
		//typename std::decay<_Tl>::type *,
		//typename std::decay<_Tr>::type *>::value &&
		sizeof(_Tl) == sizeof(_Tr)> {};

}