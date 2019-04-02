#pragma once

namespace bear
{

	class char_bool
	{
		char_bool() = default;

		char_bool(bool oth) :_value(oth) {}
		char_bool(const char &oth) :_value(oth) {}
		char_bool(const unsigned char &oth) :_value(oth) {}

		operator bool()
		{
			return _value;
		}

		operator char()
		{
			return _value;
		}

		operator unsigned char()
		{
			return _value;
		}

		char _value;
	};

}