//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsPlatformDefines.h"

/**	Commonly used types. */
namespace bs
{
	typedef char INT8;
	typedef unsigned char UINT8;
	typedef short INT16;
	typedef unsigned short UINT16;
	typedef int INT32;
	typedef unsigned int UINT32;

#if BS_COMPILER == BS_COMPILER_MSVC
	typedef unsigned __int64 UINT64;
	typedef __int64 INT64;
#else
	typedef unsigned long long UINT64;
	typedef long long INT64;
#endif
}