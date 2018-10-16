#pragma once

#include <atomic>
#include <memory>
#include <type_traits>

namespace bear
{
	template<typename _data, typename _deleter = std::default_delete<_data>>
	class unique_smp_ptr
	{
		_deleter del;
		_data * m;

		void _destroy()
		{
			if (m)
			{
				del(m);
			}
		}

	public:

		unique_smp_ptr() :m(0) {}

		explicit unique_smp_ptr(_data * _m) :m(_m) {}


		template<typename _data_, typename _deleter_>
		unique_smp_ptr(unique_smp_ptr<_data_,_deleter_> && other) :
			m(other.release()),
			del(other.get_deleter()) {}

		~unique_smp_ptr()
		{
			_destroy();
		}

		unique_smp_ptr(unique_smp_ptr &&other) :
			m(other.release())
		{
			other.m = 0;
		}

		unique_smp_ptr &operator = (unique_smp_ptr &&other)
		{
			if (this == &other)return *this;
			_destroy();

			m = other.m;
			other.m = 0;

			return *this;
		}

		_data * release()
		{
			auto tmp = m;
			m = 0;
			return tmp;
		}

		void reset(_data * _m = 0)
		{
			_destroy();
			m = _m;
		}

		void swap(unique_smp_ptr &oth)
		{
			std::swap(m, oth.m);
		}

		_data * get() const
		{
			return m;
		}

		auto get_deleter() const
		{
			return del;
		}

		operator bool() const
		{
			return m;
		}

		_data * operator -> () const
		{
			return m;
		}

		_data &operator * () const
		{
			return *m;
		}
	};


	template<typename _data, bool _multithread = false, typename _deleter = std::default_delete<_data>>
	class shared_smp_ptr
	{
		using Count = typename std::conditional<_multithread, std::atomic<int>, int>::type;

		_deleter del;
		_data * m;
		Count * c;

		void _destroy()
		{
			if (m)
			{
				int _c = (*c)--;
				if (!_c)
				{
					del(m);
					m = 0;
					delete c;
				}
			}
		}

	public:

		shared_smp_ptr() :m(0) {}

		explicit shared_smp_ptr(_data * _m) :m(_m)
		{
			if (m)c = new Count(0);
		}

		~shared_smp_ptr()
		{
			_destroy();
		}

		template<typename _od, typename _dl>
		shared_smp_ptr(unique_smp_ptr<_od, _deleter> && other) :
			m(other.release()),
			del(other.get_deleter())
		{
			if (m)c = new Count(0);
		}

		template<typename _PT, typename _dl>
		shared_smp_ptr(const shared_smp_ptr<_PT, _multithread, _dl> &other) :
			m(other.get()),
			c(other.get_conter()),
			del(other.get_deleter())
		{
			if (m) (*c) += 1;
		}

		shared_smp_ptr(shared_smp_ptr &&other) :
			m(other.m),
			c(other.c)
		{
			other.m = 0;
		}

		shared_smp_ptr(const shared_smp_ptr &other) :
			m(other.m),
			c(other.c)
		{
			if (m) (*c) += 1;
		}

		shared_smp_ptr &operator = (shared_smp_ptr &&other)
		{
			if (this == &other)return *this;

			_destroy();

			m = other.m;
			c = other.c;
			other.m = 0;
			return *this;
		}

		shared_smp_ptr &operator = (const shared_smp_ptr &other)
		{
			if (this == &other)return *this;

			_destroy();

			m = other.m;
			c = other.c;
			if (m) (*c) += 1;

			return *this;
		}

		void reset(_data * _m = 0)
		{
			_destroy();
			m = _m;
			if (m)c = new Count(0);
		}

		void swap(shared_smp_ptr &oth)
		{
			std::swap(m, oth.m);
			std::swap(c, oth.c);
		}

		_data * get() const
		{
			return m;
		}


		Count * get_conter() const
		{
			return c;
		}

		auto get_deleter() const
		{
			return del;
		}

		operator bool() const
		{
			return m;
		}

		_data * operator -> () const
		{
			return m;
		}

		_data &operator * () const
		{
			return *m;
		}
	};

	struct release_deleter
	{
		template<typename _data>
		void operator () (_data * p)
		{
			p->release();
		}
	};

	template<class T>
	using unique_rls_ptr = unique_smp_ptr<T, release_deleter>;

	template<class T, bool MT = false>
	using shared_rls_ptr = shared_smp_ptr<T, MT, release_deleter>;

	template<bool _multithread = false, typename _data, typename _deleter = std::default_delete<_data>>
	auto to_shared(std::unique_ptr<_data, _deleter> && other)
	{
		return shared_smp_ptr<_data, _multithread, _deleter>(std::move(other));
	}

	template<bool _multithread = false, typename _data, typename _deleter = std::default_delete<_data>>
	auto to_shared(unique_smp_ptr<_data, _deleter> && other)
	{
		return shared_smp_ptr<_data, _multithread, _deleter>(std::move(other));
	}

	template<typename _data, typename _deleter>
	void swap(
		unique_smp_ptr<_data, _deleter> &lr,
		unique_smp_ptr<_data, _deleter> &rr)
	{
		lr.swap(rr);
	}

	template<typename _data, bool _multithread, typename _deleter>
	void swap(
		shared_smp_ptr<_data, _multithread, _deleter> &lr,
		shared_smp_ptr<_data, _multithread, _deleter> &rr)
	{
		lr.swap(rr);
	}

	template<typename T, class... _Types>
	unique_smp_ptr<T> make_unique_smp(_Types&&... _Args)
	{
		return unique_smp_ptr<T>(new T(std::forward<_Types>(_Args)...));
	}

	template<typename T, class... _Types>
	shared_smp_ptr<T> make_shared_smp(_Types&&... _Args)
	{
		return shared_smp_ptr<T>(new T(std::forward<_Types>(_Args)...));
	}

	class any_container
	{
	protected:
		~any_container() {}
	public:

		virtual void release() = 0;

		template<typename T, class... _Types>
		inline static auto make_unique(_Types&&... _Args);

		template<typename T, class... _Types>
		inline static auto make_shared(_Types&&... _Args);
	};


	template<typename _T>
	class __real_container :public any_container
	{
		_T data;
	public:

		_T * get()
		{
			return &data;
		}

		void release()
		{
			delete this;
		}

		template<class... _Types>
		__real_container(_Types&&... _Args) :
			data(std::forward<_Types>(_Args)...) {}
	};

	template<typename T, class... _Types>
	inline auto any_container::make_unique(_Types&&... _Args)
	{
		auto ret = new __real_container<T>(std::forward<_Types>(_Args)...);
		return std::make_pair(unique_any(ret), ret->get());
	}

	template<typename T, class... _Types>
	inline auto any_container::make_shared(_Types&&... _Args)
	{
		auto ret = new __real_container<T>(std::forward<_Types>(_Args)...);
		return std::make_pair(shared_any(ret), ret->get());
	}

	using unique_any = unique_rls_ptr<any_container>;
	using shared_any = shared_rls_ptr<any_container>;
}