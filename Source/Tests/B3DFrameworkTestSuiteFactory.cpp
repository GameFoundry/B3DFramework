//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DFrameworkTestSuiteFactory.h"
#include "Testing/B3DTestResultCollector.h"
#include "Testing/B3DTestResultWriter.h"
#include "Testing/B3DTestSuiteRegistry.h"
#include "FileSystem/B3DFileSystem.h"
#include "FileSystem/B3DPath.h"
#include "B3DApplication.h"
#include "GpuBackend/B3DGpuBackend.h"

#include "TestSuites/B3DUtilityTestSuite.h"
#include "TestSuites/B3DFileSystemTestSuite.h"
#include "TestSuites/B3DECSTestSuite.h"
#include "TestSuites/B3DCoreTestSuite.h"
#include "TestSuites/B3DGpuBackendTestSuite.h"
#include "TestSuites/B3DGpuImageNativeStateTestSuite.h"
#include "TestSuites/B3DGpuAllocatorTestSuite.h"
#include "TestSuites/B3DPrefabTestSuite.h"
#include "TestSuites/B3DSceneObjectTransformTestSuite.h"
#include "TestSuites/B3DRenderableTestSuite.h"
#include "TestSuites/B3DImporterTestSuite.h"
#include "TestSuites/B3DGUIStyleSheetTestSuite.h"

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
			registry.RegisterSuite(TestSuite::Create<GpuImageNativeStateTestSuite>());
			registry.RegisterSuite(TestSuite::Create<GpuBackendTestSuite>());
			registry.RegisterSuite(TestSuite::Create<GpuAllocatorTestSuite>());
			registry.RegisterSuite(TestSuite::Create<PrefabTestSuite>());
			registry.RegisterSuite(TestSuite::Create<SceneObjectTransformTestSuite>());
			registry.RegisterSuite(TestSuite::Create<RenderableTestSuite>());
			registry.RegisterSuite(TestSuite::Create<ImporterTestSuite>());
			registry.RegisterSuite(TestSuite::Create<GUIStyleSheetTestSuite>());
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
		// Resolved through the file system rather than the command line, as argv[0] may be a bare relative name
		const Path executableDir = FileSystem::GetExecutableFolderPath();

		Vector<Path> files;
		Vector<Path> directories;
		FileSystem::GetChildren(executableDir, files, directories);

		const String expectedExtension = String(".") + DynamicLibrary::kExtension;

		// Every GPU backend plugin built for this platform drops its test DLL next to the runner, but only one
		// backend is ever started up per run. A foreign backend's suites would exercise a backend that was never
		// initialized, so they are skipped rather than loaded. The running backend is asked for its own name (which
		// is why discovery happens after start-up); non-GPU-backend plugin tests are backend-agnostic and always load.
		const String activeGpuBackend = GpuBackend::Instance().GetBackendName();

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

			// A plugin test DLL is named <pluginTarget>Tests, so stripping the suffix yields the plugin it
			// belongs to, which for GPU backends is the same name the backend is selected by.
			const String pluginName = filename.substr(0, filename.size() - 5);
			if (StringUtility::EndsWith(pluginName, "GpuBackend", false) && pluginName != activeGpuBackend)
			{
				B3D_LOG(Log, LogGeneric, "Skipping test library '{0}': its GPU backend is not the active one ('{1}').", filename, activeGpuBackend);
				continue;
			}

			// Pass the full file path so the library loader doesn't have to apply lib-prefix or extension
			// fix-up itself.
			PluginTestModule module;
			module.Library = B3DNew<DynamicLibrary>(candidate.ToString());
			if (!module.Library->IsLoaded())
			{
				B3D_LOG(Warning, LogGeneric, "Failed to load plugin test library: {0}", candidate.ToString());
				B3DDelete(module.Library);
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

			// Discovery selects test DLLs by asking the running GPU backend which one it is, so it has to follow
			// start-up. Its only consumer is the RegisterTestSuites(Plugins) call below, which is still later.
			if (appLayers.IsSet(TestLayer::Plugins))
				DiscoverPluginModules();

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
