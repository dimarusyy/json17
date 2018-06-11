#pragma once

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>

#define _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS

#include <boost/test/unit_test.hpp>
#if (BOOST_VERSION < 105900)
#include <boost/test/unit_test_suite_impl.hpp>
#endif
#include <boost/test/results_collector.hpp>
#include <boost/test/utils/basic_cstring/io.hpp>
#include <boost/test/utils/runtime/cla/parser.hpp>
#include <boost/test/unit_test_log.hpp>
#include <boost/test/unit_test_log_formatter.hpp>

