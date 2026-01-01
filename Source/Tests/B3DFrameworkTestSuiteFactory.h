//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Testing/B3DTestSuiteFactory.h"

namespace b3d
{
	/**
	 * Factory for running Framework tests (Utility + Core layers).
	 * Handles Application lifecycle for Core tests.
	 */
	class FrameworkTestSuiteFactory : public ITestSuiteFactory
	{
	public:
		i32 Run(TestLayers layers, TestOutputFormat outputFormat, const Path& outputPath) override;

		TestLayers GetSupportedLayers() const override
		{
			return TestLayer::Utility | TestLayer::Core;
		}

	protected:
		/** Starts up the relevant Application class. */
		virtual void StartApplication();

		/** Shuts down the relevant Application class. */
		virtual void ShutdownApplication();

		/** Override to register additional test suites. Called before tests run for a specific layer. */
		virtual void RegisterTestSuites(TestLayer layer);

		/** Runs all registered tests with the configured output. */
		void RunTests(TestOutputFormat outputFormat, const Path& outputPath, i32& exitCode);
	};
} // namespace b3d
