#include "stdafx.h"

#include <json17/json.h>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(value_tests)

BOOST_AUTO_TEST_CASE(create_value_bool)
{
	json17::value_t<json17::type_config> bool_value_true(true);
	json17::value_t<json17::type_config> bool_value_false(false);

	BOOST_ASSERT_MSG(bool_value_true == true, "failed to compare bool_value_true");
	BOOST_ASSERT_MSG(bool_value_false == false, "failed to compare bool_value_false");
}

BOOST_AUTO_TEST_CASE(create_value_int)
{
	json17::value_t<json17::type_config> int_value(10);

	BOOST_ASSERT_MSG(int_value == 10, "failed to compare int_value");
}

BOOST_AUTO_TEST_CASE(create_value_string)
{
	json17::value_t<json17::type_config> string_value("string");

	BOOST_ASSERT_MSG(string_value == "string", "failed to compare string_value");
}

BOOST_AUTO_TEST_CASE(create_value_array)
{
	json17::array_t<json17::type_config> arr1(1, "string", false, 10U, true, std::string("hello"));
	json17::value_t<json17::type_config> array_value{arr1};

	BOOST_ASSERT_MSG(array_value != 10, "failed to compare array_value");
	BOOST_ASSERT_MSG(array_value == arr1, "failed to compare array_value");
	BOOST_ASSERT_MSG(array_value == json17::array_t<json17::type_config>(1, "string", false, 10U, true, std::string("hello")), "failed to compare");
 	BOOST_ASSERT_MSG(array_value != json17::array_t<json17::type_config>(1), "failed to compare");
}

BOOST_AUTO_TEST_SUITE_END()