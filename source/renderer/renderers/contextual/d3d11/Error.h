#pragma once

#include "system/Logger.h"

// WIP: error handling and void __stdcall
#define ABORT_IF_FAILED(hrcall) CLOG_ABORT(FAILED(hrcall) < 0, "d3d11 failure!");
