#ifndef _IMAGE_H
#define _IMAGE_H

#include "tensor.h"
#include "ptr_algorism.h"

#define F_RGBA (0 | (1 << 8) | (2 << 16) | (3 << 24) | (4 << 28))
#define F_BGRA (2 | (1 << 8) | (0 << 16) | (3 << 24) | (4 << 28))
#define F_ARGB (1 | (2 << 8) | (3 << 16) | (0 << 24) | (4 << 28))
#define F_ABGR (3 | (2 << 8) | (1 << 16) | (0 << 24) | (4 << 28))
#define F_RGB (0 | (1 << 8) | (2 << 16) | (15 << 24) | (3 << 28))
#define F_BGR (2 | (1 << 8) | (0 << 16) | (15 << 24) | (3 << 28))

#define FI_RED(n)   (n & 255)
#define FI_GREEN(n) ((n >> 8) & 255)
#define FI_BLUE(n)  ((n >> 16) & 255)
#define FI_ALPHA(n) ((n >> 24) & 15)
#define FI_BPP(n)   ((n >> 28) & 15)

namespace bear
{
	using pos_t = std::ptrdiff_t;

	class image_size
	{
	public:
		image_size() = default;
		image_size(size_t _width, size_t _height) :height(_height), width(_width) {}
		image_size(const tensor_size<2> &oth) :height(oth[0]), width(oth[1]){}

		size_t height = 0;
		size_t width = 0;

		operator tensor_size<2>()
		{
			tensor_size<2> ret;
			ret[0] = height;
			ret[1] = width;
			return ret;
		}

		bool operator == (const image_size &oth)
		{
			return width == oth.width && height == oth.height;
		}

		bool operator != (const image_size &oth)
		{
			return width != oth.width || height!= oth.height;
		}

		friend bool operator == (const image_size &s1, const tensor_size<2> &s2)
		{
			return s1.width == s2[1] && s1.height == s2[0];
		}

		friend bool operator == (const tensor_size<2> &s2, const image_size &s1)
		{
			return s1.width == s2[1] && s1.height == s2[0];
		}

		friend bool operator != (const image_size &s1, const tensor_size<2> &s2)
		{
			return s1.width != s2[1] || s1.height != s2[0];
		}

		friend bool operator != (const tensor_size<2> &s2, const image_size &s1)
		{
			return s1.width != s2[1] || s1.height != s2[0];
		}
	};

	struct image_point
	{
		image_point(pos_t _x, pos_t _y) :x(_x), y(_y) {}
		//image_point(size_t _x, size_t _y) :x(_x), y(_y) {}
		pos_t x = 0;
		pos_t y = 0;
	};

	struct image_rectangle
	{
		image_rectangle(pos_t _x, pos_t _y, size_t _width, size_t _height) :
			pos(_x, _y), size(_width, _height) {}
		image_rectangle(image_point _pos, image_size _size) :
			pos(_pos), size(_size) {}
		image_point pos;
		image_size size;
	};

	template<typename _Elm>
	class base_image_ptr : public tensor_ptr<_Elm,2>
	{
	public:

		using base = tensor_ptr<_Elm, 2>;
		using normal_self = base_image_ptr<typename std::remove_const<_Elm>::type>;
		using const_self = base_image_ptr<const _Elm>;
		using elm_type = typename base::elm_type;
		using elm_pointer = typename base::elm_pointer;

		base_image_ptr() {}

		explicit base_image_ptr(const base &oth) :base(oth) {}

		template<typename _Oe>
		base_image_ptr(const base_image_ptr<_Oe> &oth) :base(oth)
		{
			static_assert(std::is_convertible<_Oe *, _Elm *>::value, "element type not compatible!");
			static_assert(sizeof(_Elm) == sizeof(_Oe), "element size is different!");
		}

		base_image_ptr(
			elm_pointer _data,
			size_t _width_step,
			size_t _width,
			size_t _height) :
			base(
				make_array_ptr(_data, _width),
				_height,
				_width_step
				) {}

		elm_type &pixel(size_t x, size_t y) const
		{
			return this->at(y).at(x);
		}

		bool empty() const
		{
			return base::empty();
		}

		const base &to_tensor() const
		{
			return *this;
		}

		size_t width_step() const
		{
			return this->move_step();
		}

		size_t width() const
		{
			return this->begin()->size();
		}

		size_t height() const
		{
			return this->size();
		}

		friend const base_image_ptr & to_ptr(const base_image_ptr & p)
		{
			return p;
		}

		friend base_image_ptr & to_ptr(base_image_ptr & p)
		{
			return p;
		}
	};


	template<typename _Elm, size_t _Ch>
	class image_ptr : public base_image_ptr<
		typename std::conditional<
			std::is_const<_Elm>::value,
			const std::array<
				typename std::remove_const<_Elm>::type,
				_Ch
			>,
			std::array<_Elm,_Ch>
		>::type >
	{
	public:
		using base = base_image_ptr<
			typename std::conditional<
			std::is_const<_Elm>::value,
			const std::array<typename std::remove_const<_Elm>::type,_Ch>,
			std::array<_Elm, _Ch>
			>::type >;

		using const_self = image_ptr<const _Elm, _Ch>;
		using normal_self = image_ptr<typename std::remove_const<_Elm>::type, _Ch>;
		using elm_type = typename base::elm_type;
		using channel_type = typename elm_type::value_type;
		using elm_pointer = elm_type *;
		using tensor_type = typename base::base;

		image_ptr() {}

		explicit image_ptr(const tensor_type &oth) :base(oth) {}

		template<typename _Elm_> 
		image_ptr(const image_ptr<_Elm_,_Ch> &oth) :base(oth)
		{
			static_assert(
				is_memory_compatible<elm_type, typename image_ptr<_Elm_, _Ch>::elm_type>::value,
				"element type not compatible!");
		}

		image_ptr(
			channel_type * _data,
			size_t _width_step,
			size_t _width,
			size_t _height) :
			base((elm_pointer)_data,_width_step,_width,_height){}

		image_ptr(
			channel_type * _data,
			size_t _width_step,
			image_size _size) :
			base((elm_pointer)_data, _width_step, _size.width, _size.height) {}

		constexpr size_t depth() const
		{
			return sizeof(_Elm) << 3;
		}

		constexpr size_t elm_size() const
		{
			return sizeof(_Elm);
		}

		constexpr size_t channel_size() const
		{
			return _Ch;
		}

		auto to_tensor_3d() const
		{
			return make_tensor_ptr(
				make_tensor_ptr(
					make_tensor_ptr(
						&(*this->data())[0],
						_Ch
					),
					this->width()
				),
				this->height(),
				this->width_step()
			);
		}

		void fill(channel_type _v, int _channel) const
		{
			this->for_each(
				[_v,_channel](typename tensor_type::elm_type &p)
			{
				p[_channel] = _v;
			});
		}

		image_ptr clip(image_rectangle r) const
		{
			auto h = clip_at<0>(*(tensor_type *)this, r.pos.y, r.pos.y + r.size.height);
			return image_ptr(clip_at<1>(h, r.pos.x, r.pos.x + r.size.width));
		}

		image_ptr clip(pos_t _x, pos_t _y, size_t _width, size_t _height) const
		{
			return clip(image_rectangle{ _x,_y,_width,_height });
		}
	};

	template<typename _Elm>
	class image_ptr<_Elm,1> : public base_image_ptr<_Elm>
	{
	public:
		using base = base_image_ptr<_Elm>;
		using const_self = image_ptr<const _Elm, 1>;
		using normal_self = image_ptr<typename std::remove_const<_Elm>::type, 1>;
		using channel_type = _Elm;
		using elm_type = typename base::elm_type;
		using elm_pointer = elm_type * ;
		using tensor_type = typename base::base;

		image_ptr() {}

		explicit image_ptr(const tensor_type &oth) :base(oth) {}

		template<typename _Elm_>
		image_ptr(const image_ptr<_Elm_, 1> &oth) :base(oth)
		{
			static_assert(
				is_memory_compatible<_Elm, _Elm_>::value,
				"element type not compatible!");
		}

		image_ptr(
			elm_pointer _data,
			size_t _width_step,
			size_t _width,
			size_t _height) :
			base(_data, _width_step, _width, _height) {}


		image_ptr(
			elm_pointer _data,
			size_t _width_step,
			image_size _size) :
			base(_data, _width_step, _size.width, _size.height) {}

		constexpr size_t depth() const
		{
			return sizeof(_Elm) << 3;
		}

		constexpr size_t elm_size() const
		{
			return sizeof(_Elm);
		}

		constexpr size_t channel_size() const
		{
			return 1;
		}

		auto to_tensor_3d() const
		{
			return make_tensor_ptr(
				make_tensor_ptr(
					make_tensor_ptr(
						&(*this->data())[0],
						1
					),
					this->width()
				),
				this->height(),
				this->width_step()
			);
		}

		image_ptr clip(image_rectangle r) const
		{
			return image_ptr(tensor_type::clip(
				r.pos.y,
				r.pos.y + r.size.height,
				r.pos.x,
				r.pos.x + r.size.width));
		}

		image_ptr clip(pos_t _x, pos_t _y, size_t _width, size_t _height) const
		{
			return clip(image_rectangle{ _x,_y,_width,_height });
		}
	};

	template<typename _Elm, size_t _Ch, class Alloc>
	class image
	{
		static_assert(!std::is_same<_Elm, bool>::value, "It is bool...");
	public:
		using image_type = image_ptr<_Elm, _Ch>;
		using data_type = std::vector<_Elm, Alloc>;

		using value_type = typename image_type::value_type;
		using const_value_type = typename image_type::value_type::const_self;
		using elm_type = typename image_type::elm_type;

		using iterator = typename image_type::iterator;
		using const_iterator = typename image_type::const_iterator;

		using reverse_iterator = typename image_type::reverse_iterator;
		using const_reverse_iterator = typename image_type::const_reverse_iterator;

		using difference_type = typename image_type::difference_type;
		using size_type = typename image_type::size_type;

	private:

		data_type _data;
		image_type _ptr;

	public:
		image() {}

		image(image&& oth):
			_data(std::move(oth._data)),
			_ptr(oth._ptr)
		{
			oth._ptr = image_type();
		}
		image& operator = (image&& oth)
		{
			if (this == &oth) return *this;

			_data = std::move(oth._data);
			_ptr = oth._ptr;

			oth._ptr = image_type();

			return *this;
		}

		image(const image& oth):
			_data(oth._data),
			_ptr(
				&_data[0],
				oth.width() * sizeof(typename image_type::elm_type),
				oth.width(),
				oth.height())
		{

		}

		image& operator = (const image& oth)
		{
			if (this == &oth) return *this;

			_data = oth._data;
			_ptr = image_type(
				&_data[0],
				oth.width() * sizeof(typename image_type::elm_type),
				oth.width(),
				oth.height());

			return *this;
		}
		

		image(data_type && __data, image_type ptr) :
			_data(std::move(__data)),
			_ptr(ptr)
		{
			if (ptr.front().data() < (elm_type *)&_data[0] || ptr.back().data() > (elm_type *)&_data[0] + _data.size())
			{
				__on_bear_exception(
					exception_type::pointer_outof_range,
					"image_ptr must pointed to the vector!");
			}
		}

		image(size_t _width, size_t _height) :
			_data(_width * _height * _Ch),
			_ptr(
				&_data[0],
				_width * sizeof(typename image_type::elm_type),
				_width,
				_height)
		{
		}

		image(
			image_size _size) :
			_data(_size.width * _size.height * _Ch),
			_ptr(
				&_data[0],
				_size.width * sizeof(typename image_type::elm_type),
				_size.width,
				_size.height)
		{
		}

		image &operator = (const image_type& oth)
		{
			if (_ptr.width() != oth.width() ||
				_ptr.height != oth.height())
			{
				resize_canvas(oth.size());
			}

			copy(_ptr, oth);
		}

		bool empty() const
		{
			return _ptr.empty();
		}

		void resize_canvas(image_size _size)
		{
			if (bear::size(_ptr) == _size)return;

			_data.resize(_size.width * _size.height * _Ch);

			_ptr = image_type(
				&_data[0],
				_size.width * sizeof(typename image_type::elm_type),
				_size.width,
				_size.height
			);
		}

		void resize_canvas(size_t _width, size_t _height)
		{
			resize_canvas(image_size(_width, _height));
		}

		auto &pixel(size_t x, size_t y)
		{
			return _ptr.pixel(x, y);
		}

		std::pair<std::vector<_Elm, Alloc>, image_type> decompose()
		{
			auto tmp = _ptr;
			_ptr = image_type();
			return std::make_pair(std::move(_data), tmp);
		}

		operator image_type ()
		{
			return _ptr;
		}

		operator typename image_type::const_self () const
		{
			return _ptr;
		}

		image clone() const
		{
			return image(_ptr.width(), _ptr.height());
		}

		image copy_clone() const
		{
			image ret = clone();
			copy(ret._ptr, _ptr);
			return ret;
		}


		size_t width() const
		{
			return _ptr.width();
		}

		size_t height() const
		{
			return _ptr.height();
		}

		size_t size() const
		{
			return _ptr.size();
		}

		friend image_size size(const image & img)
		{
			return image_size{ img.width(), img.height() };
		}

		constexpr size_t depth() const
		{
			return sizeof(_Elm) << 3;
		}

		constexpr size_t elm_size() const
		{
			return sizeof(_Elm);
		}

		constexpr size_t channel_size() const
		{
			return _Ch;
		}

		auto to_tensor()
		{
			return _ptr.to_tensor();
		}

		auto to_tensor_3d()
		{
			return _ptr.to_tensor_3d();
		}

		iterator begin()
		{
			return _ptr.begin();
		}

		iterator end()
		{
			return _ptr.end();
		}


		const_iterator begin() const
		{
			return _ptr.begin();
		}

		const_iterator end() const
		{
			return _ptr.end();
		}


		reverse_iterator rbegin()
		{
			return reverse_iterator(_ptr.rbegin());
		}

		reverse_iterator rend()
		{
			return reverse_iterator(_ptr.rend());
		}


		const_reverse_iterator rbegin() const
		{
			typename image_type::const_self _p = _ptr;
			return const_reverse_iterator(_p.rbegin());
		}

		const_reverse_iterator rend() const
		{
			typename image_type::const_self _p = _ptr;
			return const_reverse_iterator(_p.rend());
		}

		value_type at(size_t i)
		{
			return _ptr.at(i);
		}

		const_value_type at(size_t i) const
		{
			return _ptr.at(i);
		}

		value_type operator[](size_t i)
		{
			return _ptr.at(i);
		}

		const_value_type operator[](size_t i) const
		{
			return _ptr.at(i);
		}

		value_type front()
		{
			return _ptr.front();
		}


		const_value_type front() const
		{
			return _ptr.front();
		}

		value_type back()
		{
			return _ptr.back();
		}

		const_value_type back() const
		{
			return _ptr.back();
		}
	};

	template<typename _Elm, size_t _Dim, typename Alloc>
	inline image_ptr<_Elm, _Dim> to_ptr(image<_Elm, _Dim, Alloc>& img)
	{
		return img;
	}

	template<typename _Elm, size_t _Dim, typename Alloc>
	inline const_image_ptr<_Elm, _Dim> to_ptr(const image<_Elm, _Dim, Alloc>& img)
	{
		return img;
	}

	template<typename _Elm, size_t _Ch, typename Alloc>
	inline image_size size(const image<_Elm, _Ch, Alloc> & ts)
	{
		return image_size{ ts.width(),ts.height() };
	}

	template<typename _Elm, size_t _Ch, typename Alloc>
	inline image<_Elm, _Ch, Alloc> bind_container(std::vector<_Elm, Alloc> &&ctn, const image_ptr<_Elm, _Ch> &ptr)
	{
		return image<_Elm, _Ch, Alloc>(std::move(ctn), ptr);
	}

	template<typename _Elm, size_t _Ch>
	inline image<typename std::decay<_Elm>::type, _Ch,
		std::allocator<typename std::decay<_Elm>::type>>
		make_container(const image_ptr<_Elm, _Ch> &ptr)
	{
		image<typename std::decay<_Elm>::type, _Ch> ret;

		ret = ptr;

		return ret;
	}



	template<typename _Elm, size_t _Ch>
	inline image_size size(const image_ptr<_Elm, _Ch> & img)
	{
		return image_size{ img.width(),img.height() };
	}


	template<typename _Elm, size_t _Ch>
	inline image_size size(const image<_Elm, _Ch> & img)
	{
		return image_size{ img.width(),img.height() };
	}


	template<typename _Base>
	inline auto clip_image(base_tensor_ptr<_Base> t, image_rectangle r)
	{
		auto h = clip_at<0>(t, r.pos.y, r.pos.y + r.size.height);
		return clip_at<1>(h, r.pos.x, r.pos.x + r.size.width);
	}

	template<typename _Elm, size_t _Ch>
	inline auto clip_image(image_ptr<_Elm, _Ch> t, image_rectangle r)
	{
		return t.clip(r);
	}


	template<typename _Elm, size_t _Ch>
	inline auto clip_image(const image<_Elm, _Ch> &t, image_rectangle r)
	{
		return to_ptr(t).clip(r);
	}

	template<typename _Elm, size_t _Ch>
	inline auto clip_image(image<_Elm, _Ch> &t, image_rectangle r)
	{
		return to_ptr(t).clip(r);
	}

	template<typename _Base>
	inline size_t width(base_tensor_ptr<_Base> t)
	{
		return size_at<1>(t);
	}

	template<typename _Elm, size_t _Ch>
	inline size_t width(image_ptr<_Elm, _Ch> t)
	{
		return t.width();
	}

	template<typename _Elm, size_t _Ch>
	inline size_t width(const image<_Elm, _Ch> &t)
	{
		return t.width();
	}

	template<typename _Elm, size_t _Dim>
	inline size_t width(const tensor<_Elm, _Dim> &t)
	{
		return width(to_ptr(t));
	}

	template<typename _Base>
	inline size_t height(base_tensor_ptr<_Base> t)
	{
		return t.size();
	}

	template<typename _Elm, size_t _Ch>
	inline size_t height(image_ptr<_Elm, _Ch> t)
	{
		return t.height();
	}

	template<typename _Elm, size_t _Ch>
	inline size_t height(const image<_Elm, _Ch> &t)
	{
		return t.height();
	}

	template<typename _Elm, size_t _Dim>
	inline size_t height(const tensor<_Elm, _Dim> &t)
	{
		return height(to_ptr(t));
	}

	template<typename _Elm>
	inline size_t channel_size(base_tensor_ptr<array_ptr<_Elm>> t)
	{
		return 1;
	}

	template<typename _Elm>
	inline size_t channel_size(base_tensor_ptr<base_tensor_ptr<array_ptr<_Elm>>> t)
	{
		return size_at<2>(t);
	}

	template<typename _Elm, size_t _Dim>
	inline size_t channel_size(const tensor<_Elm, _Dim> &t)
	{
		return channel_size(to_ptr(t));
	}

	template<typename _Elm, size_t _Ch>
	inline size_t channel_size(image_ptr<_Elm,_Ch> t)
	{
		return t.channel_size();
	}

	template<typename _Elm, size_t _Ch>
	inline size_t channel_size(const image<_Elm, _Ch> &t)
	{
		return t.channel_size();
	}
}

#endif
