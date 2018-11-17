#pragma once

#include "container_pointer.h"
#include "ptr_algorism.h"


namespace bear
{

	template<typename _T>
	_T & cast_memory(array_ptr<char> mem)
	{
		if (sizeof(_T) != mem.size())
			throw bear_exception(exception_type::size_different,
				"cast memory to a type with different size!");

		return *(_T *)mem.data();
	}

	template<typename _T>
	const _T & cast_memory(const_array_ptr<char> mem)
	{
		if (sizeof(_T) != mem.size())
			throw bear_exception(exception_type::size_different,
				"cast memory to a type with different size!");

		return *(const _T *)mem.data();

	}


	template<typename _T>
	auto cast_to_memory(_T & mem)
	{
		using elm = typename std::conditional<
			std::is_const<_T>::value,
			const char,
			char>::type;

		return array_ptr<elm>((elm *)&mem, sizeof(typename std::decay<_T>::type));

	}

	template<typename _Ptr, typename _Ot, typename _St, typename _Dt>
	class basic_memory_entity
	{
	public:
		using size_type = _St;
		using difference_type = _Dt;
		using ptr_type = _Ptr;
		using const_ptr_type = typename ptr_type::const_self;
		using output_type = _Ot;
		using container = std::pair<
			basic_memory_entity,
			std::unique_ptr<typename ptr_type::value_type[]>>;

	protected:
		ptr_type _ptr;

		size_type &_get_size(size_type i) const
		{
			return cast_memory<size_type>(clip(_ptr, i * sizeof(size_type), (i + 1) * sizeof(size_type)));
		}

	public:

		basic_memory_entity() = default;

		basic_memory_entity(const ptr_type & ptr) : _ptr(ptr){}

		constexpr static size_type index_size(size_t s)
		{
			return (size_type)s * sizeof(size_type);
		}

		template<typename _T>
		static auto allocate_size(const _T &sizes)
		{
			size_type sz = 0;

			std::for_each(sizes.begin(), sizes.end(), [&sz](const_ptr_type p)
			{
				sz += (size_type)p.size();
			});

			return ptr_type(0,sz + index_size(sizes.size()));
		}


		template<typename _T>
		void init(const _T &sizes) const
		{
			if (sizes.empty())return;
			_get_size(0) = (size_type)sizes.size() * sizeof(size_type);

			for (size_type i = 1; i < (size_type)sizes.size(); ++i)
			{
				_get_size(i) = _get_size(i - 1) + (size_type)sizes[i - 1].size();
			}

			if (sizes.back().size() != back().size())
				throw bear_exception(exception_type::size_different, "total size is different to the available memory!");
		}

		template<typename _T>
		static container allocate(const _T &sizes)
		{
			auto sz = allocate_size(sizes).size();
			std::unique_ptr<typename ptr_type::value_type[]> mem(
				new typename ptr_type::value_type[sz]);

			basic_memory_entity me(ptr_type(mem.get(), sz));

			me.init(sizes);

			return make_pair(me, std::move(mem));
		}

		template<typename _T>
		void copy(const _T &sizes) const
		{
			if(size() != sizes.size())
				throw bear_exception(exception_type::size_different, "the size is different to the copy destination!");

			for (size_type i = 0; i < (size_type)sizes.size(); ++i)
			{
				bear::copy(at(i), sizes[i]);
			}
		}

		template<typename _T>
		void init_copy(const _T &sizes) const
		{
			init(sizes);
			copy(sizes);
		}

		template<typename ... _T>
		void init_size(size_t s1, _T ... sss) const
		{
			auto ss = pack_to_array((size_type)s1, (size_type)sss ...);

			init_size(ss);
		}
		
		void init_size(const_array_ptr<size_type> sizes) const
		{
			if (sizes.empty())return;
			_get_size(0) = (size_type)sizes.size() * sizeof(size_type);

			for (size_type i = 1; i < (size_type)sizes.size(); ++i)
			{
				_get_size(i) = _get_size(i - 1) + sizes[i - 1];
			}

			if (sizes.back() != back().size())
				throw bear_exception(exception_type::size_different, "total size is different to the available memory!");
		}

		bool validate() const
		{
			if (_ptr.empty())return true;
			if (_ptr.size() < 4)return false;
			size_type data_start = _get_size(0);

			if ((data_start & 3) || data_start > _ptr.size())return false;

			size_type sz = data_start / sizeof(size_type);

			for (size_type i = 1; i < sz; ++i)
			{
				size_type data_end = _get_size(i);

				if (data_end < data_start || data_end > _ptr.size())return false;

				data_start = data_end;
			}

			return true;
		}

		const ptr_type &to_memory() const
		{
			return _ptr;
		}

		bool empty() const
		{
			return _ptr.empty();
		}

		size_type size() const
		{
			if (_ptr.empty())return 0;
			return _get_size(0) / sizeof(size_type);
		}

		output_type back() const
		{
			return output_type(clip(_ptr, _get_size(size() - 1), (size_type)_ptr.size()));
		}

		output_type at(size_type i) const
		{
			if (size() <= i + 1)return back();
			return output_type(clip(_ptr,_get_size(i),_get_size(i + 1)));
		}

		output_type operator [](size_type i) const
		{
			return at(i);
		}

		class iterator :public std::iterator<std::random_access_iterator_tag, difference_type>
		{
			basic_memory_entity _self;
			//ptr_type _current;
			difference_type _pos;
		public:
			iterator(const basic_memory_entity &self) :_self(self),_pos(0) {}
			iterator() = default;

			output_type operator *() const
			{
				return output_type(_self[_pos]);
			}

			//const ptr_type * operator -> () const
			//{
			//	_current = _self[_pos];
			//	return &_current;
			//}

			output_type operator [] (difference_type i) const
			{
				return output_type(_self[_pos + i]);
			}

			iterator& operator ++ ()
			{
				++_pos;
				return *this;
			}

			iterator operator ++ (int)
			{
				auto ret = *this;
				++_pos;
				return ret;
			}

			iterator& operator -- ()
			{
				--_pos;
				return *this;
			}

			iterator operator -- (int)
			{
				auto ret = *this;
				--_pos;
				return ret;
			}

			iterator &operator += (difference_type rv)
			{
				_pos += rv;;
				return *this;
			}

			iterator &operator -= (difference_type rv)
			{
				_pos -= rv;
				return *this;
			}

			friend iterator operator + (const iterator & lv, difference_type rv)
			{
				auto ret = lv;
				ret._pos += rv;
				return ret;
			}

			friend iterator operator - (const iterator & lv, difference_type rv)
			{
				auto ret = lv;
				ret._pos -= rv;
				return ret;
			}

			friend difference_type operator - (const iterator &  rv, const iterator & lv)
			{
				return rv._pos - lv._pos;
			}

			bool operator == (const iterator &other) const
			{
				return _pos == other._pos;
			}

			bool operator != (const iterator &other) const
			{
				return _pos != other._pos;
			}

			bool operator > (const iterator &other) const
			{
				return _pos > other._pos;
			}

			bool operator < (const iterator &other) const
			{
				return _pos < other._pos;
			}

			bool operator >= (const iterator &other) const
			{
				return _pos >= other._pos;
			}

			bool operator <= (const iterator &other) const
			{
				return _pos <= other._pos;
			}
		};


		iterator begin() const
		{
			return iterator(*this);
		}

		iterator end() const
		{
			return begin() + size();
		}
	};

	using memory_entity = basic_memory_entity<array_ptr<char>, array_ptr<char>, unsigned int, int>;

	using const_memory_entity = basic_memory_entity<const_array_ptr<char>, const_array_ptr<char>, unsigned int, int>;

	template<typename _Elm>
	using entity_list = basic_memory_entity<array_ptr<char>, _Elm, unsigned int, int>;

	template<typename _Elm>
	using const_entity_list = basic_memory_entity<const_array_ptr<char>, _Elm, unsigned int, int>;
}
