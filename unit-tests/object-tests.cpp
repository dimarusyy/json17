#include "stdafx.h"

#include <json17/json.h>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(object_tests)

#if 1
BOOST_AUTO_TEST_CASE(object_t_create_from_boolean)
{
	json17::object_t o1("enabled", true);
	BOOST_ASSERT_MSG(o1.get_value<bool>("enabled") == true, "not found");
	BOOST_ASSERT_MSG(o1.get_value<bool>("disabled", false) == false, "not found");
}

BOOST_AUTO_TEST_CASE(object_t_create_from_number)
{
	json17::object_t o1("numeric", 10);
	BOOST_ASSERT_MSG(o1.get_value<json17::integer_t>("numeric") == 10, "not found");
}

BOOST_AUTO_TEST_CASE(object_t_create_from_unsigned)
{
	json17::object_t o1("unsigned", 12U);
	BOOST_ASSERT_MSG(o1.get_value<json17::unsigned_t>("unsigned") == 12U, "not found");
}

BOOST_AUTO_TEST_CASE(object_t_create_from_float)
{
	json17::object_t o1("float", 12.0);
	BOOST_ASSERT_MSG(o1.get_value<json17::real_t>("float") == 12.0, "not found");
}

BOOST_AUTO_TEST_CASE(object_t_create_from_array)
{
	json17::object_t o1("array_int", 1, 2, 3, 4);
	auto rc1 = o1.get_value<json17::array_t>("array_int") == json17::array_t{ 1, 2, 3, 4 };
	BOOST_ASSERT_MSG(rc1, "not found");

	json17::object_t o2("array_bool",  true, false, true);
	auto rc2 = o2.get_value<json17::array_t>("array_bool") == json17::array_t{ true, false, true };
	BOOST_ASSERT_MSG(rc2, "not found");

	json17::object_t o3("array_float", 1.3, 1. );
	auto rc3 = o3.get_value<json17::array_t>("array_float") == json17::array_t{ 1.3, 1. };
	BOOST_ASSERT_MSG(rc3, "not found");
}

BOOST_AUTO_TEST_CASE(object_t_create_from_variadic_template)
{
	json17::array_t arr1(1, "string", false, 10U, true, std::string("hello"));
	json17::object_t o1;
	o1["variadic_array"] = arr1;
	auto rc1 = o1.get_value<json17::array_t>("variadic_array") == arr1;
	BOOST_ASSERT_MSG(rc1, "not found");
}
#endif

BOOST_AUTO_TEST_CASE(object_t_create_from_string)
{
	json17::object_t o1("std::string", "some char[]");
	BOOST_ASSERT_MSG(o1.get_value<std::string>("std::string") == std::string("some char[]"), "not found");

	json17::object_t o2("std::string", std::string("some std::string"));
	BOOST_ASSERT_MSG(o2.get_value<std::string>("std::string") == std::string("some std::string"), "not found");
}

#if 1
BOOST_AUTO_TEST_CASE(object_t_create_from_object)
{
	json17::object_t o1("object", { "enabled", true });
	json17::object_t obj = o1.get_value<>("object");
	BOOST_ASSERT_MSG(obj.get_value<json17::boolean_t>("enabled"), "not found");
}

BOOST_AUTO_TEST_CASE(object_t_create_complex_1)
{
	json17::object_t o1({ "config", { { "log" , "some_path" }, { "level" , 4 }, { "enabled", true }, { "rotate", 1,2,3,4,5,6,true } } });

	auto o1_log = o1.get_value<json17::string_t>("config.log");
	auto o1_level = o1.get_value<json17::integer_t>("config.level");
	auto o1_enabled = o1.get_value<json17::boolean_t>("config.enabled");
	auto o1_rotate = o1.get_value<json17::array_t>("config.rotate");

	BOOST_ASSERT_MSG(o1_log == "some_path", "not found config.log");
	BOOST_ASSERT_MSG(o1_level == 4, "not found config.level");
	BOOST_ASSERT_MSG(o1_enabled == true, "not found config.enabled");
	
	auto rc = o1_rotate == json17::array_t{ 1,2,3,4,5,6,true };
	BOOST_ASSERT_MSG(rc, "not found config.rotate");
}

BOOST_AUTO_TEST_CASE(object_t_create_complex_2)
{
	json17::object_t o1("config", { {"v1", {{ "log" , "some_path" }} }, { "v2", {{ "level" , 4 },{ "enabled", true }} } });
	
	auto o1_log = o1.get_value<json17::string_t>("config.v1.log"); 
	auto o1_level = o1.get_value<json17::integer_t>("config.v2.level");
	auto o1_enabled = o1.get_value<json17::boolean_t>("config.v2.enabled");

	BOOST_ASSERT_MSG(o1_log == "some_path", "not found config.v1.log");
	BOOST_ASSERT_MSG(o1_level == 4, "not found config.v2.level");
	BOOST_ASSERT_MSG(o1_enabled == true, "not found config.v2.level");
}
#endif
BOOST_AUTO_TEST_SUITE_END()