#pragma once

#include "image.h"
#include "tensor.h"

namespace bear
{
	enum data_type
	{
		image_unknown_type,
		image_int_type,
		image_unsigned_int_type,
		image_float_type
	};

	template<typename _T>
	struct data_type_traits
	{
	};

	template<>
	struct data_type_traits<char>
	{
		static constexpr data_type value = image_int_type;
	};

	template<>
	struct data_type_traits<short>
	{
		static constexpr data_type value = image_int_type;
	};

	template<>
	struct data_type_traits<int>
	{
		static constexpr data_type value = image_int_type;
	};

	template<>
	struct data_type_traits<long long>
	{
		static constexpr data_type value = image_int_type;
	};

	template<>
	struct data_type_traits<unsigned char>
	{
		static constexpr data_type value = image_unsigned_int_type;
	};

	template<>
	struct data_type_traits<unsigned short>
	{
		static constexpr data_type value = image_unsigned_int_type;
	};

	template<>
	struct data_type_traits<unsigned int>
	{
		static constexpr data_type value = image_unsigned_int_type;
	};

	template<>
	struct data_type_traits<unsigned long long>
	{
		static constexpr data_type value = image_unsigned_int_type;
	};

	template<>
	struct data_type_traits<float>
	{
		static constexpr data_type value = image_float_type;
	};

	template<>
	struct data_type_traits<double>
	{
		static constexpr data_type value = image_float_type;
	};

	struct dynamic_image_info
	{
		size_t _width;
		size_t _height;
		size_t _channel_size;
		data_type _elm_type;
		size_t _elm_size;
		char* _data;
		size_t _width_step;
	};

	template<typename _Img>
	inline dynamic_image_info get_image_info(const _Img& ptr, data_type dt)
	{
		dynamic_image_info ret;

		ret._width = width(ptr);
		ret._height = height(ptr);
		ret._channel_size = channel_size(ptr);

		ret._elm_type = dt;
		ret._elm_size = ptr.elm_size();
		ret._data = (char*)ptr.data();
		ret._width_step = ptr.move_step();

		return ret;
	}

	template<typename _Elm>
	inline dynamic_image_info get_image_info(const base_tensor_ptr<_Elm>& ptr)
	{
		dynamic_image_info ret;

		ret._width = width(ptr);
		ret._height = height(ptr);
		ret._channel_size = channel_size(ptr);

		ret._elm_type = data_type_traits<typename std::decay<typename base_tensor_ptr<_Elm>::elm_type>::type>::value;
		ret._elm_size = sizeof(typename base_tensor_ptr<_Elm>::elm_type);
		ret._data = (char*)ptr.data();
		ret._width_step = ptr.move_step();

		return ret;
	}


	template<typename _Elm, size_t _CH>
	inline dynamic_image_info get_image_info(const const_image_ptr<_Elm, _CH>& ptr)
	{
		dynamic_image_info ret;

		ret._width = width(ptr);
		ret._height = height(ptr);
		ret._channel_size = channel_size(ptr);

		ret._elm_type = data_type_traits<_Elm>::value;
		ret._elm_size = sizeof(_Elm);
		ret._data = (char*)ptr.data();
		ret._width_step = ptr.move_step();

		return ret;
	}

	template<typename _Elm, size_t _CH>
	inline dynamic_image_info get_image_info(const image_ptr<_Elm, _CH>& ptr)
	{
		dynamic_image_info ret;

		ret._width = width(ptr);
		ret._height = height(ptr);
		ret._channel_size = channel_size(ptr);

		ret._elm_type = data_type_traits<_Elm>::value;
		ret._elm_size = sizeof(_Elm);
		ret._data = (char*)ptr.data();
		ret._width_step = ptr.move_step();

		return ret;
	}

#ifdef CV_MAJOR_VERSION

#if CV_MAJOR_VERSION < 3

	inline dynamic_image_info get_image_info(const IplImage& img)
	{
		dynamic_image_info ret;

		ret._width = img.width;
		ret._height = img.height;
		ret._channel_size = img.nChannels;

		ret._elm_type = image_unknown_type;
		ret._elm_size = img.depth >> 3;
		ret._data = (char*)img.imageData;
		ret._width_step = img.widthStep;

		return ret;
	}

	inline dynamic_image_info get_image_info_roi(const IplImage& img)
	{
		auto ret = get_image_info(img);

		if (!img.roi)return ret;

		ret._width = img.roi->width;
		ret._height = img.roi->height;
		ret._data = (char*)(
			img.imageData +
			img.roi->yOffset * img.widthStep +
			img.roi->xOffset * (img.depth >> 3) * img.nChannels);

		return ret;
	}


	inline int get_image_cv_type(const dynamic_image_info& _info)
	{
		int type = 0;

		if (1 == _info._elm_size)
		{
			if (image_int_type == _info._elm_type)
			{
				type = CV_MAKETYPE(CV_8S, (int)_info._channel_size);
			}
			else if (image_unsigned_int_type == _info._elm_type)
			{
				type = CV_MAKETYPE(CV_8U, (int)_info._channel_size);
			}
		}
		else if (2 == _info._elm_size)
		{
			if (image_int_type == _info._elm_type)
			{
				type = CV_MAKETYPE(CV_16S, (int)_info._channel_size);
			}
			else if (image_unsigned_int_type == _info._elm_type)
			{
				type = CV_MAKETYPE(CV_16U, (int)_info._channel_size);
			}
		}
		else if (4 == _info._elm_size)
		{
			if (image_int_type == _info._elm_type)
			{
				type = CV_MAKETYPE(CV_32S, (int)_info._channel_size);
			}
			else if (image_float_type == _info._elm_type)
			{
				type = CV_MAKETYPE(CV_32F, (int)_info._channel_size);
			}
		}
		else if (8 == _info._elm_size)
		{
			if (image_float_type == _info._elm_type)
			{
				type = CV_MAKETYPE(CV_64F, (int)_info._channel_size);
			}
		}

		return type;
	}

	inline dynamic_image_info get_image_info(const CvMat& img)
	{
		dynamic_image_info ret;

		ret._width = img.width;
		ret._height = img.height;
		ret._channel_size = CV_MAT_CN(img.type);

		switch (CV_MAT_DEPTH(img.type))
		{
		case CV_8U:
			ret._elm_type = image_unsigned_int_type;
			ret._elm_size = 1;
			break;
		case CV_8S:
			ret._elm_type = image_int_type;
			ret._elm_size = 1;
			break;
		case CV_16U:
			ret._elm_type = image_unsigned_int_type;
			ret._elm_size = 2;
			break;
		case CV_16S:
			ret._elm_type = image_int_type;
			ret._elm_size = 2;
			break;
		case CV_32S:
			ret._elm_type = image_int_type;
			ret._elm_size = 4;
			break;
		case CV_32F:
			ret._elm_type = image_float_type;
			ret._elm_size = 4;
			break;
		case CV_64F:
			ret._elm_type = image_float_type;
			ret._elm_size = 8;
			break;
		default:
			ret._elm_type = image_unknown_type;
			ret._elm_size = 1;
		}

		ret._width_step = img.step;
		ret._data = (char*)img.data.ptr;

		return ret;
	}

	inline dynamic_image_info get_image_info(const cv::Mat& _img)
	{
		CvMat img(_img);
		return get_image_info(img);
	}
#else
	inline dynamic_image_info get_image_info(const cv::Mat& img)
	{
		dynamic_image_info ret;

		ret._width = img.size[1];
		ret._height = img.size[0];

		//switch (img.dims)
		//{
		//case 2:
		//	ret._channel_size = 1;
		//	break;
		//case 3:
		//	ret._channel_size = img.size[2];
		//	break;
		//default:
		//	throw bear_exception(exception_type::pointer_outof_range, literal_u8("mat dim more than three!"));
		//	break;
		//}

		switch (CV_MAT_DEPTH(img.type()))
		{
		case CV_8U:
			ret._elm_type = image_unsigned_int_type;
			ret._elm_size = 1;
			break;
		case CV_16U:
			ret._elm_type = image_unsigned_int_type;
			ret._elm_size = 2;
			break;
		case CV_8S:
			ret._elm_type = image_int_type;
			ret._elm_size = 1;
			break;
		case CV_16S:
			ret._elm_type = image_int_type;
			ret._elm_size = 2;
			break;
		case CV_32S:
			ret._elm_type = image_int_type;
			ret._elm_size = 4;
			break;
		case CV_32F:
			ret._elm_type = image_float_type;
			ret._elm_size = 4;
			break;
		case CV_64F:
			ret._elm_type = image_float_type;
			ret._elm_size = 8;
			break;

#if CV_MAJOR_VERSION >= 4
		case CV_16F:
			ret._elm_type = image_float_type;
			ret._elm_size = 2;
			break;
#endif
		default:
			throw bear_exception(exception_type::pointer_outof_range, literal_u8("unknown cv type!"));
			break;
		}

		ret._channel_size = (img.type() >> CV_CN_SHIFT) + 1;
		ret._data = (char*)img.data;
		ret._width_step = img.step[0];

		return ret;
	}

	inline int get_image_cv_type(const dynamic_image_info& _info)
	{
		int type = 0;

		if (1 == _info._elm_size)
		{
			if (image_int_type == _info._elm_type)
			{
				type = CV_MAKETYPE(CV_8S, (int)_info._channel_size);
			}
			else if (image_unsigned_int_type == _info._elm_type)
			{
				type = CV_MAKETYPE(CV_8U, (int)_info._channel_size);
			}
		}
		else if (2 == _info._elm_size)
		{
			if (image_int_type == _info._elm_type)
			{
				type = CV_MAKETYPE(CV_16S, (int)_info._channel_size);
			}
			else if (image_unsigned_int_type == _info._elm_type)
			{
				type = CV_MAKETYPE(CV_16U, (int)_info._channel_size);
			}
		}
		else if (4 == _info._elm_size)
		{
			if (image_int_type == _info._elm_type)
			{
				type = CV_MAKETYPE(CV_32S, (int)_info._channel_size);
			}
			else if (image_float_type == _info._elm_type)
			{
				type = CV_MAKETYPE(CV_32F, (int)_info._channel_size);
			}
		}
		else if (8 == _info._elm_size)
		{
			if (image_float_type == _info._elm_type)
			{
				type = CV_MAKETYPE(CV_64F, (int)_info._channel_size);
			}
		}

		return type;
	}

#endif



#endif



#ifdef QT_VERSION

	inline dynamic_image_info _make_d_image(const QImage& img)
	{
		dynamic_image_info ret;

		ret._width = img.width();
		ret._height = img.height();

		switch (img.format())
		{
		case QImage::Format_Mono:
		case QImage::Format_MonoLSB:
		case QImage::Format_Indexed8:
		case QImage::Format_Alpha8:
		case QImage::Format_Grayscale8:
			ret._channel_size = 1;
			ret._elm_size = 1;
			break;
		case QImage::Format_ARGB32:
		case QImage::Format_ARGB32_Premultiplied:
		case QImage::Format_RGBX8888:
		case QImage::Format_RGBA8888:
		case QImage::Format_RGBA8888_Premultiplied:
			ret._channel_size = 4;
			ret._elm_size = 1;
			break;
		case QImage::Format_RGB32:
		case QImage::Format_BGR30:
		case QImage::Format_A2BGR30_Premultiplied:
		case QImage::Format_RGB30:
		case QImage::Format_A2RGB30_Premultiplied:
			ret._channel_size = 1;
			ret._elm_size = 4;
			break;
		case QImage::Format_RGB16:
		case QImage::Format_RGB555:
		case QImage::Format_RGB444:
		case QImage::Format_ARGB4444_Premultiplied:
			ret._channel_size = 1;
			ret._elm_size = 2;
			break;
		case QImage::Format_ARGB8565_Premultiplied:
		case QImage::Format_RGB666:
		case QImage::Format_ARGB6666_Premultiplied:
		case QImage::Format_ARGB8555_Premultiplied:
		case QImage::Format_RGB888:
			ret._channel_size = 3;
			ret._elm_size = 1;
			break;
		default:
			ret._channel_size = 4;
			ret._elm_size = 1;
		}

		ret._elm_type = image_unsigned_int_type;
		ret._width_step = img.bytesPerLine();

		return ret;
	}

	inline dynamic_image_info get_image_info(QImage& img)
	{
		dynamic_image_info ret = _make_d_image(img);
		ret._data = (char*)img.bits();
		return ret;
	}

	inline const dynamic_image_info get_image_info(const QImage& img)
	{
		dynamic_image_info ret = _make_d_image(img);
		ret._data = (char*)img.bits();
		return ret;
	}

	inline dynamic_image_info get_image_info(const dynamic_image_info& img)
	{
		return img;
	}

#endif // QT_VERSION



	class base_dynamic_image_ptr
	{
	protected:
		dynamic_image_info _info;
	public:

		bool empty() const
		{
			return _info._height == 0;
		}

		size_t width() const
		{
			return _info._width;
		}

		size_t height() const
		{
			return _info._height;
		}

		image_size size() const
		{
			return image_size{ width(), height() };
		}

		size_t channel_size() const
		{
			return _info._channel_size;
		}

		data_type elm_type() const
		{
			return _info._elm_type;
		}

		size_t elm_size() const
		{
			return _info._elm_size;
		}

		size_t width_setp() const
		{
			return _info._width_step;
		}
	};


	class dynamic_image_ptr : public base_dynamic_image_ptr
	{

	public:

		char* data() const
		{
			return _info._data;
		}

		array_ptr<char> data_array()
		{
			return array_ptr<char>(_info._data, _info._width_step * _info._height);
		}

		template<typename _Elm, size_t _Ch>
		dynamic_image_ptr(const image_ptr<_Elm, _Ch>& img)
		{
			static_assert(!std::is_const<_Elm>::value, "should not be const");
			_info = get_image_info(img);
		}

		template<typename _Elm, size_t _Ch>
		dynamic_image_ptr(const image<_Elm, _Ch>& img)
		{
			static_assert(!std::is_const<_Elm>::value, "should not be const");
			_info = get_image_info(to_ptr(img), data_type_traits<typename std::decay<_Elm>::type>::value);
		}

		template<typename _Elm>
		dynamic_image_ptr(base_tensor_ptr<base_tensor_ptr<array_ptr<_Elm>>> img)
		{
			static_assert(!std::is_const<_Elm>::value, "should not be const");
			_info = get_image_info(img);
		}

		template<typename _Elm>
		dynamic_image_ptr(base_tensor_ptr<array_ptr<_Elm>> img)
		{
			static_assert(!std::is_const<_Elm>::value, "should not be const");
			_info = get_image_info(img);
		}

		template<typename _Elm>
		dynamic_image_ptr(const tensor<_Elm, 3>& img)
		{
			static_assert(!std::is_const<_Elm>::value, "should not be const");
			_info = get_image_info(to_ptr(img));
		}

		template<typename _Elm>
		dynamic_image_ptr(const tensor<_Elm, 2>& img)
		{
			static_assert(!std::is_const<_Elm>::value, "should not be const");
			_info = get_image_info(to_ptr(img));
		}

#ifdef CV_MAJOR_VERSION

#if CV_MAJOR_VERSION < 3

		dynamic_image_ptr(const IplImage& img)
		{
			_info = get_image_info(img);
		}

		dynamic_image_ptr(const IplImage& img, bool roi_true)
		{
			_info = get_image_info_roi(img);
		}

		dynamic_image_ptr(const CvMat& img)
		{
			_info = get_image_info(img);
		}

#endif

		dynamic_image_ptr(const cv::Mat& img)
		{
			_info = get_image_info(img);
		}

		operator cv::Mat()
		{
			int type = get_image_cv_type(_info);

			cv::Mat ret((int)_info._height, (int)_info._width, type, _info._data, (int)_info._width_step);
			return ret;
		}

#endif

#ifdef QT_VERSION

		dynamic_image_ptr(QImage& img)
		{
			_info = get_image_info(img);
		}

#endif // QT_VERSION


		friend class const_dynamic_image_ptr;

		dynamic_image_ptr() {}

		dynamic_image_ptr(
			size_t width,
			size_t height,
			size_t channel_size,
			data_type elm_type,
			size_t elm_size,
			char* data,
			size_t width_step = 0)
		{
			_info._width = width;
			_info._height = height;
			_info._channel_size = channel_size;
			_info._elm_type = elm_type;
			_info._elm_size = elm_size;
			_info._data = data;
			_info._width_step = width_step;

			size_t ws = _info._width * _info._channel_size * _info._elm_size;
			if (!_info._width_step)_info._width_step = ws;
			if (_info._width_step < ws) throw bear_exception(exception_type::pointer_outof_range, literal_u8("width exceed width step!"));
		}

		template<typename _Elm, size_t _Ch>
		explicit operator image_ptr<_Elm, _Ch>() const
		{
			if ((image_unknown_type != _info._elm_type || !empty()) &&
				(_Ch != _info._channel_size || sizeof(_Elm) != _info._elm_size))
				throw bear_exception(exception_type::size_different, "pixel size different!");
			const auto et = data_type_traits<typename std::decay<_Elm>::type>::value;
			if (image_unknown_type != _info._elm_type && _info._elm_type != et)
			{
				throw bear_exception(exception_type::size_different, "wrong type!");
			}
			return image_ptr<_Elm, _Ch>((typename image_ptr<_Elm, _Ch>::channel_type*)_info._data, _info._width_step, _info._width, _info._height);
		}

		template<typename _Elm>
		explicit operator base_tensor_ptr<base_tensor_ptr<array_ptr<_Elm>>>() const
		{
			if ((image_unknown_type != _info._elm_type || !empty()) &&
				sizeof(_Elm) != _info._elm_size)
				throw bear_exception(exception_type::size_different, "pixel size different!");
			const auto et = data_type_traits<typename std::decay<_Elm>::type>::value;
			if (image_unknown_type != _info._elm_type && _info._elm_type != et)
			{
				throw bear_exception(exception_type::size_different, "wrong type!");
			}
			return make_tensor_ptr(
				make_tensor_ptr(
					make_tensor_ptr(
						(_Elm*)_info._data,
						_info._channel_size
					),
					_info._width
				),
				_info._height,
				_info._width_step
			);
		}
	};


	class const_dynamic_image_ptr : public base_dynamic_image_ptr
	{

	public:

		const char* data() const
		{
			return _info._data;
		}

		const_array_ptr<char> data_array()
		{
			return const_array_ptr<char>(_info._data, _info._width_step * _info._height);
		}

#ifdef CV_MAJOR_VERSION

#if CV_MAJOR_VERSION < 3
		const_dynamic_image_ptr(const IplImage& img)
		{
			_info = get_image_info(img);
		}

		const_dynamic_image_ptr(IplImage* img)
		{
			_info = get_image_info(*img);
		}

		const_dynamic_image_ptr(const CvMat& img)
		{
			_info = get_image_info(img);
		}
#endif

		const_dynamic_image_ptr(const cv::Mat& img)
		{
			_info = get_image_info(img);
		}

#endif

#ifdef QT_VERSION

		const_dynamic_image_ptr(const QImage& img)
		{
			_info = get_image_info(img);
		}

#endif // QT_VERSION

		template<typename _Elm, size_t _Ch>
		const_dynamic_image_ptr(const image_ptr<_Elm, _Ch>& img)
		{
			_info = get_image_info(img);
		}

		template<typename _Elm, size_t _Ch>
		const_dynamic_image_ptr(const image<_Elm, _Ch>& img)
		{
			_info = get_image_info(to_ptr(img));
		}

		template<typename _Elm>
		const_dynamic_image_ptr(base_tensor_ptr<base_tensor_ptr<array_ptr<_Elm>>> img)
		{
			_info = get_image_info(img);
		}

		template<typename _Elm>
		const_dynamic_image_ptr(base_tensor_ptr<array_ptr<_Elm>> img)
		{
			_info = get_image_info(img);
		}

		template<typename _Elm>
		const_dynamic_image_ptr(const tensor<_Elm, 3>& img)
		{
			_info = get_image_info(to_ptr(img));
		}

		template<typename _Elm>
		const_dynamic_image_ptr(const tensor<_Elm, 2>& img)
		{
			_info = get_image_info(to_ptr(img));
		}

		const_dynamic_image_ptr(const dynamic_image_ptr& other)
		{
			_info = other._info;
		}

		const_dynamic_image_ptr() {}

		const_dynamic_image_ptr(
			size_t width,
			size_t height,
			size_t channel_size,
			data_type elm_type,
			size_t elm_size,
			const char* data,
			size_t width_step = 0)
		{
			_info._width = width;
			_info._height = height;
			_info._channel_size = channel_size;
			_info._elm_type = elm_type;
			_info._elm_size = elm_size;
			_info._data = (char*)data;
			_info._width_step = width_step;

			size_t ws = _info._width * _info._channel_size * _info._elm_size;
			if (!_info._width_step)_info._width_step = ws;
			if (_info._width_step < ws) throw bear_exception(exception_type::pointer_outof_range, literal_u8("width exceed width step!"));
		}

		template<typename _Elm, size_t _Ch>
		explicit operator image_ptr<_Elm, _Ch>() const
		{
			static_assert(std::is_const<_Elm>::value, "should be const!");

			if ((image_unknown_type != _info._elm_type || !empty()) &&
				(_Ch != _info._channel_size || sizeof(_Elm) != _info._elm_size))
				throw bear_exception(exception_type::size_different, "pixel size different!");
			const auto et = data_type_traits<typename std::decay<_Elm>::type>::value;
			if (image_unknown_type != _info._elm_type && _info._elm_type != et)
			{
				assert(et == _info._elm_type);
			}
			return image_ptr<_Elm, _Ch>((typename image_ptr<_Elm, _Ch>::channel_type*)_info._data, _info._width_step, _info._width, _info._height);
		}

		template<typename _Elm>
		explicit operator base_tensor_ptr<base_tensor_ptr<array_ptr<_Elm>>>() const
		{
			static_assert(std::is_const<_Elm>::value, "should be const!");

			if ((image_unknown_type != _info._elm_type || !empty()) &&
				sizeof(_Elm) != _info._elm_size)
				throw bear_exception(exception_type::size_different, "pixel size different!");
			const auto et = data_type_traits<typename std::decay<_Elm>::type>::value;
			if (image_unknown_type != _info._elm_type && _info._elm_type != et)
			{
				throw bear_exception(exception_type::size_different, "wrong type!");
			}
			return make_tensor_ptr(
				make_tensor_ptr(
					make_tensor_ptr(
						(_Elm*)_info._data,
						_info._channel_size
					),
					_info._width
				),
				_info._height,
				_info._width_step
			);
		}
	};


	inline image_size size(const base_dynamic_image_ptr& img)
	{
		return image_size{ img.width(),img.height() };
	}

	inline size_t width(const base_dynamic_image_ptr& img)
	{
		return img.width();
	}

	inline size_t height(const base_dynamic_image_ptr& img)
	{
		return img.height();
	}

	inline size_t channel_size(const base_dynamic_image_ptr& img)
	{
		return img.channel_size();
	}


	inline auto clip_image(const const_dynamic_image_ptr& img, image_rectangle r)
	{
		if (r.pos.x < 0 || r.pos.y < 0 || r.pos.x + r.size.width > img.width() || r.pos.y + r.size.height > img.height())
		{
			throw bear_exception(exception_type::pointer_outof_range, "rectangle out of range!");
		}

		auto index = r.pos.y * img.width_setp() + r.pos.x * img.channel_size() * img.elm_size();
		return const_dynamic_image_ptr(
			r.size.width,
			r.size.height,
			img.channel_size(),
			img.elm_type(),
			img.elm_size(),
			img.data() + index,
			img.width_setp());
	}

	inline auto clip_image(const dynamic_image_ptr& img, image_rectangle r)
	{
		if (r.pos.x < 0 || r.pos.y < 0 || r.pos.x + r.size.width > img.width() || r.pos.y + r.size.height > img.height())
		{
			throw bear_exception(exception_type::pointer_outof_range, "rectangle out of range!");
		}

		auto index = r.pos.y * img.width_setp() + r.pos.x * img.channel_size() * img.elm_size();
		return dynamic_image_ptr(
			r.size.width,
			r.size.height,
			img.channel_size(),
			img.elm_type(),
			img.elm_size(),
			img.data() + index,
			img.width_setp());
	}

	inline auto scanline(const const_dynamic_image_ptr& img, size_t y)
	{
		if (y > img.height())
		{
			throw bear_exception(exception_type::pointer_outof_range, "y out of range!");
		}
		return const_array_ptr<char>(img.data() + y * img.width_setp(), img.width());
	}

	inline auto scanline(const dynamic_image_ptr& img, size_t y)
	{
		if (y > img.height())
		{
			throw bear_exception(exception_type::pointer_outof_range, "y out of range!");
		}
		return array_ptr<char>(img.data() + y * img.width_setp(), img.width());
	}

	template<typename Image, typename Arg>
	inline Image image_cast(Arg&& arg)
	{
		return Image(dynamic_image_ptr(std::forward<Arg>(arg)));
	}

	class dynamic_image : public dynamic_image_ptr {
	private:
		std::vector<char> _data;
	public:
		dynamic_image() = default;
		dynamic_image(const dynamic_image& other) = default;
		dynamic_image& operator = (const dynamic_image& other) = default;


		dynamic_image(dynamic_image&& other) noexcept :
			_data(std::move(other._data))
		{
			_info = other._info;
			other._info._width = 0;
			other._info._height = 0;
			other._info._data = NULL;
		}
		dynamic_image& operator = (dynamic_image&& other) noexcept
		{
			_data = std::move(other._data);
			_info = other._info;
			other._info._width = 0;
			other._info._height = 0;
			other._info._data = NULL;

			return *this;
		}

		dynamic_image(const_dynamic_image_ptr other) noexcept :
			_data(other.elm_size()* other.channel_size()* other.width()* other.height())
		{
			_info._width = other.width();
			_info._height = other.height();;
			_info._channel_size = other.channel_size();
			_info._elm_type = other.elm_type();
			_info._elm_size = other.elm_size();
			_info._data = &_data[0];
			_info._width_step = other.elm_size() * other.channel_size() * other.width();

			for (size_t i = 0; i < _info._height; ++i)
			{
				memcpy(
					_info._data + _info._width_step * i,
					other.data() + other.width_setp() * i,
					_info._width_step);
			}
		}
		dynamic_image& operator = (const_dynamic_image_ptr other) noexcept
		{
			_data.resize(other.elm_size() * other.channel_size() * other.width() * other.height());

			_info._width = other.width();
			_info._height = other.height();;
			_info._channel_size = other.channel_size();
			_info._elm_type = other.elm_type();
			_info._elm_size = other.elm_size();
			_info._data = &_data[0];
			_info._width_step = other.elm_size() * other.channel_size() * other.width();

			for (size_t i = 0; i < _info._height; ++i)
			{
				memcpy(
					_info._data + _info._width_step * i,
					other.data() + other.width_setp() * i,
					_info._width_step);
			}

			return *this;
		}

		array_ptr<char> data_array()
		{
			return _data;
		}

		dynamic_image(
			size_t width,
			size_t height,
			size_t channel_size,
			data_type elm_type,
			size_t elm_size) :
			dynamic_image_ptr(width, height, channel_size, elm_type, elm_size, 0, width* channel_size* elm_size),
			_data(width* height* channel_size* elm_size)
		{
			_info._data = &_data[0];
		}
	};
}
