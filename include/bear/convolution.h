﻿#pragma once

#include "tensor.h"
#include "image.h"
#include "ptr_traits.h"
#include "ptr_algorism.h"

namespace bear
{
	template<int _Dim>
	struct __conv_1d
	{
		static size_t clip_to_sub(size_t pad)
		{
			return 0;
		}

		template<typename _Dst, typename _Src, int _KDim>
		static auto clip_to(_Dst dst, _Src src, size_t pad, std::integral_constant<int, _KDim> krev)
		{
			dst = clip_at<_Dim - 1>(dst, 0, size_at<_Dim - 1>(src) + __conv_1d<_Dim - 1 - _KDim>::clip_to_sub(pad));
			return __conv_1d<_Dim - 1>::clip_to(dst, src, pad, krev);
		}


		template<typename _Dst, typename _Src>
		static bool check_size_sub(_Dst&& dst, _Src&& src, size_t ks)
		{
			return dst.size() == src.size();
		}

		template<typename _Dst, typename _Src, int _KRevDim>
		static bool check_size(_Dst&& dst, _Src&& src, size_t ks, std::integral_constant<int, _KRevDim> krev)
		{
			if (!__conv_1d<_Dim - _KRevDim>::check_size_sub(dst, src, ks)) return false;
			return __conv_1d<_Dim - 1>::check_size(dst[0], src[0], ks, krev);
		}

		template<typename _Src>
		static auto step_sub(_Src src1, _Src src2)
		{
			return src2->begin();
		}

		template<typename _Src, int _KRevDim>
		static ptrdiff_t step(_Src src1, _Src src2, std::integral_constant<int, _KRevDim> krev)
		{
			return __conv_1d<_Dim - 1>::step(src1->begin(), __conv_1d<_Dim - _KRevDim>::step_sub(src1, src2), krev);
		}

		template<typename _Dst, typename _Src, typename _Ke, typename _Knl>
		static void run(_Dst&& dst, _Src&& src, _Ke&& kennel, _Knl&& kf, ptrdiff_t step)
		{
			auto end = dst.end();
			auto src_p = src.begin();
			for (auto dst_p = dst.begin(); dst_p != end; ++dst_p, ++src_p)
			{
				__conv_1d<_Dim - 1>::run(*dst_p, *src_p, std::forward<_Ke>(kennel), std::forward<_Knl>(kf), step);
			}
		}
	};


	template<>
	struct __conv_1d<0>
	{
		static size_t clip_to_sub(size_t pad)
		{
			return pad;
		}

		template<typename _Dst, typename _Src, int _KDim>
		static auto clip_to(_Dst dst, _Src src, size_t pad, std::integral_constant<int, _KDim> krev)
		{
			return dst;
		}

		template<typename _Dst, typename _Src>
		static bool check_size_sub(_Dst&& dst, _Src&& src, size_t ks)
		{
			return dst.size() + ks - 1 == src.size();
		}

		template<typename _Dst, typename _Src, typename _Any>
		static bool check_size(_Dst&& dst, _Src&& src, size_t ks, _Any)
		{
			return true;
		}

		template<typename _Src>
		static auto step_sub(_Src src1, _Src src2)
		{
			return src1->begin() + 1;
		}

		template<typename _Src, typename _Any>
		static ptrdiff_t step(_Src src1, _Src src2, _Any)
		{
			return (ptrdiff_t) & *src2 - (ptrdiff_t) & *src1;
		}

		template<typename _Dst, typename _Src, typename _Ke, typename _Knl>
		static void run(_Dst& dst, _Src& _src, _Ke&& kennel, _Knl&& kf, ptrdiff_t step)
		{
			auto tmp = kf.zero();

			char* src = (char*)&_src;
			auto end = kennel.end();
			for (auto kennel_p = kennel.begin(); kennel_p != end; ++kennel_p, src += step)
			{

				kf.mul_add(tmp, *((_Src*)src), *kennel_p);
			}

			kf.store(dst, tmp);
		}
	};

	template<typename _Dst, typename _Src, typename _Ke>
	struct conv_1d_default
	{
		using tmp_t = decltype((_Src)0 * (_Ke)0 + (_Src)0 * (_Ke)0);

		static tmp_t zero()
		{
			return 0;
		}

		static void mul_add(tmp_t& tmp, const _Src& m1, const _Ke& m2)
		{
			tmp += m1 * m2;
		}

		static void store(_Dst& dst, const tmp_t& tmp)
		{
			dst = tmp;
		}
	};

	template<unsigned int _Dim, typename _Dst, typename _Src>
	inline auto conv_1d_pad(_Dst&& _dst, _Src&& _src, size_t left, size_t right)
	{
		auto dst = to_base_ptr(to_ptr(_dst));
		auto src = to_base_ptr(to_ptr(_src));

		dst = __conv_1d<decltype(dst)::dim>::clip_to(
			dst, src, left + right, std::integral_constant<int, _Dim>());

		auto right_b = size_at<_Dim>(dst) - right;
		auto dst_c = clip_at<_Dim>(dst, left, right_b);

		if (dst_c.data() != src.data())
		{
			copy(dst_c, src);
		}
		else if (!is_same_size(dst_c, src))
		{
			throw bear_exception(exception_type::size_different, "wrong dst size!");
		}

		auto left_p = clip_at<_Dim>(dst, left, left + 1);

		for (size_t i = 0; i < left; ++i)
		{
			copy(clip_at<_Dim>(dst, i, i + 1), left_p);
		}

		auto right_p = clip_at<_Dim>(dst, right_b - 1, right_b);

		for (size_t i = 0; i < right; ++i)
		{
			copy(clip_at<_Dim>(dst, right_b + i, right_b + i + 1), right_p);
		}

		return dst;
	}

	template<unsigned int _Dim, typename _Dst, typename _Src, typename _Ke, typename _Knl>
	inline void conv_1d(_Dst&& _dst, _Src&& _src, _Ke&& kennel, _Knl&& kn)
	{
		auto dst = to_ptr(_dst);
		auto src = to_ptr(_src);

		using dst_t = decltype(dst);
		using src_t = decltype(src);

		static_assert(ptr_traits<src_t>::dim <= ptr_traits<dst_t>::dim, "src dim must less or eq than dst!");
		static_assert(ptr_traits<src_t>::dim > _Dim, "src dim must great than conv dim!");

		using cc = __conv_1d <ptr_traits<src_t>::dim>;

		auto ok = cc::check_size(dst, src, kennel.size(), std::integral_constant<int, ptr_traits<src_t>::dim - _Dim>());
		if (!ok) throw bear_exception(exception_type::size_different, literal_u8("wrong dst size!"));

		auto step = cc::step(&src, &src, std::integral_constant<int, ptr_traits<src_t>::dim - _Dim>());

		cc::run(dst, src, std::forward<_Ke>(kennel), std::forward<_Knl>(kn), step);
	}


	template<unsigned int _Dim, typename _Dst, typename _Tmp, typename _Src, typename _Ke, typename _Knl>
	inline void conv_1d(_Dst&& _dst,
		_Tmp&& _tmp, _Src&& _src, size_t left_pad, size_t right_pad,
		_Ke&& kennel, _Knl&& kn)
	{
		auto tmp = conv_1d_pad<_Dim>(
			std::forward<_Tmp>(_tmp),
			std::forward<_Src>(_src),
			left_pad,
			right_pad);

		conv_1d<_Dim>(
			std::forward<_Dst>(_dst),
			tmp,
			std::forward<_Ke>(kennel),
			std::forward<_Knl>(kn));
	}

	template<unsigned int _Dim, typename _Dst, typename _Src, typename _Ke>
	inline void conv_1d(_Dst&& _dst, _Src&& _src, _Ke&& kennel)
	{
		auto dst = to_ptr(_dst);
		auto src = to_ptr(_src);

		using dst_elm = typename ptr_traits<decltype(dst)>::elm_type;
		using src_elm = typename ptr_traits<decltype(src)>::elm_type;
		using kel_elm = typename std::decay<decltype(kennel[0])>::type;

		using Knl = conv_1d_default<dst_elm, src_elm, kel_elm>;

		conv_1d(dst, src, std::forward<_Ke>(kennel), Knl());
	}

	template<size_t input_bits, int ofs = 0>
	struct conv_1d_int16
	{
		static int zero()
		{
			return 0;
		}

		static void mul_add(int& tmp, int m1, int m2)
		{
			tmp += m1 * m2;
		}

		static int store_inner(int tmp)
		{
			return ((tmp + (1 << (input_bits - 1))) >> input_bits) + ofs;
		}

		static void store(unsigned char& dst, int tmp)
		{
			int ret = store_inner(tmp);

			if ((unsigned int)ret > 0xff)
			{
				if (ret > 0xff) ret = 0xff;
				else ret = 0;
			}

			dst = (unsigned char)ret;
		}

		static void store(char& dst, int tmp)
		{
			int ret = store_inner(tmp);

			int rp = ret + 0x80;

			if ((unsigned int)rp > 0xff)
			{
				if (rp > 0xff) ret = 0x7f;
				else ret = 0xffffff80;
			}

			dst = (char)ret;
		}

		static void store(unsigned short& dst, int tmp)
		{
			int ret = store_inner(tmp);

			if ((unsigned int)ret > 0xffff)
			{
				if (ret > 0xffff) ret = 0xffff;
				else ret = 0;
			}

			dst = (unsigned short)ret;
		}

		static void store(short& dst, int tmp)
		{
			int ret = store_inner(tmp);

			int rp = ret + 0x8000;

			if ((unsigned int)rp > 0xffff)
			{
				if (rp > 0xffff) ret = 0x7fff;
				else ret = 0xffff8000;
			}

			dst = (short)ret;
		}

		static void store(int& dst, int tmp)
		{
			dst = store_inner(tmp);
		}
	};


}
