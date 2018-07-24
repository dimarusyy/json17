#pragma once

#include "config.h"

#ifdef _DEBUG
#define BOOST_SPIRIT_X3_DEBUG
#endif

#include <boost/variant.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/fusion/adapted.hpp>
#include <boost/fusion/adapted/std_pair.hpp>
#include <boost/spirit/home/x3.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/spirit/home/x3/support/traits/container_traits.hpp>

#include <boost/spirit/include/karma.hpp>
#include <boost/spirit/include/qi.hpp>

#include <iomanip>
#include <sstream>
#include <initializer_list>
#include <vector>
#include <exception>

//////////////////////////////////////////////////////////////////////////
// json17 detail
//////////////////////////////////////////////////////////////////////////
namespace json17
{
	namespace detail
	{
		struct add_utf8_sym_ex
		{
			add_utf8_sym_ex(char s) : _s(s) {}
			template <typename T>
			void operator()(const T& ctx) const
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

		static std::string utf8_escape(char ch)
		{
			std::ostringstream oss;
			oss << "\\u" << std::setw(4) << std::setfill('0') << std::hex << ((int)ch);
			return oss.str();
		}

		static void append_utf8(config::string_t& to, uint32_t codepoint)
		{
			auto out = std::back_inserter(to);
			boost::utf8_output_iterator<decltype(out)> convert(out);
			*convert++ = codepoint;
		}

		BOOST_PHOENIX_ADAPT_FUNCTION(std::string, utf8_escape_phx, detail::utf8_escape, 1)
	}
}
//////////////////////////////////////////////////////////////////////////
// !json17 detail
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// json17 types
//////////////////////////////////////////////////////////////////////////
namespace json17
{
	struct object_t //<name> : <value>
	{
		struct array_t;
		using value_t = boost::variant<
			config::null_t,
			config::boolean_t, config::integer_t, config::unsigned_t, config::real_t, config::string_t,
			boost::recursive_wrapper<object_t>,
			boost::recursive_wrapper<array_t>>;
		using value_type = std::pair<config::string_t, value_t>;

		using holder_t = config::holder_t<config::string_t, value_t>;

		struct array_t : config::array_t<object_t::value_t>
		{
			using base_t = config::array_t<value_t>;

			template <typename...Args>
			array_t(Args...args)
				: base_t(std::initializer_list<object_t::value_t>{object_t::value_t(args)...})
			{
			}

			template <typename T>
			array_t(std::initializer_list<T> il)
				: base_t(std::begin(il), std::end(il))
			{
			}

			array_t(std::initializer_list<object_t::value_t> il)
				: base_t(il)
			{
			}
		};

		// default
		object_t() = default;

		// { "foo" : true }
		template <typename U, typename std::enable_if<
			std::is_same<config::null_t, U>::value ||
			std::is_same<config::boolean_t, U>::value ||
			std::is_same<config::integer_t, U>::value ||
			std::is_same<config::unsigned_t, U>::value ||
			std::is_same<config::real_t, U>::value>::type* = 0>
			object_t(config::string_t path, U&& val)
		{
			add(path, value_t(std::forward<U>(val)));
		}

		// { "foo" : "some_string" }
		template <typename U, typename std::enable_if<std::is_constructible<config::string_t, U>::value>::type* = 0>
		object_t(config::string_t path, U&& val)
		{
			add(path, value_t(config::string_t(std::forward<U>(val))));
		}

		// {  "foo" : [1, 2, 3] }
		template <typename...Args>
		object_t(config::string_t path, Args...args)
		{
			add(path, value_t(array_t(args...)));
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

		void add(config::string_t key, value_t val)
		{
			_pairs.emplace(key, val);
		}

		void add(value_type&& val)
		{
			_pairs.insert(std::move(val));
		}

		bool operator==(const object_t& other) const
		{
			return _pairs == other._pairs;
		}

		bool operator!=(const object_t& other) const
		{
			return !(*this == other);
		}

		value_t operator[](const config::string_t& path) const
		{
			const auto path_it = _pairs.find(path);
			if (path_it == _pairs.end())
			{
				return config::null_t{};
			}
			return path_it->second;
		}

		value_t& operator[](const config::string_t& path)
		{
			const auto path_it = _pairs.find(path);
			if (path_it == _pairs.end())
			{
				_pairs[path] = value_t{};
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

		holder_t _pairs;
	};

	using null_t = config::null_t;
	using boolean_t = config::boolean_t;
	using unsigned_t = config::unsigned_t;
	using integer_t = config::integer_t;
	using real_t = config::real_t;
	using string_t = config::string_t;
	using array_t = object_t::array_t;
	using value_t = object_t::value_t;
}
//////////////////////////////////////////////////////////////////////////
// !json17 types
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// boost.spirit.x3 traits
//////////////////////////////////////////////////////////////////////////
namespace boost { namespace spirit { namespace x3 { namespace traits {
	template<>
	struct push_back_container<json17::object_t>
	{
		template <typename T>
		static bool call(json17::object_t& obj, T&& val)
		{
			obj.add(std::forward<T>(val));
			return true;
		}
	};
} } } }
//////////////////////////////////////////////////////////////////////////
// ! boost.spirit.x3 traits
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// boost.spirit.karma adopting
//////////////////////////////////////////////////////////////////////////

BOOST_FUSION_ADAPT_STRUCT(json17::object_t, (json17::object_t::holder_t, _pairs))

//////////////////////////////////////////////////////////////////////////
// ! boost.spirit.karma adopting
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// json17 serialize/de-serialize
//////////////////////////////////////////////////////////////////////////
namespace json17
{
	namespace x3 = boost::spirit::x3;
	namespace phx = boost::phoenix;
	namespace fusion = boost::fusion;
	
	namespace qi = boost::spirit::qi; 
	namespace karma = boost::spirit::karma;

	namespace encoding = boost::spirit::ascii;

	namespace parser
	{
		struct true_false_t : x3::symbols<boolean_t>
		{
			true_false_t()
			{
				add("true", true);
				add("false", false);
			}
		};

		static auto _4hex = x3::uint_parser<uint32_t, 16, 4, 4>{}[detail::add_utf8_sym];
		static auto non_escaped_string = (~x3::char_("\"\\"))[([](auto &ctx) { x3::_val(ctx) += x3::_attr(ctx); })];
		static auto escaped_string =
			x3::lit("\x5C") >>                         // \ (reverse solidus)
			(
				x3::lit("\x22")[detail::add_utf8_sym_ex('"')]  // "    quotation mark  U+0022
				|
				x3::lit("\x5C")[detail::add_utf8_sym_ex('\\')] // \    reverse solidus U+005C
				|
				x3::lit("\x2F")[detail::add_utf8_sym_ex('/')]  // /    solidus         U+002F
				|
				x3::lit("\x62")[detail::add_utf8_sym_ex('\b')]  // b    backspace       U+0008
				|
				x3::lit("\x66")[detail::add_utf8_sym_ex('\f')]  // f    form feed       U+000C
				|
				x3::lit("\x6E")[detail::add_utf8_sym_ex('\n')]  // n    line feed       U+000A
				|
				x3::lit("\x72")[detail::add_utf8_sym_ex('\r')]  // r    carriage return U+000D
				|
				x3::lit("\x74")[detail::add_utf8_sym_ex('\t')]  // t    tab             U+0009
				|
				x3::lit("\x75") >> _4hex           // uXXXX                U+XXXX
				);

		struct object_t_tag {};

		static auto r_null = x3::lit("null") >> x3::attr(json17::null_t());

		static true_false_t r_boolean;

		static auto r_value = x3::rule<struct value_, object_t::value_t>{ "value_t" };
		static auto r_object = x3::rule<struct object_t_, object_t>{ "object_t" };

		static auto r_string = x3::rule<struct string_t_, config::string_t>{ "string_t" };
		static auto r_string_def = x3::lexeme['"' >> *(non_escaped_string | escaped_string) >> '"'];

		static auto r_array = x3::rule<struct array_t_, array_t>{ "array_t" };
		static auto r_array_def = '[' >> -(r_value % ',') >> ']';

		static auto r_integer = x3::rule<struct numeric_t_, config::integer_t>{ "numeric_t" };
		static auto r_integer_def = x3::int_parser<config::integer_t>{};

		static auto r_unsigned = x3::rule<struct unsigned_t_, config::unsigned_t>{ "unsigned_t" };
		static auto r_unsigned_def = x3::uint_parser<config::unsigned_t>{};

		static auto r_real = x3::rule<struct real_, config::real_t>{ "real_t" };
		static auto r_real_def = x3::real_parser<config::real_t>{};

		static auto r_numeric = x3::lexeme[r_unsigned >> !x3::char_(".eE")] | x3::lexeme[r_integer >> !x3::char_(".eE")] | r_real;

		static auto r_value_def = r_null | r_boolean | r_numeric | r_string | r_array | r_object;

		static auto r_object_def = '{' >> ((r_string >> ':' >> r_value) % ',') >> '}';

		BOOST_SPIRIT_DEFINE(r_integer, r_unsigned, r_real, r_value, r_array, r_string, r_object);
	}

	template <typename Iterator, typename Delimiter>
	struct generator final : karma::grammar<Iterator, object_t(), Delimiter>
	{
		generator() : generator::base_type(_start)
		{
			_boolean.add(config::boolean_t(false), "false")(config::boolean_t(true), "true");

			_unsigned = karma::uint_generator<config::unsigned_t>{};
			_integer = karma::uint_generator<config::unsigned_t>{};
			_real = karma::real_generator<config::real_t>{};

			_value = karma::attr_cast<null_t>(karma::lit("null")) | _boolean | _integer | _unsigned | _real | _string | _array | _object;
			_array = '[' << -(_value % ',') << ']';

			_object = '{' << -((_string << ':' << _value) % ',') << '}';

			_char_escape.add
			    ('"', "\\\"")
				('\\', "\\\\")
				//('/',  "\\/")
				('\b', "\\b")
				('\f', "\\f")
				('\n', "\\n")
				('\r', "\\r")
				('\t', "\\t");

			_utf8_escaped =
				karma::eps(karma::_val >= uint32_t(0x0) && karma::_val <= uint32_t(0x1f)) << 
				karma::string[karma::_1 = detail::utf8_escape_phx(karma::_val)];

			_char = _char_escape | _utf8_escaped | encoding::char_;
			_string = '"' << *_char << '"';

			// entry point
			_start = _object;

			BOOST_SPIRIT_DEBUG_NODES((_start)(_value)(_object)(_array)(_boolean)(_unsigned)(_integer)(_real)(_string)(_char)(_utf8_escaped))
		}

	private:
		karma::symbols<boolean_t, config::string_t> _boolean;

		karma::rule<Iterator, object_t(), Delimiter> _start;

		karma::rule<Iterator, object_t(), Delimiter> _object;
		karma::rule<Iterator, object_t::value_t(), Delimiter> _value;
		karma::rule<Iterator, array_t(), Delimiter>  _array;

		karma::rule<Iterator, integer_t(), Delimiter>  _integer;
		karma::rule<Iterator, unsigned_t(), Delimiter>  _unsigned;
		karma::rule<Iterator, real_t(), Delimiter>  _real;

		karma::rule<Iterator, config::string_t()> _string;
		karma::rule<Iterator, char()>     _char;
		karma::rule<Iterator, uint32_t()> _utf8_escaped;

		karma::symbols<char, config::string_t>    _char_escape;
	};

	inline object_t from_string(const config::string_t &input)
	{
		namespace x3 = boost::spirit::x3;

		auto f = input.begin(), l = input.end();
		object_t obj;
		const auto ok = x3::parse(f, l, x3::skip(x3::space)[parser::r_object], obj);
		if (ok)
		{
			return obj;
		}
		else
		{
			std::stringstream ss;
			ss << "failed to parse, remaining : [" << std::quoted(config::string_t{ f, l }) << "]";
			throw std::runtime_error(ss.str());
		}
	}

 	inline config::string_t to_string(const object_t& obj)
 	{
		config::string_t output;
		auto out = std::back_inserter(output);

		static const generator<decltype(out), qi::unused_type> g;
		karma::generate(out, g, obj);

		return output;
 	}
}
//////////////////////////////////////////////////////////////////////////
// !json17 json17 serialize/de-serialize
//////////////////////////////////////////////////////////////////////////

