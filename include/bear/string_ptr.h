#pragma once

#include "array_ptr.h"
#include <string>
#include <utility>
#include <sstream>

namespace bear
{
	template<typename _Elm, typename _Trt>
	class basic_string_ptr : public array_ptr<_Elm>
	{
	public:

		using traits_type = _Trt;
		using base = array_ptr<_Elm>;

		using normal_self = basic_string_ptr<typename std::remove_const<_Elm>::type,_Trt>;
		using const_self = basic_string_ptr<const _Elm,_Trt>;
		using other_self = typename std::conditional<
			std::is_const<_Elm>::value,
			normal_self,
			const_self>::type;

	private:

		static int _cmp(
			const _Elm *p,
			size_t p_size,
			const _Elm *q,
			size_t q_size)
		{
			size_t ms;
			int sf;

			if (p_size > q_size)
			{
				ms = q_size;
				sf = 1;
			}
			else if (p_size < q_size)
			{
				ms = p_size;
				sf = -1;
			}
			else
			{
				ms = p_size;
				sf = 0;
			}

			int cf = _Trt::compare(p, q, ms);

			if (cf)return cf;
			return sf;
		}


		static int _cmp(
			const _Elm *p,
			size_t p_size,
			const _Elm *q)
		{
			size_t i = p_size;
			for (;;)
			{
				if (!i)
				{
					if (*q)return -1;
					return 0;
				}
				else if (!*q)
				{
					if (i)return 1;
					return 0;
				}

				if (*p < *q)return -1;
				if (*p > *q)return 1;


				++p;
				++q;
				--i;

			}
		}

	public:

		basic_string_ptr() = default;
		basic_string_ptr(const basic_string_ptr &oth) = default;



		template<typename _Oe_>
		basic_string_ptr(const array_ptr<_Oe_> &oth) :base(oth)
		{
			static_assert(
				is_memory_compatible<typename base::elm_type, _Oe_>::value,
				"element type not compatible!");
		}

		template<typename _Iter>
		basic_string_ptr(_Iter _begin, _Iter _end) : base(_begin, _end) {}

		basic_string_ptr(_Elm * _begin, size_t _size) :base(_begin, _size) {}

		basic_string_ptr(_Elm * _cs) : base(_cs, _Trt::length(_cs)){}

		template<typename _Oe, typename _Alc>
		basic_string_ptr(const std::vector<_Oe, _Alc> & _ctn) :base(_ctn) {}

		template<typename _Oe, size_t Size>
		basic_string_ptr(const std::array<_Oe, Size> & _ctn) : base(_ctn) {}

		template<typename _Oe, typename _Trt_>
		basic_string_ptr(const std::basic_string<_Oe, _Trt_> & _ctn) : base(_ctn) {}

		template<typename _Oe, typename _Alc>
		basic_string_ptr(std::vector<_Oe, _Alc> & _ctn) : base(_ctn) {}

		template<typename _Oe, size_t Size>
		basic_string_ptr(std::array<_Oe, Size> & _ctn) : base(_ctn) {}

		template<typename _Oe, typename _Trt_>
		basic_string_ptr(std::basic_string<_Oe, _Trt_> & _ctn) : base(_ctn) {}

		inline auto split(_Elm c) const
		{
			auto bg = base::begin();
			auto pos = _Trt::find(bg, base::size(), c);

			if (!pos)return std::make_pair(*this, basic_string_ptr());

			return std::make_pair(
				basic_string_ptr(bg, pos - bg),
				basic_string_ptr(pos + 1, base::end() - pos - 1));
		}

		std::basic_string<typename std::decay<_Elm>::type, _Trt> to_string() const{
			return std::basic_string<typename std::decay<_Elm>::type, _Trt>(this->begin(), this->end());
		}

		operator std::basic_string<typename std::decay<_Elm>::type, _Trt>() const
		{
			return std::basic_string<typename std::decay<_Elm>::type, _Trt>(this->begin(), this->end());
		}

		friend bool operator == (const basic_string_ptr &ls, const basic_string_ptr &rs)
		{
			return !_cmp(&ls[0], ls.size(), &rs[0], rs.size());
		}

		friend bool operator != (const basic_string_ptr &ls, const basic_string_ptr &rs)
		{
			return _cmp(&ls[0], ls.size(), &rs[0], rs.size());
		}

		friend bool operator > (const basic_string_ptr &ls, const basic_string_ptr &rs)
		{
			return _cmp(&ls[0], ls.size(), &rs[0], rs.size()) > 0;
		}

		friend bool operator < (const basic_string_ptr &ls, const basic_string_ptr &rs)
		{
			return _cmp(&ls[0], ls.size(), &rs[0], rs.size()) < 0;
		}

		friend bool operator >= (const basic_string_ptr &ls, const basic_string_ptr &rs)
		{
			return _cmp(&ls[0], ls.size(), &rs[0], rs.size()) >= 0;
		}

		friend bool operator <= (const basic_string_ptr &ls, const basic_string_ptr &rs)
		{
			return _cmp(&ls[0], ls.size(), &rs[0], rs.size()) <= 0;
		}



		bool operator == (const other_self &rs) const
		{
			return !_cmp(this->data(), this->size(), rs.data(), rs.size());
		}

		bool operator != (const other_self &rs) const
		{
			return _cmp(this->data(), this->size(), rs.data(), rs.size());
		}

		bool operator > (const other_self &rs) const
		{
			return _cmp(this->data(), this->size(), rs.data(), rs.size()) > 0;
		}

		bool operator < (const other_self &rs) const
		{
			return _cmp(this->data(), this->size(), rs.data(), rs.size()) < 0;
		}

		bool operator >= (const other_self &rs) const
		{
			return _cmp(this->data(), this->size(), rs.data(), rs.size()) >= 0;
		}

		bool operator <= (const other_self &rs) const
		{
			return _cmp(this->data(), this->size(), rs.data(), rs.size()) <= 0;
		}





		friend bool operator == (const basic_string_ptr &ls, const std::string &rs)
		{
			return !_cmp(&ls[0], ls.size(), &rs[0], rs.size());
		}

		friend bool operator != (const basic_string_ptr &ls, const std::string &rs)
		{
			return _cmp(&ls[0], ls.size(), &rs[0], rs.size());
		}

		friend bool operator > (const basic_string_ptr &ls, const std::string &rs)
		{
			return _cmp(&ls[0], ls.size(), &rs[0], rs.size()) > 0;
		}

		friend bool operator < (const basic_string_ptr &ls, const std::string &rs)
		{
			return _cmp(&ls[0], ls.size(), &rs[0], rs.size()) < 0;
		}

		friend bool operator >= (const basic_string_ptr &ls, const std::string &rs)
		{
			return _cmp(&ls[0], ls.size(), &rs[0], rs.size()) >= 0;
		}

		friend bool operator <= (const basic_string_ptr &ls, const std::string &rs)
		{
			return _cmp(&ls[0], ls.size(), &rs[0], rs.size()) <= 0;
		}



		friend bool operator == (const  std::string &ls, const basic_string_ptr &rs)
		{
			return !_cmp(&ls[0], ls.size(), &rs[0], rs.size());
		}

		friend bool operator != (const  std::string &ls, const basic_string_ptr &rs)
		{
			return _cmp(&ls[0], ls.size(), &rs[0], rs.size());
		}

		friend bool operator > (const  std::string &ls, const basic_string_ptr &rs)
		{
			return _cmp(&ls[0], ls.size(), &rs[0], rs.size()) > 0;
		}

		friend bool operator < (const  std::string &ls, const basic_string_ptr &rs)
		{
			return _cmp(&ls[0], ls.size(), &rs[0], rs.size()) < 0;
		}

		friend bool operator >= (const  std::string &ls, const basic_string_ptr &rs)
		{
			return _cmp(&ls[0], ls.size(), &rs[0], rs.size()) >= 0;
		}

		friend bool operator <= (const  std::string &ls, const basic_string_ptr &rs)
		{
			return _cmp(&ls[0], ls.size(), &rs[0], rs.size()) <= 0;
		}



		friend bool operator == (const basic_string_ptr &ls, _Elm * rs)
		{
			return !_cmp(&ls[0], ls.size(), rs);
		}

		friend bool operator != (const basic_string_ptr &ls, _Elm * rs)
		{
			return _cmp(&ls[0], ls.size(), rs);
		}

		friend bool operator > (const basic_string_ptr &ls, _Elm * rs)
		{
			return _cmp(&ls[0], ls.size(), rs) > 0;
		}

		friend bool operator < (const basic_string_ptr &ls, _Elm * rs)
		{
			return _cmp(&ls[0], ls.size(), rs) < 0;
		}

		friend bool operator >= (const basic_string_ptr &ls, _Elm * rs)
		{
			return _cmp(&ls[0], ls.size(), rs) >= 0;
		}

		friend bool operator <= (const basic_string_ptr &ls, _Elm * rs)
		{
			return _cmp(&ls[0], ls.size(), rs) <= 0;
		}



		friend bool operator == (_Elm * rs, const basic_string_ptr &ls)
		{
			return !_cmp(&ls[0], ls.size(), rs);
		}

		friend bool operator != (_Elm * rs, const basic_string_ptr &ls)
		{
			return _cmp(&ls[0], ls.size(), rs);
		}

		friend bool operator > (_Elm * rs, const basic_string_ptr &ls)
		{
			return _cmp(&ls[0], ls.size(), rs) < 0;
		}

		friend bool operator < (_Elm * rs, const basic_string_ptr &ls)
		{
			return _cmp(&ls[0], ls.size(), rs) > 0;
		}

		friend bool operator >= (_Elm * rs, const basic_string_ptr &ls)
		{
			return _cmp(&ls[0], ls.size(), rs) <= 0;
		}

		friend bool operator <= (_Elm * rs, const basic_string_ptr &ls)
		{
			return _cmp(&ls[0], ls.size(), rs) >= 0;
		}

		inline auto clip(size_t start, size_t end)
		{
			if (end > this->size())end = this->size();
			if (start > end)start = end;
			return basic_string_ptr(this->data() + start, end - start);
		}

		friend size_t size(const basic_string_ptr & str)
		{
			return str.size();
		}
	};

	template<typename _Elm, typename _Trt>
	inline std::basic_string<typename std::decay<_Elm>::type, _Trt>
		make_container(const basic_string_ptr<_Elm, _Trt> &str)
	{
		return std::basic_string<typename std::decay<_Elm>::type, _Trt>(str.begin(), str.end());
	}


	template<typename _Elm>
	inline basic_string_ptr<_Elm> to_ptr(_Elm * oth)
	{
		return basic_string_ptr<_Elm>(oth);
	}


	template<typename _Elm, typename _Trt>
	inline basic_string_ptr<_Elm, _Trt> to_ptr(std::basic_string<_Elm, _Trt> &oth)
	{
		return basic_string_ptr<_Elm, _Trt>(oth);
	}

	template<typename _Elm, typename _Trt>
	inline const_basic_string_ptr<_Elm, _Trt> to_ptr(const std::basic_string<_Elm, _Trt> &oth)
	{
		return const_basic_string_ptr<_Elm, _Trt>(oth);
	}


	template<typename _Elm, typename _Trt>
	inline auto _clip(const basic_string_ptr<_Elm, _Trt> & oth, size_t start, size_t end)
	{
		return oth.clip(start, end);
	}

	template<typename _Elm, typename _Trt>
	inline auto split(const basic_string_ptr<_Elm, _Trt> & oth, typename basic_string_ptr<_Elm, _Trt>::elm_type c)
	{
		std::vector<basic_string_ptr<_Elm, _Trt>> ret;

		if (oth.empty())return ret;

		auto pr = oth.split(c);

		for (; !pr.second.empty(); pr = pr.second.split(c))
		{
			ret.push_back(pr.first);
		}

		ret.push_back(pr.first);

		return ret;
	}



	template<typename _Stm, typename _Elm, typename _Trt>
	inline _Stm && operator >> (_Stm && stm, basic_string_ptr<_Elm, _Trt> arr)
	{
		auto e = arr.end();
		for (auto b = arr.begin(); b < e; ++b)
		{
			std::forward<_Stm>(stm) >> *b;
		}
		return std::forward<_Stm>(stm);
	}


	template<typename _Stm, typename _Elm, typename _Trt>
	inline _Stm && operator << (_Stm && stm, basic_string_ptr<_Elm, _Trt> arr)
	{
		auto e = arr.end();
		for (auto b = arr.begin(); b < e; ++b)
		{
			std::forward<_Stm>(stm) << *b;
		}
		return std::forward<_Stm>(stm);
	}

	template<typename _T, typename _Elm, typename _Trt>
	inline _T string_cast(basic_string_ptr<_Elm, _Trt> arr)
	{
		using elm = typename std::remove_const<_Elm>::type;

		std::basic_istringstream<elm, _Trt> ss(arr);

		_T ret;

		ss >> ret;

		return ret;
	}



	template<typename _Elm, typename _Trt, typename _Alc>
	inline std::basic_string<_Elm,_Trt,_Alc> operator + (std::basic_string<_Elm, _Trt, _Alc> &&s, basic_string_ptr<_Elm, _Trt> arr)
	{
		s.append(arr.data(), arr.size());
		return std::move(s);
	}

	template<typename _Elm, typename _Trt, typename _Alc>
	inline std::basic_string<_Elm, _Trt, _Alc> operator + (const std::basic_string<_Elm, _Trt, _Alc> &_s, basic_string_ptr<_Elm, _Trt> arr)
	{
		auto s = _s;
		s.append(arr.data(), arr.size());
		return s;
	}
}