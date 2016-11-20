//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsPlatformUtility.h"

#include <stdlib.h>

namespace bs
{
	void PlatformUtility::terminate(bool force)
	{
		exit(0);
	}
}
