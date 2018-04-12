#pragma once

#include "config.h"

#include <boost/variant.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/spirit/include/qi.hpp>

#include <iomanip>
#include <sstream>
#include <initializer_list>
#include <vector>
#include <exception>

namespace json17
{
	//////////////////////////////////////////////////////////////////////////

	namespace qi = boost::spirit::qi;

	//////////////////////////////////////////////////////////////////////////

	struct object_t //<name> : <value>
	{
		struct array_t;
		using value_type = boost::variant<
			config::null_t,
			config::boolean_t, config::numeric_t, config::unsigned_t, config::float_t, config::string_t,
			boost::recursive_wrapper<object_t>,
			boost::recursive_wrapper<array_t>>;

		struct array_t
		{
			using array_type = config::array_t<value_type>;
			array_type _values;
		
			array_t() = default;

			template <typename...Args>
			array_t(Args...args)
				: _values(std::initializer_list<value_type>{value_type(args)...})
			{
			}

			template <typename T>
			array_t(std::initializer_list<T> il)
				: _values(std::begin(il), std::end(il))
			{
			}

			array_t(std::initializer_list<value_type> il)
				: _values(il)
			{
			}

			template <typename T>
			value_type& operator[](T&& key)
			{
				return _values.at(std::forward<T>(key));
			}

			template <typename T>
			const value_type& operator[](T&& key) const
			{
				return _values.at(std::forward<T>(key));
			}

			bool operator==(const array_t& other) const
			{
				return _values == other._values;
			}

			// friends
			friend decltype(auto) begin(const array_t& arr) { return std::begin(arr._values); }
			friend decltype(auto) end(const array_t& arr) {	return std::end(arr._values);	}
			friend decltype(auto) begin(array_t& arr) { return std::begin(arr._values); }
			friend decltype(auto) end(array_t& arr) { return std::end(arr._values); }
		};

		using holder_type = config::map_t<config::string_t, value_type>;

		object_t() = default;

		// { "foo" : true }
		template <typename U>
		object_t(config::string_t path, U&& val,
				 typename std::enable_if<
				 std::is_same<config::null_t, U>::value or
				 std::is_same<config::boolean_t, U>::value or
				 std::is_same<config::numeric_t, U>::value or
				 std::is_same<config::unsigned_t, U>::value or
				 std::is_same<config::float_t, U>::value>::type* = 0)
		{
			add(path, value_type(std::forward<U>(val)));
		}

		// { "foo" : "some_string" }
		template <typename U>
		object_t(config::string_t path, U&& val, 
				 typename std::enable_if<
				 std::is_constructible<config::string_t, U>::value>::type* = 0)
		{
			add(path, value_type(config::string_t(std::forward<U>(val))));
		}

		// {  "foo" : [1, 2, 3] }
		template <typename T>
		object_t(config::string_t path, std::initializer_list<T> il)
		{
			add(path, value_type(array_t(il)));
		}

		// { "foo" : { "bar" : 10 } }
		object_t(config::string_t path, object_t obj)
		{
			add(path, std::move(obj));
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

		void add(config::string_t key, value_type val)
		{
			_pairs.emplace(key, val);
		}

		value_type operator[](const config::string_t& path) const
		{
			const auto path_it = _pairs.find(path);
			if (path_it == _pairs.end())
			{
				return config::null_t{};
			}
			return path_it->second;
		}

		value_type& operator[](const config::string_t& path)
		{
			const auto path_it = _pairs.find(path);
			if (path_it == _pairs.end())
			{
				_pairs[path] = value_type{};
				return _pairs[path];
			};
			return path_it->second;
		}

		template <typename R = object_t>
		R get_value(const config::string_t& path, R def = R{}) const
		{
			try
			{
				std::vector<config::string_t> v_path;
				boost::split(v_path, path, boost::is_any_of(config::string_t(".")));
				if (v_path.size() == 1)
				{
					return boost::get<R>(operator[](path));
				}
				else
				{
					auto val = operator[](*std::begin(v_path));
					v_path.erase(std::begin(v_path));
					return boost::get<object_t>(val).get_value<R>(boost::join(v_path, config::string_t(".")), def);
				}
			}
			catch (std::exception&)
			{
				return def;
			}
		}

	private:
		holder_type _pairs;
	};

	//////////////////////////////////////////////////////////////////////////

	using null_t = config::null_t;
	using boolean_t = config::boolean_t;
	using numeric_t = config::numeric_t;
	using unsigned_t = config::unsigned_t;
	using float_t = config::float_t;
	using string_t = config::string_t;
	using array_t = object_t::array_t;

	//////////////////////////////////////////////////////////////////////////

	struct json
	{

		// 		json(std::initializer_list<T> il)
		// 		{
		// 
		// 		}
	};

	//////////////////////////////////////////////////////////////////////////

	template <typename Iterator, typename Skipper>
	struct json_reader : qi::grammar<Iterator, object_t(), Skipper>
	{
		json_reader() : json_reader::base_type(_object)
		{
			//_array = qi::lit('[') >> -(_value % ',') >> qi::lit(']');
			_value = qi::bool_ | qi::int_ | qi::uint_ | qi::double_ | *qi::char_("a-zA-Z0-9");// | _object | _array;
			_key_value = qi::lit("\"") >> *qi::char_("a-zA-Z0-9") >> qi::lit("\"") >> qi::lit(":") >> _value;
			_object = qi::lit("{") >> -(_key_value % ",") >> qi::lit("}");

			BOOST_SPIRIT_DEBUG_NODE(_value);
			BOOST_SPIRIT_DEBUG_NODE(_key_value);
		}

	private:
		//qi::rule<Iterator, array_t(), Skipper> _array;
		qi::rule<Iterator, object_t::value_t(), Skipper> _value;
		qi::rule<Iterator, object_t(), Skipper> _key_value, _object;
	};

	object_t from_string(const std::string& input)
	{
		json_reader<std::string::const_iterator, qi::ascii::space_type> g{};
		auto f = input.begin(), l = input.end();
		object_t obj;
		bool ok = qi::phrase_parse(f, l, g, qi::ascii::space, obj);
		if (ok)
		{
			return obj;
		}
		else
		{
			std::stringstream ss;
			ss << "failed to parse, remaining : [" << std::quoted(std::string{ f, l }) << "]";
			throw std::runtime_error(ss.str());
		}
	}

	//////////////////////////////////////////////////////////////////////////
}
