#pragma once

#include "gflags/gflags.h"

DECLARE_string(soapy_url);

// Note: this function does not return unless there is an error, or the server has been stopped.
int run_soapy_server();
