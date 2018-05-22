#pragma once

#include "config.h"

#ifdef _DEBUG
#define BOOST_SPIRIT_X3_DEBUG
#endif

#include <boost/variant.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/spirit/home/x3.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/phoenix/bind/bind_member_function.hpp>

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

		struct array_t : config::array_t<object_t::value_type>
		{
			using base_t = config::array_t<value_type>;

			template <typename...Args>
			array_t(Args...args)
				: base_t(std::initializer_list<object_t::value_type>{object_t::value_type(args)...})
			{
			}

			template <typename T>
			array_t(std::initializer_list<T> il)
				: base_t(std::begin(il), std::end(il))
			{
			}

			array_t(std::initializer_list<object_t::value_type> il)
				: base_t(il)
			{
			}
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
		template <typename...Args>
		object_t(config::string_t path, Args...args)
		{
			add(path, value_type(array_t(args...)));
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

	using json = object_t;

	//////////////////////////////////////////////////////////////////////////

	namespace reader
	{
		//////////////////////////////////////////////////////////////////////////

		namespace x3 = boost::spirit::x3;
		namespace phx = boost::phoenix;
		namespace fusion = boost::fusion;

		namespace details
		{
			//////////////////////////////////////////////////////////////////////////

			struct true_false_t : x3::symbols<boolean_t>
			{
				true_false_t()
				{
					add("true", true);
					add("false", false);
				}
			};

			struct add_utf8_sym_ex
			{
				add_utf8_sym_ex(char s) : _s(s) {}
				template <typename CTX>
				void operator()(const CTX& ctx) const 
				{ 
					x3::_val(ctx) += _s; 
				}
				char _s;
			};

			static auto add_utf8_sym = [](auto& ctx)
			{
				using back_inserter_t = std::back_insert_iterator<config::string_t>;
				back_inserter_t out_iter(x3::_val(ctx));
				boost::utf8_output_iterator<back_inserter_t> utf8_iter(out_iter);
				*utf8_iter++ = x3::_attr(ctx);
			};

			static auto _4hex = x3::uint_parser<uint32_t, 16, 4, 4>{}[add_utf8_sym];
			static auto non_escaped_string = (~x3::char_("\"\\"))[([](auto &ctx)
																   {
																	   x3::_val(ctx) += x3::_attr(ctx);
																   })];
			static auto escaped_string =
				x3::lit("\x5C") >>                         // \ (reverse solidus)
				(
					x3::lit("\x22")[add_utf8_sym_ex('"')]  // "    quotation mark  U+0022
					|
					x3::lit("\x5C")[add_utf8_sym_ex('\\')] // \    reverse solidus U+005C
					|
					x3::lit("\x2F")[add_utf8_sym_ex('/')]  // /    solidus         U+002F
					|
					x3::lit("\x62")[add_utf8_sym_ex('\b')]  // b    backspace       U+0008
					|
					x3::lit("\x66")[add_utf8_sym_ex('\f')]  // f    form feed       U+000C
					|
					x3::lit("\x6E")[add_utf8_sym_ex('\n')]  // n    line feed       U+000A
					|
					x3::lit("\x72")[add_utf8_sym_ex('\r')]  // r    carriage return U+000D
					|
					x3::lit("\x74")[add_utf8_sym_ex('\t')]  // t    tab             U+0009
					|
					x3::lit("\x75") >> _4hex           // uXXXX                U+XXXX
				);

			//////////////////////////////////////////////////////////////////////////
		}

		struct object_t_tag {};

		static details::true_false_t r_boolean;

		static auto r_value = x3::rule<struct value_, object_t::value_type>{ "value_t" };
		static auto r_object = x3::rule<struct object_t_>{ "object_t" };

		static auto r_string = x3::rule<struct string_t_, config::string_t>{ "string_t" };
		static auto r_string_def = x3::lexeme['"' >> *(details::non_escaped_string | details::escaped_string) >> '"'];

		static auto r_array = x3::rule<struct array_t_, array_t>{ "array_t" };
		static auto r_array_def = '[' >> -(r_value % ',') >> ']';

		static auto r_numeric = x3::int_parser<numeric_t>{};
		static auto r_unsigned = x3::uint_parser<unsigned_t>{};
		static auto r_float = x3::real_parser<float_t>{};
		
		static auto r_number = x3::lexeme[r_numeric | r_unsigned >> !x3::char_(".eE")] | r_float;

		static auto r_value_def = r_boolean | r_number | r_string | r_array | r_object;

		static auto create_object = [](auto &ctx)
		{
			auto obj = x3::get<object_t_tag>(ctx);
			auto attr = x3::_attr(ctx);
			obj.get().add(fusion::at_c<0>(attr), fusion::at_c<1>(attr));
		};
		static auto r_object_def = (r_string >> x3::lit(':') >> r_value)[create_object];

		BOOST_SPIRIT_DEFINE(r_value, r_array, r_string, r_object);

		//////////////////////////////////////////////////////////////////////////
	}


	inline object_t from_string(const std::string& input)
	{
		namespace x3 = boost::spirit::x3;

		auto f = input.begin(), l = input.end();
		object_t obj;
		const auto parser = x3::with<reader::object_t_tag>(std::ref(obj))
			[x3::lit('{') >> -(reader::r_object % ',' >> x3::lit('}'))];
		const bool ok = x3::phrase_parse(f, l, parser, x3::space);
		if (ok && f == l)
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
