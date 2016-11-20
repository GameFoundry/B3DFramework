//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsTestOutput.h"
#include "BsPrerequisitesUtil.h"

namespace bs
{
	/** @addtogroup Testing
	 *  @{
	 */

	/** Outputs unit test failures to stdout. */
	class BS_UTILITY_EXPORT ConsoleTestOutput : public TestOutput
	{
	public:
		/** @copydoc TestOutput::outputFail */
		void outputFail(const String& desc,
		                const String& function,
		                const String& file,
		                long line) final override;
	};

	/** @} */
}
