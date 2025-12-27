//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Testing/B3DTestOutput.h"
#include "Error/B3DException.h"

using namespace b3d;

void ExceptionTestOutput::DoOnOutputFail(const String& desc, const String& function, const String& file, long line)
{
	B3D_EXCEPT(UnitTestException, desc);
}
