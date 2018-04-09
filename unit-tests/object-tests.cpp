#include "stdafx.h"

#include <json17/json.h>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(object_tests)

BOOST_AUTO_TEST_CASE(object_t_create_from_boolean)
{
	json17::object_t o1("enabled", true);
	BOOST_ASSERT_MSG(o1.get<json17::boolean_t>("enabled") == true, "not found");
	BOOST_ASSERT_MSG(o1.get<json17::boolean_t>("enabled", false) == true, "not found");
	BOOST_ASSERT_MSG(o1.get<json17::boolean_t>("disabled", false) == false, "not found");
}

BOOST_AUTO_TEST_CASE(object_t_create_from_number)
{
	json17::object_t o1("numeric", 10);
	BOOST_ASSERT_MSG(o1.get<json17::numeric_t>("numeric") == 10, "not found");
}

BOOST_AUTO_TEST_CASE(object_t_create_from_unsigned)
{
	json17::object_t o1("unsigned", 12U);
	BOOST_ASSERT_MSG(o1.get<json17::unsigned_t>("unsigned") == 12U, "not found");
}

BOOST_AUTO_TEST_CASE(object_t_create_from_float)
{
	json17::object_t o1("float", 12.0);
	BOOST_ASSERT_MSG(o1.get<json17::float_t>("float") == 12.0, "not found");
}

BOOST_AUTO_TEST_CASE(object_t_create_from_array)
{
	json17::object_t o1("array_int", {1, 2});
	auto rc1 = o1.get<json17::array_t>("array_int") == json17::array_t{ 1, 2 };
	BOOST_ASSERT_MSG(rc1, "not found");

	json17::object_t o2("array_bool", { true, false, true });
	auto rc2 = o2.get<json17::array_t>("array_bool") == json17::array_t{ true, false, true };
	BOOST_ASSERT_MSG(rc2, "not found");

	json17::object_t o3("array_float", { 1.3, 1. });
	auto rc3 = o3.get<json17::array_t>("array_float") == json17::array_t{ 1.3, 1. };
	BOOST_ASSERT_MSG(rc3, "not found");
}

BOOST_AUTO_TEST_CASE(object_t_create_from_object)
{
	json17::object_t o1("object", { "enabled", true });
	auto obj = o1.get("object");
	BOOST_ASSERT_MSG(obj.get<json17::boolean_t>("enabled"), "not found");
}

BOOST_AUTO_TEST_CASE(object_t_create_from_string)
{
	json17::object_t o1("std::string", "some char[]");
	BOOST_ASSERT_MSG(o1.get<std::string>("std::string") == std::string("some char[]"), "not found");

	json17::object_t o2("std::string",std::string("some std::string"));
	BOOST_ASSERT_MSG(o2.get<std::string>("std::string") == std::string("some std::string"), "not found");
}

BOOST_AUTO_TEST_CASE(object_t_create_complex_1)
{
	json17::object_t o1("config", { {"log" , "som_path"}, {"level" , 4} });
}

BOOST_AUTO_TEST_SUITE_END()