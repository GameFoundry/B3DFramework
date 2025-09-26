//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Testing/B3DTestOutput.h"
#include "B3DUtilityPrerequisites.h"

namespace b3d
{
	/** @addtogroup Testing
	 *  @{
	 */

	/** Outputs unit test failures to stdout. */
	class B3D_EXPORT ConsoleTestOutput : public TestOutput
	{
	public:
		void OutputFail(const String& desc, const String& function, const String& file, long line) override;
	};

	/** @} */
} // namespace b3d
