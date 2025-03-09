#pragma once

#if not defined(EMBEDDED_PLATFORM)

#include "gflags/gflags.h"

#else

// On embedded platforms, we do not use gflags. Instead, we define our own macros. These macros only
// define or declare the relevant flag variable. These flags are assigned their default value and
// are defined as const variables. We do not attempt any means of setting the value at run time.

#include <cstdint>

#define DEFINE_VARIABLE(type, name, value) \
  static const type FLAGS_##name { value }

#define DEFINE_bool(name, val, txt) DEFINE_VARIABLE(bool, name, val)
#define DEFINE_int32(name, val, txt) DEFINE_VARIABLE(int32_t, name, val)
#define DEFINE_uint32(name, val, txt) DEFINE_VARIABLE(uint32_t, name, val)
#define DEFINE_int64(name, val, txt) DEFINE_VARIABLE(int64_t, name, val)
#define DEFINE_uint64(name, val, txt) DEFINE_VARIABLE(uint64_t, name, val)
#define DEFINE_double(name, val, txt) DEFINE_VARIABLE(double, name, val)
#define DEFINE_string(name, val, txt) DEFINE_VARIABLE(std::string, name, val)

#define DECLARE_VARIABLE(type, name) extern const type FLAGS_##name

#define DECLARE_bool(name) DECLARE_VARIABLE(bool, name)
#define DECLARE_int32(name) DECLARE_VARIABLE(int32_t, name)
#define DECLARE_uint32(name) DECLARE_VARIABLE(uint32_t, name)
#define DECLARE_int64(name) DECLARE_VARIABLE(int64_t, name)
#define DECLARE_uint64(name) DECLARE_VARIABLE(uint64_t, name)
#define DECLARE_double(name) DECLARE_VARIABLE(double, name)
#define DECLARE_string(name) DECLARE_VARIABLE(std::string, name)

#endif
