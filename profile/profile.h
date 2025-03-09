#pragma once

#if !defined(PROFILING_ENABLED)

// Without PROFILING_ENABLED, these calls are no-ops, and we do not include the header dependency.
// See .bazelrc for the profiling configuration details.

namespace tvsc::profile {
inline void profiler_start(const char* filename = "profile.prof") {}
inline void profiler_stop() {}
}  // namespace tvsc::profile

#else

// With PROFILING_ENABLED, we include the header from gperftools and call into that library.

#include <gperftools/profiler.h>

namespace tvsc::profile {
inline void profiler_start(const char* filename = "profile.prof") { ProfilerStart(filename); }
inline void profiler_stop() { ProfilerStop(); }
}  // namespace tvsc::profile

#endif
