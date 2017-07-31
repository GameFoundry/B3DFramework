//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsIBLUtility.h"

namespace bs { namespace ct
{
	const UINT32 IBLUtility::REFLECTION_CUBEMAP_SIZE = 256;
	const UINT32 IBLUtility::IRRADIANCE_CUBEMAP_SIZE = 32;

	const IBLUtility& gIBLUtility()
	{
		return IBLUtility::instance();
	}
}}
