#pragma once

#include <atomic>
#include <memory>
#include <type_traits>

namespace bear
{
	class inplace_deleter {};

	template<typename _data, bool _multithread = false, typename _deleter = inplace_deleter>
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
		shared_smp_ptr(std::unique_ptr<_od, _deleter> && other) :
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


	template<typename _data, bool _multithread>
	class shared_smp_ptr<_data, _multithread, inplace_deleter>
	{
		using Count = typename std::conditional<_multithread, std::atomic<int>, int>::type;

		struct data_t
		{
			template<class... _Types>
			data_t(_Types&&... _Args) :m(std::forward<_Types>(_Args)...), c(0) {}
			_data m;
			Count c;
		};
		data_t * m;

		void _destroy()
		{
			if (m)
			{
				int _c = (m->c)--;
				if (!_c)
				{
					delete m;
					m = 0;
				}
			}
		}

		shared_smp_ptr(data_t * _m) :m(_m) {}
	public:

		shared_smp_ptr() :m(0) {}

		template<class... _Types>
		static shared_smp_ptr create(_Types&&... _Args)
		{
			return shared_smp_ptr(new data_t(std::forward<_Types>(_Args)...));
		}

		~shared_smp_ptr()
		{
			_destroy();
		}

		shared_smp_ptr(const shared_smp_ptr &other) :
			m(other.m)
		{
			if (m) (m->c) += 1;
		}

		shared_smp_ptr(shared_smp_ptr &&other) :
			m(other.m)
		{
			other.m = 0;
		}

		shared_smp_ptr &operator = (shared_smp_ptr &&other)
		{
			if (this == &other)return *this;

			_destroy();

			m = other.m;
			other.m = 0;
			return *this;
		}

		shared_smp_ptr &operator = (const shared_smp_ptr &other)
		{
			if (this == &other)return *this;

			_destroy();

			m = other.m;
			if (m) (m->c) += 1;

			return *this;
		}

		void swap(shared_smp_ptr &oth)
		{
			std::swap(m, oth.m);
		}

		_data * get() const
		{
			return &m->m;
		}


		auto get_deleter() const
		{
			return inplace_deleter();
		}

		operator bool() const
		{
			return m;
		}

		_data * operator -> () const
		{
			return &m->m;
		}

		_data &operator * () const
		{
			return m->m;
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
	using unique_rls_ptr = std::unique_ptr<T, release_deleter>;

	template<class T, bool MT = false>
	using shared_rls_ptr = shared_smp_ptr<T, MT, release_deleter>;

	template<typename _data, bool _multithread, typename _deleter>
	void swap(
		shared_smp_ptr<_data, _multithread, _deleter> &lr,
		shared_smp_ptr<_data, _multithread, _deleter> &rr)
	{
		lr.swap(rr);
	}

	template<typename T, class... _Types>
	auto make_unique_smp(_Types&&... _Args)
	{
		return std::unique_ptr<T>(new T(std::forward<_Types>(_Args)...));
	}

	template<typename T, class... _Types>
	auto make_shared_smp(_Types&&... _Args)
	{
		return shared_smp_ptr<T>::create(std::forward<_Types>(_Args)...);
	}

	template<typename T, class... _Types>
	auto make_shared_mt_smp(_Types&&... _Args)
	{
		return shared_smp_ptr<T,true>::create(std::forward<_Types>(_Args)...);
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

	using unique_any = unique_rls_ptr<any_container>;
	using shared_any = shared_rls_ptr<any_container>;

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
}