#include "stdafx.h"

#include <json17/json.h>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(array_tests)

BOOST_AUTO_TEST_CASE(create_array)
{
	json17::array_t<json17::type_config> arr1(1, "string", false, 10U, true, std::string("hello"));
	json17::array_t<json17::type_config> arr2{ 1, "string", false, 10U, true, std::string("hello") };
}

BOOST_AUTO_TEST_SUITE_END()