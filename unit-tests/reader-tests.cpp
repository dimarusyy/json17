#include "stdafx.h"

#include <json17/json.h>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(reader_tests)

BOOST_AUTO_TEST_CASE(reader_test_types)
{
 	const std::string str = R"({ "string" : "some_string", "numeric" : 10, "integer" : -10, "bool" : true, "float" : -1.04e6, "array": [1, true, "some_string"]})";
	BOOST_CHECK_NO_THROW(json17::from_string(str));
}

BOOST_AUTO_TEST_CASE(reader_test_complex)
{
	const std::string str = R"({ "object" : { "numeric" : 10 } })";
	BOOST_CHECK_NO_THROW(json17::from_string(str));
}

BOOST_AUTO_TEST_SUITE_END()