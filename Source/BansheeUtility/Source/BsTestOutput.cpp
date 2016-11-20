//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsTestOutput.h"
#include "BsException.h"

namespace bs
{
	void ExceptionTestOutput::outputFail(const String& desc, const String& function, const String& file, long line)
	{
		BS_EXCEPT(UnitTestException, desc);
	}
}
