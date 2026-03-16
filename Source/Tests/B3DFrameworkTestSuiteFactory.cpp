//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DFrameworkTestSuiteFactory.h"
#include "Testing/B3DTestResultCollector.h"
#include "Testing/B3DTestResultWriter.h"
#include "Testing/B3DTestSuiteRegistry.h"
#include "B3DApplication.h"

#include "TestSuites/B3DUtilityTestSuite.h"
#include "TestSuites/B3DFileSystemTestSuite.h"
#include "TestSuites/B3DECSTestSuite.h"
#include "TestSuites/B3DCoreTestSuite.h"
#include "TestSuites/B3DPrefabTestSuite.h"
#include "TestSuites/B3DSceneObjectTransformTestSuite.h"
#include "TestSuites/B3DRenderableTestSuite.h"

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
			registry.RegisterSuite(TestSuite::Create<SceneObjectTransformTestSuite>());
			registry.RegisterSuite(TestSuite::Create<RenderableTestSuite>());
		}
	}

	void FrameworkTestSuiteFactory::RunTests(TestOutput& output)
	{
		const Vector<SPtr<TestSuite>>& suites = TestSuiteRegistry::Instance().GetSuites();

		for (const auto& suite : suites)
			suite->Run(output);

		TestSuiteRegistry::Instance().Clear();
	}

	i32 FrameworkTestSuiteFactory::Run(TestLayers layers, TestOutputFormat outputFormat, const Path& outputPath)
	{
		TestResultCollector collector;

		// Phase 1: Utility tests (no Application needed)
		if (layers.IsSet(TestLayer::Utility))
		{
			RegisterTestSuites(TestLayer::Utility);
			RunTests(collector);
		}

		// Phase 2: Core/Editor tests (need Application)
		TestLayers appLayers = layers & (TestLayer::Core | TestLayer::Editor);
		if (appLayers)
		{
			StartApplication();

			if (appLayers.IsSet(TestLayer::Core))
				RegisterTestSuites(TestLayer::Core);

			if (appLayers.IsSet(TestLayer::Editor))
				RegisterTestSuites(TestLayer::Editor);

			RunTests(collector);
		}

		// Write results before shutting down, as the console is freed during shutdown
		if (outputFormat == TestOutputFormat::JSON)
		{
			Path jsonPath = outputPath.IsEmpty() ? Path("test_results.json") : outputPath;
			TestResultWriter::WriteToJSON(jsonPath, collector.GetResults());
		}
		else
			TestResultWriter::WriteToConsole(collector.GetResults());

		if (appLayers)
			ShutdownApplication();

		return collector.GetExitCode();
	}
} // namespace b3d

// Plugin exports
extern "C" B3D_PLUGIN_EXPORT b3d::ITestSuiteFactory* CreateFrameworkTestSuiteFactory()
{
	return b3d::B3DNew<b3d::FrameworkTestSuiteFactory>();
}

extern "C" B3D_PLUGIN_EXPORT void DestroyTestSuiteFactory(b3d::ITestSuiteFactory* factory)
{
	b3d::B3DDelete(factory);
}
