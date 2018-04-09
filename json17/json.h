#pragma once

#include "config.h"

#include <boost/variant.hpp>
#include <boost/mpl/joint_view.hpp>
#include <boost/mpl/copy.hpp>

#include <initializer_list>
#include <vector>
#include <exception>

namespace json17
{
	//////////////////////////////////////////////////////////////////////////

	namespace mpl = boost::mpl;

	//////////////////////////////////////////////////////////////////////////

	namespace details
	{
		//////////////////////////////////////////////////////////////////////////


		struct value_t
		{
			using allowed_types = mpl::vector<boost::blank, 
											  config::boolean_t, config::numeric_t, config::unsigned_t, config::float_t, config::string_t, 
											  config::array_t<value_t>>;
			using value_types = typename boost::make_variant_over<allowed_types>::type;

			template <typename U>
			value_t(U val, typename std::enable_if<
				std::is_same<U, config::null_t>::value or
 				std::is_same<config::boolean_t, U>::value or
 				std::is_same<config::numeric_t, U>::value or
 				std::is_same<config::unsigned_t, U>::value or
 				std::is_same<config::float_t, U>::value
				>::type* t = 0)
				: _value(std::move(val))
			{
			}

			template <typename U>
			value_t(U&& val, typename std::enable_if<
				not std::is_same<typename std::remove_reference<U>::type, value_t>::value
				and std::is_convertible<U, config::string_t>::value
			>::type* = 0)
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
				return boost::get<R>(_value);
			}

			template <typename R>
			R get() const
			{
				return boost::get<R>(_value);
			}

		private:
			value_types _value{ boost::blank{} };
		};

		struct object_base_t //<name> : <value>
		{
			using value_types = boost::variant<value_t, config::map_t<object_base_t>>;

			// "foo" : {}
			object_base_t()
				: _path(), _obj(config::null_t{})
			{
			}

			// "foo" : true
			template <typename U>
			object_base_t(std::string path, U&& value)
				: _path(std::move(path)), _obj(std::forward<U>(value))
			{
			}

			// "foo" : [1, 2, 3]
			template <typename T>
			object_base_t(std::string path, std::initializer_list<T> il)
				: _path(std::move(path)), _obj(value_t(std::begin(il), std::end(il)))
			{
			}

			// "foo" : { "bar" : 10 }
			object_base_t(std::string path, object_base_t obj)
				: _path(path), _obj(config::map_t<object_base_t>{})
			{
				add(_path, std::move(obj));
			}

			void add(std::string path, object_base_t obj)
			{
				boost::get<config::map_t<object_base_t>>(_obj).emplace(path, obj);
			}

			template <typename R>
			R get(const std::string& path, R def = R{}) const
			{
				if (_path == path)
				{
					try
					{
						value_t v = boost::get<value_t>(_obj);
						return v.get<R>();
					}
					catch (std::exception&)
					{
					}
				}
				return def;
			}

			object_base_t get(const std::string& path, object_base_t def = object_base_t{})
			{
				try
				{
					auto m = boost::get<config::map_t<object_base_t>>(_obj);
					return m.at(path);
				}
				catch (std::exception&)
				{
				}
				return def;
			}

		private:
			std::string _path;
			value_types _obj;
		};

		//////////////////////////////////////////////////////////////////////////
	}

	//////////////////////////////////////////////////////////////////////////

	using null_t = config::null_t;
	using boolean_t = config::boolean_t;
	using numeric_t = config::numeric_t;
	using unsigned_t = config::unsigned_t;
	using float_t = config::float_t;
	using string_t = config::string_t;
	using array_t = config::array_t<details::value_t>;

	using object_t = details::object_base_t;

	struct json
	{

		// 		json(std::initializer_list<T> il)
		// 		{
		// 
		// 		}
	};

	//////////////////////////////////////////////////////////////////////////
}
