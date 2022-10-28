//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Testing/BsTestOutput.h"
#include "Prerequisites/BsPrerequisitesUtil.h"

namespace bs
{
	/** @addtogroup Testing
	 *  @{
	 */

	/** Outputs unit test failures to stdout. */
	class BS_UTILITY_EXPORT ConsoleTestOutput : public TestOutput
	{
	public:
		void OutputFail(const String& desc, const String& function, const String& file, long line) override;
	};

	/** @} */
} // namespace bs
