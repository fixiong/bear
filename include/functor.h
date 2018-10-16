#pragma once

#include<type_traits>
#include<utility>

namespace bear
{
	
	template<typename _Result, typename ... _Types >
	class functor;

	template<typename _Fun>
	struct __check_fun
	{
		using type = int;
	};

	template<typename _Result, typename ... _Types >
	struct __check_fun<functor<_Result, _Types ...> > {};

	template<typename _Result, typename ... _Types >
	class functor
	{
	private:

		class _interface
		{
		public:
			virtual _Result run(_Types... _args) = 0;
			virtual _interface  * clone() const = 0;
			virtual void release() noexcept = 0;
		protected:
			~_interface() {}
		};

		template<typename _Fun>
		class _container :public _interface
		{
			_Fun _fun;
		public:

			template<typename R_Fun>
			_container(R_Fun && _f) :_fun(std::forward<R_Fun>(_f)) {}

			_Result run(_Types... _args)
			{
				return _fun(static_cast<_Types>(_args)...);
			}

			_interface * clone() const
			{
				return new _container(_fun);
			}

			void release() noexcept
			{
				delete this;
			}
		private:
			~_container() {}
		};

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
		_Result operator()(_Types... _args) const //won't work!
		{
			return _get_ctn()->run(static_cast<_Types>(_args)...);
		}

		_Result operator()(_Types... _args)
		{
			return _get_ctn()->run(static_cast<_Types>(_args)...);
		}

		template<typename R_Fun>
		functor(R_Fun &&_fun, typename __check_fun<typename std::decay<R_Fun>::type>::type _c = 0)
		{
			using _Fun = typename std::decay<R_Fun>::type;
			static_assert(!std::is_same<_Fun, functor>::value, "recursive create");
			_ctn = new _container<_Fun>(std::forward<R_Fun>(_fun));
		}

		functor() : _ctn(0) {}

		functor(const functor &other) : _ctn(other._ctn->clone()) {}

		functor &operator = (const functor &other)
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

		functor(functor &&other) : _ctn(other._ctn)
		{
			other._ctn = 0;
		}

		functor &operator = (functor &&other)
		{
			if (this == &other)return *this;

			if (_ctn)_ctn->release();

			_ctn = other._ctn;
			other._ctn = 0;

			return *this;
		}

		operator bool()
		{
			return _ctn;
		}

		~functor()
		{
			if (_ctn)_ctn->release();
		}
	};



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
}