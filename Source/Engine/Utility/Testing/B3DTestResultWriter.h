//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DUtilityPrerequisites.h"
#include "Testing/B3DTestResultCollector.h"
#include "FileSystem/B3DPath.h"

namespace b3d
{
	/** @addtogroup Testing
	 *  @{
	 */

	/** Static utility class for writing test results to various formats. */
	class B3D_EXPORT TestResultWriter
	{
	public:
		/**
		 * Writes test results to a JSON file.
		 *
		 * @param	outputPath	Path to the output JSON file.
		 * @param	results		Test suite results to write.
		 */
		static void WriteToJSON(const Path& outputPath, const Vector<TestSuiteResult>& results);

		/**
		 * Writes test results to stdout with colored output.
		 *
		 * @param	results		Test suite results to write.
		 */
		static void WriteToConsole(const Vector<TestSuiteResult>& results);
	};

	/** @} */
} // namespace b3d
