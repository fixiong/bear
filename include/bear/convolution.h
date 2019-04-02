#ifndef _CONVOLUTION_H
#define _CONVOLUTION_H

#include "tensor.h"
#include "image.h"
#include "ptr_traits.h"

namespace bear
{

	template<size_t _Ks>
	struct __ak_dsp_fnl
	{
		template<typename _Kt, size_t _Ks, typename _Th, typename ... _T>
		static auto run(const std::array<_Kt, _Ks> &kn, _Th & ah, _T & ... arg)
		{
			return __ak_dsp_fnl<_Ks - 1>(kn, arg ...) + kn[_Ks] * ah;
		}
	};

	template<0>
	struct __ak_dsp_fnl
	{
		template<typename _Kt, size_t _Ks, typename _T>
		static auto run(const std::array<_Kt, _Ks> &kn, _T & arg)
		{
			return kn[0] * arg;
		}

	};

	template<typename _Tt>
	struct __apl_kernel
	{
		template<typename _Kt, size_t _Ks, typename _Rt, typename ... _T>
		static void run(const std::array<_Kt, _Ks> &kn, _Rt &ret, _T & ... arg)
		{
			ret = __ak_dsp_fnl<_Ks - 1>::run(kn, arg ...);
		}
	};


	template<typename _Tt, size_t _Sz>
	struct __apl_kernel<std::array<_Tt,_Sz>>
	{
		template<typename _Kt, size_t _Ks, typename _Rt, typename _T1, typename ... _T>
		static void run(const std::array<_Kt, _Ks> &kn, _Rt &ret, _T1 & t1,_T & ... arg)
		{
			for (int i = 0; i < _Sz; ++i)
			{
				__apl_kernel<typename std::decay<decltype(*t1)>::type>::run(kn, ret[i], t1[i], arg[i] ...);
			}
		}
	};

	template<typename _Tt>
	struct __apl_kernel<array_ptr<_Tt>>
	{
		template<typename _Kt, size_t _Ks, typename _Rt, typename _T1, typename ... _T>
		static void run(const std::array<_Kt, _Ks> &kn, _Rt &ret, _T1 & t1, _T & ... arg)
		{
			for (int i = 0; i < ret.size(); ++i)
			{
				__apl_kernel<typename std::decay<decltype(*t1)>::type>::run(kn, ret[i], t1[i], arg[i] ...);
			}
		}
	};

	template<typename _Base>
	struct __apl_kernel<base_tensor_ptr<_Base>>
	{
		template<typename _Kt, size_t _Ks, typename _Rt, typename _T1, typename ... _T>
		static void run(const std::array<_Kt, _Ks> &kn, _Rt &ret, _T1 & t1, _T & ... arg)
		{
			for (int i = 0; i < ret.size(); ++i)
			{
				__apl_kernel<typename std::decay<decltype(*t1)>::type>::run(kn, ret[i], t1[i], arg[i] ...);
			}
		}
	};


	template<
		size_t _Dim,
		size_t _Str>
		struct __conv_1d
	{
		template<
			typename _Dst,
			typename _Src,
			typename _Knl,
			typename _Saver
		>
			static void run(
				const _Dst &dst,
				const _Src &src,
				const _Knl &kn,
				ptrdiff_t ofs,
				_Saver && sv)
		{
			auto de = dst.end();
			auto di = dst.begin();
			auto si = src.begin();
			for (; di < de; ++di, ++si)
			{
				__conv_1d<_Dim - 1, _Str>::
					run(*di, *si, kn, ofs, std::forward<_Saver>(sv));
			}
		}
	};


	template<typename _Fn, typename _Kt, typename _Rt, typename _Iter, typename ... _Idx>
	inline void _unpack_iterator(
		_Fn && fn,
		const _Kt &kn,
		_Rt &ret,
		const _Iter &iter,
		std::tuple<_Idx ...>)
	{
		std::forward<_Fn>(fn)(kn, ret, iter + _Idx::value ...);
	}

	template<size_t _Str>
	struct __conv_1d<0,_Str>
	{
		template<
			typename _Dst,
			typename _Src,
			typename _Kelm,
			size_t _Ksz,
			typename _Saver>
		static void run(
			const _Dst &dst,
			const _Src &src,
			const std::array<_Kelm, _Ksz> &kn,
			ptrdiff_t ofs,
			_Saver && sv)
		{
			using src_type = typename std::decay<decltype(src[0])>::type;
			using tmp_type = typename __get_tmp_type<src_type, _Kelm>::type;
			using tmp_ctn = typename __get_tmp_ctn<tmp_type>::type;

			tmp_ctn tc = __get_tmp_ctn<tmp_type>::make(dst[0]);


			auto di = dst.begin();
			auto dend = dst.end();

			auto si = src.begin() + ofs;
			auto send0 = src.begin();
			auto send1 = src.end() + 1 - _Ksz;

			for (; si < send0 && di < dend; ++di, si += _Str)
			{

			}

			for (; si < send1 && di < dend; ++di, si += _Str)
			{
				_unpack_iterator(
					__apl_kernel<src_type>::run,
					kn, tc, si, make_type_index<_Ksz>);
			}

			for (; di < dend; ++di, si += _Str)
			{

			}

		}
	};




	template<typename _S, typename _K>
	struct __get_tmp_type
	{
		using type = decltype(__get_t<_S>() * __get_t<_K>() + __get_t<_S>() * __get_t<_K>());
	};

	template<typename _Elm, size_t _Sz, typename _K>
	struct __get_tmp_type<std::array<_Elm, _Sz>, _K>
	{
		using type = std::array<typename __get_tmp_type<_Elm, _K>::type, _Sz>;
	};

	template<typename _Base, typename _K>
	struct __get_tmp_type<array_ptr<_Base>, _K>
	{
		using type = array_ptr<typename __get_tmp_type<_Base, _K>::type>;
	};


	template<typename _Base, typename _K>
	struct __get_tmp_type<base_tensor_ptr<_Base>, _K>
	{
		using type = base_tensor_ptr<typename __get_tmp_type<_Base, _K>::type>;
	};

	template<typename _P>
	struct __get_tmp_ctn
	{
		using type = _P;

		template<typename _T>
		static type make(const _T &)
		{
			return type();
		}
	};

	template<typename _P, size_t _S>
	struct __get_tmp_ctn<std::array<_P, _S>>
	{
		using type = std::array<_P, _S>;

		template<typename _T>
		static type make(const _T &)
		{
			return type();
		}
	};

	template<typename _Elm>
	struct __get_tmp_ctn<array_ptr<_Elm>>
	{
		using type = std::vector<_Elm>;

		template<typename _T>
		static type make(const _T & dst)
		{
			return type(size(dst));
		}
	};

	template<typename _Elm>
	struct __get_tmp_ctn<base_tensor_ptr<_Elm>>
	{
		using type = tensor<typename base_tensor_ptr<_Elm>::elm_type, base_tensor_ptr<_Elm>::dim>;

		template<typename _T>
		static type make(const _T & dst)
		{
			return type(size(dst));
		}
	};



	template<unsigned int _Dim, typename _Ke, typename _Knl, typename _Cnt>
	inline auto __conv_1d(_Cnt && ts, _Knl kn = _Knl())
	{
		auto src = to_base_ptr(to_ptr(ts));
		auto dst_ctn = make_container(src);
		resize_at<_Dim>(dst_ctn, kn.result_size());
		using tmp_ptr = typename ptr_type_at<decltype(src), _Dim>::type;
		using src_elm_type = typename ptr_get_elm<tmp_ptr>::type;

		__conv_1d<_Dim>::run(to_ptr(dst_ctn), src);
		return dst_ctn;
	}
}


#endif