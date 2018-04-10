#pragma once

#include <boost/mpl/vector.hpp>
#include <boost/blank.hpp>

#include <cstdint>
#include <vector>
#include <map>

namespace json17
{
	//////////////////////////////////////////////////////////////////////////

	namespace mpl = boost::mpl;

	//////////////////////////////////////////////////////////////////////////

	// config struct to declare aliases for default types
	struct config
	{
		//////////////////////////////////////////////////////////////////////////

		using null_t = boost::blank;
		using boolean_t = bool;
		using numeric_t = int32_t;
		using unsigned_t = uint32_t;
		using float_t = double;
		using string_t = std::string;

		template <typename T>
		using array_t = std::vector<T>;

		template <typename K, typename V>
		using map_t = std::map<K, V>;

		//////////////////////////////////////////////////////////////////////////
	};
}
