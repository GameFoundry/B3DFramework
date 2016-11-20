//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsUUID.h"
#include "BsPlatform.h"
#include <chrono>

using namespace std::chrono;

namespace bs
{
	String UUIDGenerator::generateRandom()
	{
		return PlatformUtility::generateUUID();
	}
};