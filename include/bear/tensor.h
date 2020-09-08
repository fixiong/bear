#pragma once

#include "container_pointer.h"
#include <utility>

namespace bear
{

	template<typename _Elm, size_t _Dim, typename Alloc>
	class tensor
	{
		static_assert(!std::is_same<_Elm, bool>::value, "It is bool...");
	public:
		using ptr_type = tensor_ptr<_Elm, _Dim>;
		using data_type = std::vector<_Elm, Alloc>;

		using value_type = typename ptr_type::value_type;
		using const_value_type = typename ptr_type::value_type::const_self;
		using elm_type = _Elm;

		using iterator = typename ptr_type::iterator;
		using const_iterator = typename ptr_type::const_iterator;

		using reverse_iterator = typename ptr_type::reverse_iterator;
		using const_reverse_iterator = typename ptr_type::const_reverse_iterator;

		using difference_type = typename ptr_type::difference_type;
		using size_type = typename ptr_type::size_type;
		using deep_size_type = typename ptr_type::deep_size_type;

		static constexpr size_t dim = ptr_type::dim;

	private:
		static size_t _total_size(size_t sz)
		{
			return sz;
		}

		template<typename ... _T >
		static size_t _total_size(size_t sz, _T ... sizes)
		{
			return sz * _total_size(sizes ...);
		}

		data_type _data;
		ptr_type _ptr;

	public:

		tensor(data_type && __data, const ptr_type &ptr) :
			_data(std::move(__data)),
			_ptr(ptr)
		{
			if (ptr.front().data() < &_data[0] || ptr.back().data() > &_data[0] + _data.size())
			{
				__on_bear_exception(
					exception_type::pointer_outof_range,
					"tensor_ptr must pointed to the vector!");
			}
		}

		tensor() {}

		tensor(tensor&& oth) = default;
		tensor& operator = (tensor&& oth) = default;

		tensor(const tensor& oth) :
			_data(oth._data),
			_ptr(&_data[0], bear::size(oth._ptr))
		{
		}

		tensor& operator = (const tensor& oth)
		{
			if (this == &oth) return *this;

			_data = oth._data;
			_ptr = image_type(&_data[0], bear::size(oth._ptr));

			return *this;
		}

		template<typename ... _T>
		tensor(_T ... sizes) :
			_data(_total_size(sizes ...)),
			_ptr(&_data[0], sizes ...)
		{
		}

		tensor(const deep_size_type &sizes) :
			_data(total_size(sizes)),
			_ptr(&_data[0], sizes)
		{
		}

		tensor &operator = (const ptr_type& oth)
		{
			if (bear::size(_ptr) != size(oth))
			{
				resize(size(oth));
			}

			_ptr.copy(oth);
		}

		void resize(const deep_size_type &_size)
		{
			if (bear::size(_ptr) == _size)return;

			_data.resize(total_size(_size));
			_ptr = ptr_type(&_data[0], _size);
		}


		operator ptr_type ()
		{
			return _ptr;
		}

		operator typename ptr_type::const_self() const
		{
			return _ptr;
		}

		std::pair<std::vector<_Elm, Alloc>, ptr_type> decompose()
		{
			auto tmp = _ptr;
			_ptr = ptr_type();
			return std::make_pair(std::move(_data), tmp);
		}

		tensor clone() const
		{
			return tensor(size(_ptr));
		}

		size_t size() const
		{
			return _ptr.size();
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
			typename ptr_type::const_self _p = _ptr;
			return const_reverse_iterator(_p.rbegin());
		}

		const_reverse_iterator rend() const
		{
			typename ptr_type::const_self _p = _ptr;
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
	inline tensor_ptr<_Elm, _Dim> to_ptr(tensor<_Elm, _Dim, Alloc>& img)
	{
		return img;
	}

	template<typename _Elm, size_t _Dim, typename Alloc>
	inline const_tensor_ptr<_Elm, _Dim> to_ptr(const tensor<_Elm, _Dim, Alloc>& img)
	{
		return img;
	}

	template<typename _Elm, size_t _Dim, typename Alloc>
	inline tensor_size<_Dim> size(const tensor<_Elm, _Dim, Alloc> & ts)
	{
		return size(to_ptr(ts));
	}

	template<typename _Elm, typename _Base, typename Alloc>
	inline tensor<
		typename base_tensor_ptr<_Base>::elm_type,
		base_tensor_ptr<_Base>::dim,
		Alloc> bind_container(std::vector<_Elm, Alloc> &&ctn, const base_tensor_ptr<_Base> &ptr)
	{
		return tensor<
			typename base_tensor_ptr<_Base>::elm_type,
			base_tensor_ptr<_Base>::dim,
			Alloc>(std::move(ctn), ptr);
	}

	template<typename _Base>
	inline tensor<
		typename std::decay<typename base_tensor_ptr<_Base>::elm_type>::type,
		base_tensor_ptr<_Base>::dim, std::allocator<
		typename std::decay<typename base_tensor_ptr<_Base>::elm_type>::type>
	> make_container(const base_tensor_ptr<_Base> & oth)
	{
		using _Elm = typename std::decay<typename base_tensor_ptr<_Base>::elm_type>::type;

		tensor<_Elm, base_tensor_ptr<_Base>::dim> ret(size(oth));

		to_ptr(ret).copy(oth);

		return ret;
	}

	template<typename _Elm, typename Alloc, typename ... _Sz>
	inline auto reshape(std::vector<_Elm, Alloc> && oth, _Sz ... _sizes)
	{
		auto ptr = reshape(oth, _sizes ...);
		return bind_container(std::move(oth), ptr);
	}

	template<typename _Elm, typename Alloc, size_t _Dim>
	inline auto reshape(std::vector<_Elm, Alloc> && oth, const tensor_size<_Dim> &sizes)
	{
		auto ptr = reshape(oth, sizes);
		return bind_container(std::move(oth), ptr);
	}




	template<typename _Elm, size_t _Dim, size_t _Aln>
	class aligned_tensor
	{
	public:
		using ptr_type = tensor_ptr<_Elm, _Dim>;

		using value_type = typename ptr_type::value_type;
		using const_value_type = typename ptr_type::value_type::const_self;
		using elm_type = _Elm;

		using iterator = typename ptr_type::iterator;
		using const_iterator = typename ptr_type::const_iterator;

		using reverse_iterator = typename ptr_type::reverse_iterator;
		using const_reverse_iterator = typename ptr_type::const_reverse_iterator;

		using difference_type = typename ptr_type::difference_type;
		using size_type = typename ptr_type::size_type;
		using deep_size_type = typename ptr_type::deep_size_type;

		static constexpr size_t dim = ptr_type::dim;

	private:


		size_t _make_data(const deep_size_type & sz)
		{
			constexpr const size_t line_size = (sz.back() * sizeof(elm_type) + _Aln - 1) / _Aln * _Aln;

			buf = new char[total_size(sz) / sz.back() * line_size + _Aln - 1];

			size_t pd = reinterpret_cast<size_t &>(buf);

			pd = (pd + _Aln - 1) / _Aln * _Aln;

			_data = reinterpret_cast<elm_type * &>(pd);

			return line_size;
		}

		char * buf = 0;
		elm_type * _data = 0;
		ptr_type _ptr;

	public:

		~aligned_tensor()
		{
			delete[] buf;
		}

		aligned_tensor() {}

		template<typename ... _T>
		aligned_tensor(_T ... sizes);

		aligned_tensor(const deep_size_type &sizes);

		aligned_tensor &operator = (const ptr_type& oth)
		{
			if (size(_ptr) != size(oth))
			{
				resize(size(oth));
			}

			copy(_ptr, oth);
		}

		aligned_tensor(const aligned_tensor&) = delete;
		aligned_tensor &operator =(const aligned_tensor&) = delete;

		aligned_tensor(aligned_tensor &&_other);
		aligned_tensor & operator =(aligned_tensor&& _other);

		void resize(const deep_size_type &_size)
		{
			if (size(_ptr) == _size)return;

			//_data.resize(_size.total_size());
			_ptr = ptr_type(&_data[0], _size);
		}


		operator ptr_type ()
		{
			return _ptr;
		}

		operator typename ptr_type::const_self() const
		{
			return _ptr;
		}


		friend const ptr_type &to_ptr(aligned_tensor &img)
		{
			return img._ptr;
		}

		friend typename ptr_type::const_self to_ptr(const aligned_tensor &img)
		{
			return img._ptr;
		}

		aligned_tensor clone() const;

		aligned_tensor copy_clone() const
		{
			aligned_tensor ret = clone();
			copy(ret._ptr, _ptr);
			return ret;
		}

		size_t size() const
		{
			return _ptr.size();
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
			typename ptr_type::const_self _p = _ptr;
			return const_reverse_iterator(_p.rbegin());
		}

		const_reverse_iterator rend() const
		{
			typename ptr_type::const_self _p = _ptr;
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

		value_type & operator[](size_t i)
		{
			return _ptr.at(i);
		}

		const_value_type & operator[](size_t i) const
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
}