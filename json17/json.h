#pragma once

#include "config.h"

#include <boost/variant.hpp>
#include <initializer_list>
#include <vector>
#include <exception>

namespace json17
{

	//////////////////////////////////////////////////////////////////////////

	struct object_t //<name> : <value>
	{
		struct value_t
		{
			using value_types = boost::variant<
				config::null_t,
				config::boolean_t, config::numeric_t, config::unsigned_t, config::float_t, config::string_t,
				boost::recursive_wrapper<object_t>,
				config::array_t<value_t>>;

			value_t()
				: _value(config::null_t{})
			{
			}

			template <typename U>
			value_t(U val, typename std::enable_if<
				std::is_same<object_t, U>::value or
				std::is_same<config::null_t, U>::value or
				std::is_same<config::boolean_t, U>::value or
				std::is_same<config::numeric_t, U>::value or
				std::is_same<config::unsigned_t, U>::value or
				std::is_same<config::float_t, U>::value>::type* t = 0)
				: _value(std::move(val))
			{
			}

			template <typename U>
			value_t(U&& val, typename std::enable_if<
				not std::is_same<typename std::remove_reference<U>::type, value_t>::value
				and std::is_convertible<U, config::string_t>::value>::type* = 0)
				: _value(config::string_t(std::forward<U>(val)))
			{
			}

			value_t(std::initializer_list<value_t> il)
				: value_t(std::begin(il), std::end(il))
			{
			}

			template <typename Iterator>
			value_t(Iterator b, Iterator e)
				: _value(config::array_t<value_t>(b, e))
			{
			}

			bool operator==(const value_t& other) const
			{
				return _value == other._value;
			}

			bool operator!=(const value_t& other) const
			{
				return !(*this == other);
			}

			template <typename R>
			operator R() const 
			{
				return get_value<R>();
			}

			template <typename R>
			R get_value(R def = R{}) const
			{
				try
				{
					return boost::get<R>(_value);
				}
				catch (std::exception & )
				{
					return def;
				}
			}

		private:
			value_types _value;
		};

		using holder_t = config::map_t<config::string_t, value_t>;

		object_t() = default;

		// "foo" : true
		template <typename U>
		object_t(config::string_t path, U&& value)
		{
			add(path, value_t(std::forward<U>(value)));
		}

		// "foo" : [1, 2, 3]
		template <typename T>
		object_t(config::string_t path, std::initializer_list<T> il)
		{
			add(path, value_t(std::begin(il), std::end(il)));
		}

		// "foo" : { "bar" : 10 }
		object_t(config::string_t path, object_t obj)
		{
			add(path, value_t(std::move(obj)));
		}

		// { {"foo" : true}, {"bar" : 4} }
 		object_t(std::initializer_list<object_t> il)
 		{
 			std::for_each(std::begin(il), std::end(il), [this](const auto& v) 
			{
				_pairs.insert(std::begin(v._pairs), std::end(v._pairs));
			});
 		}

		bool operator==(const object_t& other) const
		{
			return _pairs == other._pairs;
		}

		bool operator!=(const object_t& other) const
		{
			return !(*this == other);
		}

		void add(config::string_t key, value_t val)
		{
			_pairs.emplace(key, val);
		}

		value_t operator[](const config::string_t& path) const
		{
			return get(path);
		}

		value_t get(const config::string_t& path) const
		{
			const auto path_it = _pairs.find(path);
			if (path_it == _pairs.end())
			{
				return value_t{};
			}
			return path_it->second;
		}

		template <typename R>
		R get_value(const config::string_t& path, R def = R{}) const
		{
			return get(path).get_value<R>(def);
		}

	private:
		holder_t _pairs;
	};

	//////////////////////////////////////////////////////////////////////////

	using null_t = config::null_t;
	using boolean_t = config::boolean_t;
	using numeric_t = config::numeric_t;
	using unsigned_t = config::unsigned_t;
	using float_t = config::float_t;
	using string_t = config::string_t;
	using array_t = config::array_t<object_t::value_t>;

	//////////////////////////////////////////////////////////////////////////

	struct json
	{

		// 		json(std::initializer_list<T> il)
		// 		{
		// 
		// 		}
	};

	//////////////////////////////////////////////////////////////////////////
}
