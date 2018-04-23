#pragma once

#include "config.h"

#ifdef _DEBUG
#define BOOST_SPIRIT_X3_DEBUG
#endif

#include <boost/variant.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/spirit/home/x3.hpp>
#include <boost/spirit/include/phoenix.hpp>

#include <iomanip>
#include <sstream>
#include <initializer_list>
#include <vector>
#include <exception>

namespace json17
{
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

			array_t(std::initializer_list<object_t::value_type> il)
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
		template <typename U, typename std::enable_if<
			std::is_same<config::null_t, U>::value or
			std::is_same<config::boolean_t, U>::value or
			std::is_same<config::numeric_t, U>::value or
			std::is_same<config::unsigned_t, U>::value or
			std::is_same<config::float_t, U>::value>::type* = 0>
		object_t(config::string_t path, U&& val)
		{
			add(path, value_type(std::forward<U>(val)));
		}

		// { "foo" : "some_string" }
		template <typename U, typename std::enable_if<std::is_constructible<config::string_t, U>::value>::type* = 0>
		object_t(config::string_t path, U&& val)
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

	namespace reader
	{
		namespace x3 = boost::spirit::x3;
		namespace phx = boost::phoenix;
		namespace fusion = boost::fusion;

		struct true_false_t : x3::symbols<boolean_t>
		{
			true_false_t()
			{
				add("true", true);
				add("false", false);
			}
		};

		static true_false_t r_boolean;
		static x3::rule<struct value_, object_t::value_type> r_value{"value_t"};
		static x3::rule<struct object_t_, object_t> r_object{"object_t"};
		static x3::rule<struct array_t_, array_t> r_array{ "array_t" };

		static auto r_string = x3::lexeme['"' >> *x3::char_ >> '"'];
		static auto r_array_def = '[' >> -(r_value % ',') >> ']';

		static auto r_numeric = x3::int_parser<numeric_t>{};
		static auto r_unsigned = x3::uint_parser<unsigned_t>{};
		static auto r_float = x3::real_parser<float_t>{};
		static auto r_number = x3::lexeme[r_numeric | r_unsigned >> !x3::char_(".e0-9")] | r_float;

		static auto r_value_def = r_boolean | r_number | r_string;// r_boolean | r_number | r_string | r_array | r_object;

		static auto create_object = [](auto &ctx)
		{
			auto attr = x3::_attr(ctx);
			phx::bind(&object_t::add, x3::_val(ctx), fusion::at_c<0>(attr), fusion::at_c<1>(attr));
		};
		static auto r_object_def = ('{' >> r_string >> ':' >> r_value >> '}')[create_object];

		BOOST_SPIRIT_DEFINE(r_value, r_object, r_array);
	}


	inline object_t from_string(const std::string& input)
	{
		namespace x3 = boost::spirit::x3;

		auto f = input.begin(), l = input.end();
		object_t obj;
		bool ok = x3::phrase_parse(f, l, reader::r_object, x3::space, obj);
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
