#pragma once

#include "ptr_types.h"
#include <iterator>

namespace bear
{
	using std::size_t;
	using std::ptrdiff_t;

	template<typename _Ts>
	class array_iterator : public std::iterator<std::random_access_iterator_tag, typename _Ts::difference_type>
	{
	public:

		using value_type = _Ts;
		using difference_type = typename value_type::difference_type;
		using const_self = array_iterator<typename value_type::const_self>;
		using normal_self = array_iterator<typename value_type::normal_self>;
		using elm_type = typename value_type::elm_type;
		using elm_pointer = elm_type * ;

		static constexpr size_t dim = value_type::dim + 1;
		using deep_size_type = tensor_size<dim - 1>;

	private:

		value_type _value;
		difference_type _step = 0;

	public:

		array_iterator() = default;
		array_iterator(const array_iterator &oth) = default;


		explicit array_iterator(const value_type &oth, difference_type step = 0) :
			_value(oth),
			_step(step ? step :_value.move_step() * _value.size())
		{
			if (_value.move_step() * _value.size() > (size_t)(_step >= 0 ? _step : -_step))
				__on_bear_exception(
					exception_type::pointer_outof_range,
					"step must grater than the memory pointed to!");
		}

		template<typename _Oe_>
		array_iterator(const array_iterator<_Oe_> &oth) :
			_value(*oth),
			_step(oth.move_step())
		{
			static_assert(
				is_memory_compatible<elm_type, typename array_iterator<_Oe_>::elm_type>::value,
				"element type not compatible!");
		}


		operator void *() const
		{
			return (void *)_value.data();
		}

		const value_type &operator *() const
		{
			return _value;
		}

		const value_type * operator -> () const
		{
			return &_value;
		}

		value_type operator [] (difference_type i) const
		{
			auto ret = _value;
			ret.move_pointer(i * _step);
			return ret;
		}

		array_iterator& operator ++ ()
		{
			_value.move_pointer(_step);
			return *this;
		}

		array_iterator operator ++ (int)
		{
			auto ret = *this;
			_value.move_pointer(_step);
			return ret;
		}

		array_iterator& operator -- ()
		{
			_value.move_pointer(-_step);
			return *this;
		}

		array_iterator operator -- (int)
		{
			auto ret = *this;
			_value.move_pointer(-_step);
			return ret;
		}

		array_iterator &operator += (difference_type rv)
		{
			_value.move_pointer(rv * _step);
			return *this;
		}

		array_iterator &operator -= (difference_type rv)
		{
			_value.move_pointer(rv * -_step);
			return *this;
		}

		friend array_iterator operator + (const array_iterator & lv, difference_type rv)
		{
			auto ret = lv;
			ret._value.move_pointer(rv * lv._step);
			return ret;
		}

		friend array_iterator operator - (const array_iterator & lv, difference_type rv)
		{
			auto ret = lv;
			ret._value.move_pointer(rv * -lv._step);
			return ret;
		}

		friend difference_type operator - (const array_iterator &  rv, const array_iterator & lv)
		{
			difference_type d = (char *)(void *)rv - (char *)(void *)lv;
			return d / lv._step;
		}

		bool operator == (const array_iterator &other) const
		{
			return (void *)*this == (void *)other;
		}

		bool operator != (const array_iterator &other) const
		{
			return (void *)*this != (void *)other;
		}

		bool operator > (const array_iterator &other) const
		{
			return (void *)*this > (void *)other;
		}

		bool operator < (const array_iterator &other) const
		{
			return (void *)*this < (void *)other;
		}

		bool operator >= (const array_iterator &other) const
		{
			return (void *)*this >= (void *)other;
		}

		bool operator <= (const array_iterator &other) const
		{
			return (void *)*this <= (void *)other;
		}

		bool is_plan() const
		{
			return _value.move_step() * _value.size() == _step * _value.is_plan();
		}

		elm_pointer redirect_pointer(elm_pointer _ptr, difference_type _pos) const
		{
			return elm_pointer((char *)_ptr + _step * _pos);
		}

		difference_type move_step() const
		{
			return _step;
		}

		void move_pointer(difference_type step)
		{
			_value.move_pointer(step);
		}
	};




	template<typename _Ts>
	class reverse_array_iterator : public array_iterator<_Ts>
	{
	public:

		using base = array_iterator<_Ts>;
		using value_type = _Ts;
		using difference_type = typename value_type::difference_type;
		using const_self = reverse_array_iterator<typename value_type::const_self>;
		using normal_self = reverse_array_iterator<typename value_type::normal_self>;

	public:

		reverse_array_iterator() = default;
		reverse_array_iterator(const reverse_array_iterator &oth) = default;

		reverse_array_iterator(const value_type &_v, difference_type _s) :base(_v,-_s){}

		reverse_array_iterator(const array_iterator<_Ts> &oth) :
			base(*oth, -oth.move_step())
		{
			++*this;
		}

		template<typename _Oe_>
		reverse_array_iterator(const reverse_array_iterator<_Oe_> &oth) :
			base(oth){}

		operator void *()
		{
			return ((base *)this)->operator void *();
		}

		bool operator > (const reverse_array_iterator &other) const
		{
			return (void *)*this < (void *)other;
		}

		bool operator < (const reverse_array_iterator &other) const
		{
			return (void *)*this > (void *)other;
		}

		bool operator >= (const reverse_array_iterator &other) const
		{
			return (void *)*this <= (void *)other;
		}

		bool operator <= (const reverse_array_iterator &other) const
		{
			return (void *)*this >= (void *)other;
		}
	};
}