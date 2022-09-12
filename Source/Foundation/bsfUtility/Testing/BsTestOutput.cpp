//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Testing/BsTestOutput.h"
#include "Error/BsException.h"

namespace bs
{
	void ExceptionTestOutput::OutputFail(const String& desc, const String& function, const String& file, long line)
	{
		BS_EXCEPT(UnitTestException, desc);
	}
}
