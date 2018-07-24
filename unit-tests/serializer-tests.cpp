#include "stdafx.h"

#include <json17/json.h>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(serializer_tests)

BOOST_AUTO_TEST_CASE(serialize_1)
{
	json17::object_t o1({ "config",{ { "log" , "some_path" },{ "level" , 4 },{ "enabled", true },{ "rotate", 1,2,3,4,5,6, true } } });
	const auto str = json17::to_string(o1);
	
	//BOOST_ASSERT_MSG(o1.get_value<bool>("enabled") == true, "not found");
	//BOOST_ASSERT_MSG(o1.get_value<bool>("disabled", false) == false, "not found");
}


BOOST_AUTO_TEST_SUITE_END()