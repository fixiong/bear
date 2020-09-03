#pragma once

#include<type_traits>
#include<utility>
#include"bear_exception.h"

namespace bear
{
	class _base_functor_noexcept
	{
	};

	class _base_functor_default
	{
	};

	class _base_functor_const
	{
	};
	
	template<typename _Traits, typename _Result, typename ... _Types >
	class base_functor;

	template<typename _Fun>
	struct __check_fun
	{
		using type = int;
	};

	template<typename _Result, typename ... _Types >
	struct __check_fun<base_functor<_Result, _Types ...> > {};

	////////////////// default ///////////////

	template<typename _Traits, typename _Result, typename ... _Types >
	class _base_functor_interface
	{
	public:
		virtual _Result run(_Types... _args) = 0;
		virtual _base_functor_interface* clone() const = 0;
		virtual void release() noexcept = 0;
	protected:
		~_base_functor_interface() {}
	};

	template<typename _Fun, typename _Traits, typename _Result, typename ... _Types >
	class _base_functor_container :public _base_functor_interface<_Traits, _Result, _Types ...>
	{
		_Fun _fun;
	public:

		template<typename R_Fun>
		_base_functor_container(R_Fun&& _f) :_fun(std::forward<R_Fun>(_f)) {}

		_Result run(_Types... _args)
		{
			return _fun(static_cast<_Types>(_args)...);
		}

		_base_functor_interface<_Traits, _Result, _Types ...>* clone() const
		{
			return new _base_functor_container(_fun);
		}

		void release() noexcept
		{
			delete this;
		}
	private:
		~_base_functor_container() {}
	};

	////////////////////////////////////////////


	////////////////// noexcept ///////////////

	template<typename _Result, typename ... _Types >
	class _base_functor_interface<_base_functor_noexcept, _Result, _Types ...>
	{
	public:
		virtual _Result run(_Types... _args) noexcept = 0;
		virtual _base_functor_interface* clone() const = 0;
		virtual void release() noexcept = 0;
	protected:
		~_base_functor_interface() {}
	};

	template<typename _Fun, typename _Result, typename ... _Types >
	class _base_functor_container<_Fun, _base_functor_noexcept, _Result, _Types ...>
		:public _base_functor_interface<_base_functor_noexcept, _Result, _Types ...>
	{
		_Fun _fun;
	public:

		template<typename R_Fun>
		_base_functor_container(R_Fun&& _f) :_fun(std::forward<R_Fun>(_f)) {}

		_Result run(_Types... _args) noexcept
		{
			return _fun(static_cast<_Types>(_args)...);
		}

		_base_functor_interface<_base_functor_noexcept, _Result, _Types ...>* clone() const
		{
			return new _base_functor_container(_fun);
		}

		void release() noexcept
		{
			delete this;
		}
	private:
		~_base_functor_container() {}
	};

	////////////////////////////////////////////


	////////////////// const ///////////////

	template<typename _Result, typename ... _Types >
	class _base_functor_interface<_base_functor_const, _Result, _Types ...>
	{
	public:
		virtual _Result run(_Types... _args) const = 0;
		virtual _base_functor_interface* clone() const = 0;
		virtual void release() noexcept = 0;
	protected:
		~_base_functor_interface() {}
	};

	template<typename _Fun, typename _Result, typename ... _Types >
	class _base_functor_container<_Fun, _base_functor_const, _Result, _Types ...>
		:public _base_functor_interface<_base_functor_const, _Result, _Types ...>
	{
		_Fun _fun;
	public:

		template<typename R_Fun>
		_base_functor_container(R_Fun&& _f) :_fun(std::forward<R_Fun>(_f)) {}

		_Result run(_Types... _args) const
		{
			return _fun(static_cast<_Types>(_args)...);
		}

		_base_functor_interface<_base_functor_const, _Result, _Types ...>* clone() const
		{
			return new _base_functor_container(_fun);
		}

		void release() noexcept
		{
			delete this;
		}
	private:
		~_base_functor_container() {}
	};

	////////////////////////////////////////////

	template<typename _Traits, typename _Result, typename ... _Types >
	class base_functor
	{
	private:

		using _interface = _base_functor_interface<_Traits, _Result, _Types ...>;

		template<typename _Fun>
		using _container = _base_functor_container<_Fun, _Traits, _Result, _Types ...>;

		_interface * _ctn;

		_interface * _get_ctn()
		{
			return _ctn;
		}

		const _interface * _get_ctn() const
		{
			return _ctn;
		}

	public:
		_Result operator()(_Types... _args) const
		{
			return _get_ctn()->run(static_cast<_Types>(_args)...);
		}

		_Result operator()(_Types... _args)
		{
			if (_ctn == nullptr)
			{
				throw bear_exception(exception_type::other_error, "call a empty base_functor!");
			}
			return _get_ctn()->run(static_cast<_Types>(_args)...);
		}

		template<typename R_Fun>
		base_functor(R_Fun &&_fun, typename __check_fun<typename std::decay<R_Fun>::type>::type _c = 0)
		{
			using _Fun = typename std::decay<R_Fun>::type;
			static_assert(!std::is_same<_Fun, base_functor>::value, "recursive create");
			_ctn = new _container<_Fun>(std::forward<R_Fun>(_fun));
		}

		base_functor() : _ctn(0) {}

		base_functor(const base_functor &other) : _ctn(other._ctn ? other._ctn->clone() : 0) {}

		base_functor &operator = (const base_functor &other)
		{
			if (this == &other)return *this;

			if (_ctn)_ctn->release();

			if (other._ctn)
			{
				_ctn = other._ctn->clone();
			}
			else
			{
				_ctn = 0;
			}

			return *this;
		}

		base_functor(base_functor &&other) : _ctn(other._ctn)
		{
			other._ctn = 0;
		}

		base_functor &operator = (base_functor &&other) noexcept
		{
			if (this == &other)return *this;

			if (_ctn)_ctn->release();

			_ctn = other._ctn;
			other._ctn = 0;

			return *this;
		}

		operator bool() const
		{
			return _ctn;
		}

		~base_functor()
		{
			if (_ctn)_ctn->release();
		}
	};

	template<typename _Result, typename ... _Types >
	using functor = base_functor<_base_functor_default, _Result, _Types ...>;

	template<typename _Result, typename ... _Types >
	using noexcept_functor = base_functor<_base_functor_noexcept, _Result, _Types ...>;

	template<typename _Result, typename ... _Types >
	using const_functor = base_functor<_base_functor_default, _Result, _Types ...>;



	template<typename _Result, typename ... _Types >
	class functor_ptr
	{
		void * _fun;
		_Result (*_invoker)(void * _f, _Types... _args);
	private:

		template<typename _Fun>
		static _Result _invoker_type(void * _f, _Types... _args)
		{
			return (*(_Fun *)_f)(static_cast<_Types>(_args)...);
		}

		template<typename _Type>
		struct check_t
		{
			static _Type run();
		};

		int check_f(_Result r);

	public:
		_Result operator()(_Types... _args) const
		{
			return _invoker(_fun, static_cast<_Types>(_args)...);
		}

		template<typename _Fun>
		functor_ptr(_Fun * p) : _fun(p), _invoker(_invoker_type<_Fun>)
		{
			using _ts = decltype(check_f((*p)(check_t<_Types>::run() ...)));
		}

		functor_ptr() : _fun(0), _invoker(0) {}

		operator bool()
		{
			return _fun;
		}
	};

	class defer
	{
		noexcept_functor<void> _fun;

	public:
		template<typename Fun>
		explicit defer(Fun&& fun) : _fun(std::forward<Fun>(fun)) {}

		defer() {}
		defer(const defer& other) = delete;
		defer& operator = (const defer& other) = delete;

		defer(defer&& other) = default;
		defer& operator = (defer&& other) = default;

		~defer()
		{
			if (_fun)
			{
				_fun();
			}
		}

		operator bool()
		{
			return _fun;
		}
	};
}