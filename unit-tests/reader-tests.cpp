#include "stdafx.h"

#include <json17/json.h>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(reader_tests)

BOOST_AUTO_TEST_CASE(reader_test_1)
{
 	const std::string str = R"({ "string" : "some_string", "numeric" : 10, "integer" : -10, "bool" : true})";
 	auto obj = json17::from_string(str);
}

BOOST_AUTO_TEST_SUITE_END()