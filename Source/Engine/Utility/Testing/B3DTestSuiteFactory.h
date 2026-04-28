//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DUtilityPrerequisites.h"
#include "Testing/B3DTestSuite.h"
#include "Utility/B3DFlags.h"
#include "FileSystem/B3DPath.h"

namespace b3d
{
	/** @addtogroup Testing
	 *  @{
	 */

	/**
	 * Specifies which layer of tests to run. Tests are organized by layer to allow running
	 * subsets of tests without full engine initialization.
	 */
	enum class TestLayer
	{
		Utility = 1 << 0, /**< Utility layer tests (no Application required). */
		Core    = 1 << 1, /**< Core layer tests (requires Application). */
		Editor  = 1 << 2, /**< Editor layer tests (requires EditorApplication). */
		Plugins = 1 << 3  /**< Discover and load plugin test DLLs (e.g. @c bsfVulkanGpuBackendTests.dll) and run their suites alongside Core/Editor tests. Plugin suites are registered into the Core phase, so they require an @c Application. */
	};

	using TestLayers = Flags<TestLayer>;
	B3D_FLAGS_OPERATORS(TestLayer)

	/** Specifies the output format for test results. */
	enum class TestOutputFormat
	{
		Console, /**< Output to console. */
		JSON     /**< Output to JSON file. */
	};

	/**
	 * Factory interface for running tests. The factory handles the full test lifecycle including
	 * Application startup/shutdown if required. Implementations should be provided for different
	 * application types (Framework vs Editor).
	 */
	class B3D_EXPORT ITestSuiteFactory
	{
	public:
		virtual ~ITestSuiteFactory();

		/**
		 * Runs tests for the specified layers. Handles all Application lifecycle internally.
		 *
		 * @param	layers			Combination of test layers to run.
		 * @param	outputFormat	Output format for test results.
		 * @param	outputPath		Path for output file (used with JSON format).
		 * @return					Exit code (0 for success, non-zero for failures).
		 */
		virtual i32 Run(TestLayers layers, TestOutputFormat outputFormat, const Path& outputPath) = 0;

		/** Returns which layers this factory can handle. */
		virtual TestLayers GetSupportedLayers() const = 0;
	};

	/** @} */
} // namespace b3d
