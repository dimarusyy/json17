#pragma once

#include "config.h"

#ifdef _DEBUG
#define BOOST_SPIRIT_X3_DEBUG
#endif

#include <boost/blank.hpp>
#include <boost/variant.hpp>

#include <boost/fusion/adapted.hpp>
#include <boost/fusion/adapted/std_pair.hpp>
#include <boost/spirit/home/x3.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/spirit/home/x3/support/traits/container_traits.hpp>
#include <boost/spirit/include/support_istream_iterator.hpp>

#include <boost/spirit/include/karma.hpp>
#include <boost/spirit/include/qi.hpp>

#include <boost/algorithm/string.hpp>

#include <boost/filesystem.hpp>

#include <cstdint>
#include <iomanip>
#include <sstream>
#include <initializer_list>
#include <vector>
#include <exception>
#include <unordered_map>
#include <type_traits>
#include <locale>
#include <codecvt>

//////////////////////////////////////////////////////////////////////////
// json17 detail
//////////////////////////////////////////////////////////////////////////
namespace json17
{
#if 0
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
			using back_inserter_t = std::back_insert_iterator<TConfig::string_t>;
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

		static void append_utf8(TConfig::string_t& to, uint32_t codepoint)
		{
			auto out = std::back_inserter(to);
			boost::utf8_output_iterator<decltype(out)> convert(out);
			*convert++ = codepoint;
		}

		BOOST_PHOENIX_ADAPT_FUNCTION(std::string, utf8_escape_phx, detail::utf8_escape, 1)
	}
#endif
}
//////////////////////////////////////////////////////////////////////////
// !json17 detail
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// json17 types
//////////////////////////////////////////////////////////////////////////
namespace json17
{
	struct type_config
	{
		using null_t = boost::blank;
		using boolean_t = bool;
		using integer_t = int32_t;
		using unsigned_t = uint32_t;
		using real_t = double;
		using string_t = std::string;

		template <typename T>
		using array_t = std::vector<T>;
	};

	template <typename TConfig>
	struct array_t;

	template <typename TConfig>
	struct object_t;

	template <typename TConfig>
	using value_base_t = boost::variant<
		typename TConfig::null_t,
		typename TConfig::boolean_t, typename TConfig::integer_t, typename TConfig::unsigned_t, typename TConfig::real_t, typename TConfig::string_t,
		boost::recursive_wrapper<object_t<TConfig>>,
		boost::recursive_wrapper<array_t<TConfig>>
	>;

	template <typename TConfig>
	struct value_t : value_base_t<TConfig>
	{
		using base_t = value_base_t<TConfig>;

		// use base_t ctor's
		using base_t::base_t;
		
		value_t(const typename TConfig::string_t::value_type* val)
			: base_t(TConfig::string_t(val))
		{
		}

		template <typename T>
		bool operator==(const T& rhs) const
		{
			return boost::apply_visitor([&rhs](const auto& val) 
										{ 
											if constexpr (std::is_same_v<value_t<TConfig>, std::decay_t<T>>)
											{
												// swap
												return rhs == val;
											}
											else if constexpr (std::is_same_v<std::decay_t<decltype(val)>, std::decay_t<T>>)
											{
												return val == rhs;
											}
											else if constexpr (std::is_same_v<std::decay_t<decltype(val)>, TConfig::string_t> &&
															   std::is_constructible_v<TConfig::string_t, std::decay_t<decltype(rhs)>>)
											{
												return val == rhs;
											}
											else
											{
												return false;
											}
										},
										*this);
		}

		template <typename T>
		bool operator!=(const T& rhs) const
		{
			return !(this->operator ==(rhs));
		}
	};

	template <typename TConfig>
	struct array_t : TConfig::template array_t<value_t<TConfig>>
	{
		using base_t = typename TConfig::template array_t<value_t<TConfig>>;
		
		using base_t::base_t;
		using base_t::begin;
		using base_t::end;

		template <typename...Args>
		array_t(Args...args)
			: base_t(std::initializer_list<value_t<TConfig>>{value_t<TConfig>(args)...})
		{
		}

		template <typename T>
		array_t(std::initializer_list<T> il)
			: base_t(std::begin(il), std::end(il))
		{
		}

		array_t(std::initializer_list<value_t<TConfig>> il)
			: base_t(il)
		{
		}

		bool operator==(const array_t<TConfig>& rhs) const
		{
			const auto p = std::mismatch(this->begin(), this->end(), rhs.begin(), rhs.end());
			return p.first == this->end() && p.second == rhs.end();
		}

		bool operator!=(const array_t<TConfig>& rhs) const
		{
			return !(*this == rhs);
		}
	};

	template <typename TConfig>
	struct object_t //<name> : <value>
	{
		object_t() = default;

		template <typename T>
		object_t(typename TConfig::string_t path, T&& val)
		{
			add(path, value_t<TConfig>(std::forward<T>(val)));
		}

		// {  "foo" : [1, 2, 3] }
		template <typename...Args>
		object_t(typename TConfig::string_t path, Args...args)
		{
			add(path, array_t<TConfig>(args...));
		}

		// { "foo" : { "bar" : 10 } }
		object_t(typename TConfig::string_t path, object_t<TConfig> obj)
		{
			add(path, std::move(obj));
		}

		// { {"foo" : true}, {"bar" : 4} }
		object_t(std::initializer_list<object_t<TConfig>> il)
		{
			std::for_each(std::begin(il), std::end(il), [this](const auto& v)
						  {
							  _pairs.insert(std::begin(v._pairs), std::end(v._pairs));
						  });
		}

		// { "foo" : {...} }
		object_t(const value_t<TConfig>& val)
			: object_t<TConfig>(boost::apply_visitor([](const auto& v)
										{
											if constexpr (std::is_same_v<std::decay_t<decltype(v)>, object_t<TConfig>>)
											{
												return v;
											}
											else
											{
												static_assert("can't construct object");
												return object_t<TConfig>();
											}
										}, val))
		{
		}

		bool operator==(const object_t<TConfig>& rhs) const
		{
			return _pairs == rhs._pairs;
		}

		bool operator!=(const object_t<TConfig>& rhs) const
		{
			return !(*this == rhs);
		}

		using holder_t = std::unordered_map<typename TConfig::string_t, value_t<TConfig>>;
		holder_t _pairs;

		void add(typename holder_t::key_type key, typename holder_t::mapped_type val)
		{
			_pairs.emplace(key, val);
		}

		void add(typename holder_t::value_type&& val)
		{
			_pairs.insert(std::move(val));
		}

		value_t<TConfig>& operator[](const typename TConfig::string_t& path)
		{
			bool is_found = false;
			auto get_value = [this, &is_found](const TConfig::string_t& key) -> value_t<TConfig>&
			{
				const auto path_it = _pairs.find(key);
				if (path_it == _pairs.end())
				{
					is_found = false;
					_pairs.insert(std::make_pair(key, TConfig::null_t{}));
					return _pairs.find(key)->second;
				};
				is_found = true;
				return path_it->second;
			};

			// split path
			auto v_path = split_path(path);

			// traverse recursively
			const auto front_key = v_path.front();
			if (v_path.size() == 1)
			{
				return get_value(front_key);
			}
			else
			{
				// remove first entry 
				v_path.erase(v_path.begin());

				// try to get value
				auto& key_val = get_value(front_key);
				if (is_found) // found = yes
				{
					return boost::apply_visitor([&v_path, this](auto& v) -> value_t<TConfig>&
												{
													if constexpr (std::is_same_v<std::decay_t<decltype(v)>, object_t<TConfig>>)
													{
														return join_path_and_invoke(v_path, v);														
													}
													else
													{
														// some value is given but not object_t for deeper traversing
														throw std::runtime_error("can't traverse json : expected sub-level but value is obtained");
													}
												},
												key_val);
				}
				else
				{
					// add null_t
					auto obj = object_t<TConfig>(front_key, TConfig::null_t{});
					_pairs[front_key] = obj;

					return join_path_and_invoke(v_path, obj);
				}
			}
		}

		private:
			auto split_path(const typename TConfig::string_t& path) const
			{
				std::vector<TConfig::string_t> v_path;
				if constexpr (std::is_same_v<TConfig::string_t::value_type, std::string::value_type>)
				{
					boost::split(v_path, path, boost::is_any_of(TConfig::string_t(".")));
				}
				else
				{
					// handle wchar_t
					boost::split(v_path, path, boost::is_any_of(TConfig::string_t(L".")));
				}
				return v_path;
			}

			auto& join_path_and_invoke(const std::vector<typename TConfig::string_t>& v_path, object_t<TConfig>& obj)
			{
				if constexpr (std::is_same_v<TConfig::string_t::value_type, std::string::value_type>)
				{
					return obj.operator [](boost::join(v_path, TConfig::string_t(".")));
				}
				else
				{
					// handle wchar_t
					return obj.operator [](boost::join(v_path, TConfig::string_t(L".")));
				}
			}
	};
}

//////////////////////////////////////////////////////////////////////////
// boost.spirit.x3 traits
//////////////////////////////////////////////////////////////////////////
namespace boost { namespace spirit { namespace x3 { namespace traits {
template<typename TConfig>
struct push_back_container<json17::object_t<TConfig>>
{
	template <typename T>
	static bool call(json17::object_t<TConfig>& obj, T&& val)
	{
		obj.add(std::forward<T>(val));
		return true;
	}
};
}}}}
//////////////////////////////////////////////////////////////////////////
// ! boost.spirit.x3 traits
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// json17::parser
//////////////////////////////////////////////////////////////////////////
namespace json17
{
	namespace fs = boost::filesystem;
	namespace x3 = boost::spirit::x3;
	namespace phx = boost::phoenix;
	namespace fusion = boost::fusion;

	template <typename TConfig>
	struct true_false_t : x3::symbols<typename TConfig::boolean_t>
	{
		using base_t = x3::symbols<typename TConfig::boolean_t>;
		true_false_t()
		{
			base_t::add("true", true);
			base_t::add("false", false);
		}
	};

	template <typename TConfig>
	struct add_sym_ex
	{
		add_sym_ex(typename TConfig::string_t::value_type s) : _s(s) {}

		template <typename T>
		void operator()(const T& ctx) const
		{
			x3::_val(ctx) += _s;
		}
		typename TConfig::string_t::value_type _s;
	};

	template <typename TConfig>
	struct parser
	{
		static const auto add_sym = [](auto& ctx)
		{
			using back_inserter_t = std::back_insert_iterator<TConfig::string_t>;
			back_inserter_t out_iter(x3::_val(ctx));
			boost::utf8_output_iterator<back_inserter_t> iter(out_iter);
			*iter++ = x3::_attr(ctx);
		};

		static const auto _4hex = x3::uint_parser<uint32_t, 16, 4, 4>{}[add_sym];
		static const auto non_escaped_string = (~x3::char_("\"\\"))[([](auto &ctx) { x3::_val(ctx) += x3::_attr(ctx); })];
		static const auto escaped_string =
			x3::lit("\x5C") >>                         // \ (reverse solidus)
			(
				x3::lit("\x22")[add_sym_ex<TConfig>('"')]  // "    quotation mark  U+0022
				|
				x3::lit("\x5C")[add_sym_ex<TConfig>('\\')] // \    reverse solidus U+005C
				|
				x3::lit("\x2F")[add_sym_ex<TConfig>('/')]  // /    solidus         U+002F
				|
				x3::lit("\x62")[add_sym_ex<TConfig>('\b')]  // b    backspace       U+0008
				|
				x3::lit("\x66")[add_sym_ex<TConfig>('\f')]  // f    form feed       U+000C
				|
				x3::lit("\x6E")[add_sym_ex<TConfig>('\n')]  // n    line feed       U+000A
				|
				x3::lit("\x72")[add_sym_ex<TConfig>('\r')]  // r    carriage return U+000D
				|
				x3::lit("\x74")[add_sym_ex<TConfig>('\t')]  // t    tab             U+0009
				|
				x3::lit("\x75") >> _4hex           // uXXXX                U+XXXX
				);

		static const auto r_null = x3::lit("null") >> x3::attr(json17::null_t());

		static const true_false_t<TConfig> r_boolean;

		static const auto r_value = x3::rule<struct value_, value_t<TConfig>>{ "value_t" };
		static const auto r_object = x3::rule<struct object_t_, object_t<TConfig>>{ "object_t" };

		static const auto r_string = x3::rule<struct string_t_, TConfig::string_t>{ "string_t" };
		static const auto r_string_def = x3::lexeme['"' >> *(non_escaped_string | escaped_string) >> '"'];

		static const auto r_array = x3::rule<struct array_t_, array_t<TConfig>>{ "array_t" };
		static const auto r_array_def = '[' >> -(r_value % ',') >> ']';

		static const auto r_integer = x3::rule<struct numeric_t_, TConfig::integer_t>{ "numeric_t" };
		static const auto r_integer_def = x3::int_parser<TConfig::integer_t>{};

		static const auto r_unsigned = x3::rule<struct unsigned_t_, TConfig::unsigned_t>{ "unsigned_t" };
		static const auto r_unsigned_def = x3::uint_parser<TConfig::unsigned_t>{};

		static const auto r_real = x3::rule<struct real_, TConfig::real_t>{ "real_t" };
		static const auto r_real_def = x3::real_parser<TConfig::real_t>{};

		static const auto r_numeric = x3::lexeme[r_unsigned >> !x3::char_(".eE")] | x3::lexeme[r_integer >> !x3::char_(".eE")] | r_real;

		static const auto r_value_def = r_null | r_boolean | r_numeric | r_string | r_array | r_object;

		static const auto r_object_def = '{' >> ((r_string >> ':' >> r_value) % ',') >> '}';

		BOOST_SPIRIT_DEFINE(r_integer, r_unsigned, r_real, r_value, r_array, r_string, r_object);
	};
}
//////////////////////////////////////////////////////////////////////////
// !json17::parser
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
// json17::json
//////////////////////////////////////////////////////////////////////////
namespace json17
{
	template <typename TConfig = type_config>
	struct json : object_t<TConfig>
	{
		using base_t = object_t<TConfig>;

		using base_t::base_t;
		using base_t::operator ==;
		using base_t::operator !=;
		using base_t::operator [];

		void operator<<(const std::istream& input)
		{
			*this = std::move(parse(input));
		}

		void operator<<(const typename TConfig::string_t& input)
		{
			*this = std::move(parse(input));
		}

	private :
		auto parse(const typename TConfig::string_t& str)
		{
			auto f(std::begin(str)), l(std::end(str));
			return parse_impl(f, l);
		}

		auto parse(const std::istream& input)
		{
			boost::spirit::istream_iterator f(input), l;
			return parse_impl(f, l);
		}

		template <typename It>
		auto parse_impl(It f, It l)
		{
			namespace x3 = boost::spirit::x3;

			object_t<TConfig> obj;
			const auto ok = x3::parse(f, l, x3::skip(x3::space)[parser<TConfig>::r_object], obj);
			if (ok)
			{
				return obj;
			}
			else
			{
				std::stringstream ss;
				if constexpr (std::is_same_v<TConfig::string_t::value_type, std::string::value_type>)
				{
					ss << "failed to parse, remaining : [" << 
						std::quoted(TConfig::string_t{ f, l }) << "]";
				}
				else
				{
					ss << "failed to parse, remaining : [" << 
						std::quoted(std::wstring_convert<std::codecvt_utf8<TConfig::string_t::value_type>>().to_string(typename TConfig::string_t{ f, l })) << "]";
				}
				throw std::runtime_error(ss.str());
			}
		}
	};
}
//////////////////////////////////////////////////////////////////////////
// !json17::json
//////////////////////////////////////////////////////////////////////////

#if 0
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
}}}}
//////////////////////////////////////////////////////////////////////////
// ! boost.spirit.x3 traits
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// boost.spirit.karma adapting
//////////////////////////////////////////////////////////////////////////

BOOST_FUSION_ADAPT_STRUCT(json17::object_t, _pairs)

//////////////////////////////////////////////////////////////////////////
// ! boost.spirit.karma adapting
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

		static auto r_string = x3::rule<struct string_t_, TConfig::string_t>{ "string_t" };
		static auto r_string_def = x3::lexeme['"' >> *(non_escaped_string | escaped_string) >> '"'];

		static auto r_array = x3::rule<struct array_t_, array_t>{ "array_t" };
		static auto r_array_def = '[' >> -(r_value % ',') >> ']';

		static auto r_integer = x3::rule<struct numeric_t_, TConfig::integer_t>{ "numeric_t" };
		static auto r_integer_def = x3::int_parser<TConfig::integer_t>{};

		static auto r_unsigned = x3::rule<struct unsigned_t_, TConfig::unsigned_t>{ "unsigned_t" };
		static auto r_unsigned_def = x3::uint_parser<TConfig::unsigned_t>{};

		static auto r_real = x3::rule<struct real_, TConfig::real_t>{ "real_t" };
		static auto r_real_def = x3::real_parser<TConfig::real_t>{};

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
			_boolean.add(TConfig::boolean_t(false), "false")(TConfig::boolean_t(true), "true");

			_unsigned = karma::uint_generator<TConfig::unsigned_t>{};
			_integer = karma::uint_generator<TConfig::unsigned_t>{};
			_real = karma::real_generator<TConfig::real_t>{};

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
		karma::symbols<boolean_t, TConfig::string_t> _boolean;

		karma::rule<Iterator, object_t(), Delimiter> _start;

		karma::rule<Iterator, object_t(), Delimiter> _object;
		karma::rule<Iterator, object_t::value_t(), Delimiter> _value;
		karma::rule<Iterator, array_t(), Delimiter>  _array;

		karma::rule<Iterator, integer_t(), Delimiter>  _integer;
		karma::rule<Iterator, unsigned_t(), Delimiter>  _unsigned;
		karma::rule<Iterator, real_t(), Delimiter>  _real;

		karma::rule<Iterator, TConfig::string_t()> _string;
		karma::rule<Iterator, char()>     _char;
		karma::rule<Iterator, uint32_t()> _utf8_escaped;

		karma::symbols<char, TConfig::string_t>    _char_escape;
	};

	inline object_t from_string(const TConfig::string_t &input)
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
			ss << "failed to parse, remaining : [" << std::quoted(TConfig::string_t{ f, l }) << "]";
			throw std::runtime_error(ss.str());
		}
	}

	inline TConfig::string_t to_string(const object_t& obj)
	{
		TConfig::string_t output;
		auto out = std::back_inserter(output);

		static const generator<decltype(out), qi::unused_type> g;
		karma::generate(out, g, obj);

		return output;
	}
}
//////////////////////////////////////////////////////////////////////////
// !json17 serialize/de-serialize
//////////////////////////////////////////////////////////////////////////

#endif