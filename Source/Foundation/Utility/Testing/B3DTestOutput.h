//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DUtilityPrerequisites.h"

namespace b3d
{
	/** @addtogroup Testing
	 *  @{
	 */

	/** Abstract interface used for outputting unit test results. */
	class B3D_UTILITY_EXPORT TestOutput
	{
	public:
		virtual ~TestOutput() {}

		/**
		 * Triggered when a unit test fails.
		 *
		 * @param[in]	desc		Reason why the unit test failed.
		 * @param[in]	function	Name of the function the test failed in.
		 * @param[in]	file		File the unit test failed in.
		 * @param[in]	line		Line of code the unit test failed on.
		 */
		virtual void OutputFail(const String& desc, const String& function, const String& file, long line) = 0;
	};

	/** Outputs unit test results so that failures are reported as exceptions. Success is not reported. */
	class B3D_UTILITY_EXPORT ExceptionTestOutput : public TestOutput
	{
	public:
		void OutputFail(const String& desc, const String& function, const String& file, long line) final;
	};

	/** @} */
} // namespace b3d
