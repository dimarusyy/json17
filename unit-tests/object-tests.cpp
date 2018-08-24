#include "stdafx.h"

#include <json17/json.h>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(object_tests)

#if 1
BOOST_AUTO_TEST_CASE(object_t_create_from_boolean)
{
	json17::object_t<json17::type_config> o1("enabled", true);
	BOOST_ASSERT_MSG(o1["enabled"] == true, "not found");
	BOOST_ASSERT_MSG(o1["disabled"] == json17::type_config::null_t{}, "not found");
}

BOOST_AUTO_TEST_CASE(object_t_create_from_number)
{
	json17::object_t<json17::type_config> o1("numeric", 10);
	BOOST_ASSERT_MSG(o1["numeric"] == 10, "not found");
}

BOOST_AUTO_TEST_CASE(object_t_create_from_unsigned)
{
	json17::object_t<json17::type_config> o1("unsigned", 12U);
	BOOST_ASSERT_MSG(o1["unsigned"] == 12U, "not found");
}

BOOST_AUTO_TEST_CASE(object_t_create_from_float)
{
	json17::object_t<json17::type_config> o1("float", 12.0);
	BOOST_ASSERT_MSG(o1["float"] == 12.0, "not found");
}

BOOST_AUTO_TEST_CASE(object_t_create_from_array)
{
	json17::object_t<json17::type_config> o1("array_int", 1, 2, 3, 4);
	auto rc1 = o1["array_int"] == json17::array_t<json17::type_config>{ 1, 2, 3, 4 };
	BOOST_ASSERT_MSG(rc1, "not found");

	json17::object_t<json17::type_config> o2("array_bool",  true, false, true);
	auto rc2 = o2["array_bool"] == json17::array_t<json17::type_config>{ true, false, true };
	BOOST_ASSERT_MSG(rc2, "not found");

	json17::object_t<json17::type_config> o3("array_float", 1.3, 1. );
	auto rc3 = o3["array_float"] == json17::array_t<json17::type_config>{ 1.3, 1. };
	BOOST_ASSERT_MSG(rc3, "not found");
}

BOOST_AUTO_TEST_CASE(object_t_create_from_string)
{
	json17::object_t<json17::type_config> o1("std::string", "some char[]");
	BOOST_ASSERT_MSG(o1["std::string"] == std::string("some char[]"), "not found");

	json17::object_t<json17::type_config> o2("std::string", std::string("some std::string"));
	BOOST_ASSERT_MSG(o2["std::string"] == std::string("some std::string"), "not found");
}

#endif

#if 1
BOOST_AUTO_TEST_CASE(object_t_create_from_variadic_template)
{
	json17::array_t<json17::type_config> arr1(1, "string", false, 10U, true, std::string("hello"));
	json17::object_t<json17::type_config> o1;
	o1["variadic_array"] = arr1;
	auto rc1 = o1["variadic_array"] == arr1;
	BOOST_ASSERT_MSG(rc1, "not found");
}
#endif

#if 1
BOOST_AUTO_TEST_CASE(object_t_create_from_object)
{
	json17::object_t<json17::type_config> o1("object", { "enabled", true });
	json17::object_t<json17::type_config> obj = o1["object"];
	BOOST_ASSERT_MSG(obj["enabled"] == true, "not found");
}

BOOST_AUTO_TEST_CASE(object_t_create_complex_1)
{
	json17::object_t<json17::type_config> o1({ "config", { { "log" , "some_path" }, { "level" , 4 }, { "enabled", true }, { "rotate", 1,2,3,4,5,6,true } } });

	auto o1_log = o1["config.log"];
	auto o1_level = o1["config.level"];
	auto o1_enabled = o1["config.enabled"];
	auto o1_rotate = o1["config.rotate"];

	BOOST_ASSERT_MSG(o1_log == "some_path", "not found config.log");
	BOOST_ASSERT_MSG(o1_level == 4, "not found config.level");
	BOOST_ASSERT_MSG(o1_enabled == true, "not found config.enabled");
	
	auto rc = o1_rotate == json17::array_t<json17::type_config>{ 1,2,3,4,5,6,true };
	BOOST_ASSERT_MSG(rc, "not found config.rotate");
}

BOOST_AUTO_TEST_CASE(object_t_create_complex_2)
{
	json17::object_t<json17::type_config> o1("config", { {"v1", {{ "log" , "some_path" }} }, { "v2", {{ "level" , 4 },{ "enabled", true }} } });
	
	auto o1_log = o1["config.v1.log"]; 
	auto o1_level = o1["config.v2.level"];
	auto o1_enabled = o1["config.v2.enabled"];

	BOOST_ASSERT_MSG(o1_log == "some_path", "not found config.v1.log");
	BOOST_ASSERT_MSG(o1_level == 4, "not found config.v2.level");
	BOOST_ASSERT_MSG(o1_enabled == true, "not found config.v2.level");
}
#endif
BOOST_AUTO_TEST_SUITE_END()