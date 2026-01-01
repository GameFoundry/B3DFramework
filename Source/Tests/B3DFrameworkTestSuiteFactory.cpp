//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DFrameworkTestSuiteFactory.h"
#include "Testing/B3DConsoleTestOutput.h"
#include "Testing/B3DJSONTestOutput.h"
#include "Testing/B3DTestSuiteRegistry.h"
#include "B3DApplication.h"

#include "TestSuites/B3DUtilityTestSuite.h"
#include "TestSuites/B3DFileSystemTestSuite.h"
#include "TestSuites/B3DECSTestSuite.h"
#include "TestSuites/B3DCoreTestSuite.h"
#include "TestSuites/B3DPrefabTestSuite.h"

namespace b3d
{
	void FrameworkTestSuiteFactory::StartApplication()
	{
		VideoMode videoMode(1280, 720);
		Application::StartUp(videoMode, "UnitTestRunner", false);
	}

	void FrameworkTestSuiteFactory::ShutdownApplication()
	{
		Application::ShutDown();
	}

	void FrameworkTestSuiteFactory::RegisterTestSuites(TestLayer layer)
	{
		TestSuiteRegistry& registry = TestSuiteRegistry::Instance();

		if (layer == TestLayer::Utility)
		{
			registry.RegisterSuite(TestSuite::Create<UtilityTestSuite>());
			registry.RegisterSuite(TestSuite::Create<FileSystemTestSuite>());
			registry.RegisterSuite(TestSuite::Create<ECSTestSuite>());
		}
		else if (layer == TestLayer::Core)
		{
			registry.RegisterSuite(TestSuite::Create<CoreTestSuite>());
			registry.RegisterSuite(TestSuite::Create<PrefabTestSuite>());
		}
	}

	void FrameworkTestSuiteFactory::RunTests(TestOutputFormat outputFormat, const Path& outputPath, i32& exitCode)
	{
		const Vector<SPtr<TestSuite>>& suites = TestSuiteRegistry::Instance().GetSuites();

		auto fnRunTests = [&suites](TestOutput& output)
		{
			for (const auto& suite : suites)
				suite->Run(output);
		};

		if (outputFormat == TestOutputFormat::JSON)
		{
			Path jsonPath = outputPath.IsEmpty() ? Path("test_results.json") : outputPath;
			JSONTestOutput testOutput(jsonPath);
			fnRunTests(testOutput);
			if (testOutput.GetExitCode() != 0)
				exitCode = testOutput.GetExitCode();
		}
		else
		{
			ConsoleTestOutput testOutput;
			fnRunTests(testOutput);
			if (testOutput.GetExitCode() != 0)
				exitCode = testOutput.GetExitCode();
		}

		TestSuiteRegistry::Instance().Clear();
	}

	i32 FrameworkTestSuiteFactory::Run(TestLayers layers, TestOutputFormat outputFormat, const Path& outputPath)
	{
		i32 exitCode = 0;

		// Utility tests don't need Application
		if (layers.IsSet(TestLayer::Utility))
		{
			RegisterTestSuites(TestLayer::Utility);
			RunTests(outputFormat, outputPath, exitCode);
		}

		// Core (and Editor in subclass) tests need Application
		TestLayers appLayers = layers & (TestLayer::Core | TestLayer::Editor);
		if (appLayers)
		{
			StartApplication();

			if (appLayers.IsSet(TestLayer::Core))
				RegisterTestSuites(TestLayer::Core);

			if (appLayers.IsSet(TestLayer::Editor))
				RegisterTestSuites(TestLayer::Editor);

			RunTests(outputFormat, outputPath, exitCode);
			ShutdownApplication();
		}

		return exitCode;
	}
} // namespace b3d
