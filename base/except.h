/**
 * Functions and macros for throwing exceptions while supporting platforms that do not have
 * exceptions.
 *
 * The except() function and its overloads log a message and throw an exception. On platforms
 * without exception support, this exception is treated as an uncaught exception and results in
 * terminating the program.
 */
#pragma once

#ifndef PLATFORM_OS_NONE
#include "base/except_with_glog.h"
#else
#include "base/except_without_glog.h"
#endif
