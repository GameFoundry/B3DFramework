//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

// This file's only purpose is to serve as an include guard for cpuid.h by checking for two of it's internal macros
#if  !(defined(__cpuid) && defined(__cpuid_count))
#include "cpuid.h"
#endif
