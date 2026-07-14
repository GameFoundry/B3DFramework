//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DFrameworkTestSuiteFactory.h"
#include "Testing/B3DTestResultCollector.h"
#include "Testing/B3DTestResultWriter.h"
#include "Testing/B3DTestSuiteRegistry.h"
#include "Utility/B3DCommandLine.h"
#include "FileSystem/B3DFileSystem.h"
#include "FileSystem/B3DPath.h"
#include "B3DApplication.h"

#include "TestSuites/B3DUtilityTestSuite.h"
#include "TestSuites/B3DFileSystemTestSuite.h"
#include "TestSuites/B3DECSTestSuite.h"
#include "TestSuites/B3DCoreTestSuite.h"
#include "TestSuites/B3DGpuBackendTestSuite.h"
#include "TestSuites/B3DGpuAllocatorTestSuite.h"
#include "TestSuites/B3DPrefabTestSuite.h"
#include "TestSuites/B3DSceneObjectTransformTestSuite.h"
#include "TestSuites/B3DRenderableTestSuite.h"
#include "TestSuites/B3DImporterTestSuite.h"

#include "Debug/B3DDebug.h"

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
			registry.RegisterSuite(TestSuite::Create<GpuBackendTestSuite>());
			registry.RegisterSuite(TestSuite::Create<GpuAllocatorTestSuite>());
			registry.RegisterSuite(TestSuite::Create<PrefabTestSuite>());
			registry.RegisterSuite(TestSuite::Create<SceneObjectTransformTestSuite>());
			registry.RegisterSuite(TestSuite::Create<RenderableTestSuite>());
			registry.RegisterSuite(TestSuite::Create<ImporterTestSuite>());
		}
		else if (layer == TestLayer::Plugins)
		{
			// Plugin test DLLs discovered earlier in Run() each register their own TestSuite
			// instances into the shared registry. Plugin suites depend on Application + GpuBackend
			// so they run alongside Core/Editor inside the application phase, but registration is
			// independent of TestLayer::Core to allow opting in to plugin-only runs.
			for (const PluginTestModule& module : mPluginModules)
				module.Register();
		}
	}

	void FrameworkTestSuiteFactory::DiscoverPluginModules()
	{
		const Path executableDir = CommandLine::GetExecutablePath().GetParent();

		Vector<Path> files;
		Vector<Path> directories;
		FileSystem::GetChildren(executableDir, files, directories);

		const String expectedExtension = String(".") + DynamicLibrary::kExtension;

		for (const Path& candidate : files)
		{
			const String filename = candidate.GetFilename(false);
			const String extension = candidate.GetExtension();

			// Glob equivalent: bsf*Tests.<dll/so/dylib>. Auto-skips FrameworkTests/EditorTests, the
			// runner exe itself, and anything else that doesn't match the plugin-test naming pattern.
			if (extension != expectedExtension)
				continue;
			if (filename.size() < 8 /* "bsf" + at least one char + "Tests" */)
				continue;
			if (filename.compare(0, 3, "bsf") != 0)
				continue;
			if (filename.compare(filename.size() - 5, 5, "Tests") != 0)
				continue;

			PluginTestModule module;
			try
			{
				// Pass the full file path so DynamicLibrary::Load doesn't have to apply lib-prefix or
				// extension fix-up itself.
				module.Library = B3DNew<DynamicLibrary>(candidate.ToString());
				module.Library->Load();
			}
			catch (...)
			{
				B3D_LOG(Warning, LogGeneric, "Failed to load plugin test library: {0}", candidate.ToString());
				if (module.Library != nullptr)
				{
					B3DDelete(module.Library);
					module.Library = nullptr;
				}
				continue;
			}

			module.Register = reinterpret_cast<PluginTestModule::FnRegisterTestSuites>(
				module.Library->GetSymbol("RegisterTestSuites"));

			if (module.Register == nullptr)
			{
				B3D_LOG(Warning, LogGeneric, "Plugin test library '{0}' does not export RegisterTestSuites; skipping.", filename);
				module.Library->Unload();
				B3DDelete(module.Library);
				continue;
			}

			mPluginModules.push_back(module);
		}
	}

	void FrameworkTestSuiteFactory::UnloadPluginModules()
	{
		for (auto it = mPluginModules.rbegin(); it != mPluginModules.rend(); ++it)
		{
			if (it->Library != nullptr)
			{
				it->Library->Unload();
				B3DDelete(it->Library);
			}
		}
		mPluginModules.clear();
	}

	void FrameworkTestSuiteFactory::RunTests(TestOutput& output)
	{
		const Vector<TShared<TestSuite>>& suites = TestSuiteRegistry::Instance().GetSuites();

		for (const auto& suite : suites)
			suite->Run(output);

		TestSuiteRegistry::Instance().Clear();
	}

	i32 FrameworkTestSuiteFactory::Run(TestLayers layers, TestOutputFormat outputFormat, const Path& outputPath)
	{
		TestResultCollector collector;

		// Discover plugin test DLLs up front so they are visible before any layer dispatches. The
		// later RegisterTestSuites(Plugins) call uses mPluginModules, so this must happen before
		// the application phase below.
		if (layers.IsSet(TestLayer::Plugins))
			DiscoverPluginModules();

		// Phase 1: Utility tests (no Application needed)
		if (layers.IsSet(TestLayer::Utility))
		{
			RegisterTestSuites(TestLayer::Utility);
			RunTests(collector);
		}

		// Phase 2: Core/Editor/Plugins tests (need Application). Plugins is treated as an application
		// layer in its own right so `--test-layer plugins` runs plugin suites without also pulling in
		// the framework Core suite registrations.
		TestLayers appLayers = layers & (TestLayer::Core | TestLayer::Editor | TestLayer::Plugins);
		if (appLayers)
		{
			StartApplication();

			if (appLayers.IsSet(TestLayer::Core))
				RegisterTestSuites(TestLayer::Core);

			if (appLayers.IsSet(TestLayer::Editor))
				RegisterTestSuites(TestLayer::Editor);

			if (appLayers.IsSet(TestLayer::Plugins))
				RegisterTestSuites(TestLayer::Plugins);

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

		// Plugin DLLs must be unloaded while the Application is still alive: the test suites they
		// registered hold backend objects (GpuDevice, command buffers) that need the application's
		// teardown order to free correctly. Unload here, then shut down the application.
		UnloadPluginModules();

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
