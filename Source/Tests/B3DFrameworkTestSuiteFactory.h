//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Testing/B3DTestSuiteFactory.h"
#include "Testing/B3DTestOutput.h"
#include "Utility/B3DDynamicLibrary.h"

#if B3D_PLATFORM == B3D_PLATFORM_WIN32
#	if B3D_COMPILER == B3D_COMPILER_MSVC
#		if defined(B3D_FRAMEWORK_TESTS_EXPORTS)
#			define B3D_FRAMEWORK_TESTS_EXPORT __declspec(dllexport)
#		else
#			define B3D_FRAMEWORK_TESTS_EXPORT __declspec(dllimport)
#		endif
#	else
#		if defined(B3D_FRAMEWORK_TESTS_EXPORTS)
#			define B3D_FRAMEWORK_TESTS_EXPORT __attribute__((dllexport))
#		else
#			define B3D_FRAMEWORK_TESTS_EXPORT __attribute__((dllimport))
#		endif
#	endif
#else
#	define B3D_FRAMEWORK_TESTS_EXPORT __attribute__((visibility("default")))
#endif

namespace b3d
{
	/** Factory for running framework unit tests. */
	class B3D_FRAMEWORK_TESTS_EXPORT FrameworkTestSuiteFactory : public ITestSuiteFactory
	{
	public:
		i32 Run(TestLayers layers, TestOutputFormat outputFormat, const Path& outputPath) override;
		TestLayers GetSupportedLayers() const override { return TestLayer::Utility | TestLayer::Core | TestLayer::Plugins; }

	protected:
		/** Plugin test DLL discovered and loaded by @c DiscoverPluginModules. */
		struct PluginTestModule
		{
			using FnRegisterTestSuites = void (*)();

			DynamicLibrary* Library = nullptr;
			FnRegisterTestSuites Register = nullptr;
		};

		/** Starts up the relevant Application class. */
		virtual void StartApplication();

		/** Shuts down the relevant Application class. */
		virtual void ShutdownApplication();

		/** Override to register additional test suites. Called before tests run for a specific layer. */
		virtual void RegisterTestSuites(TestLayer layer);

		/** Runs all registered tests through the provided output. */
		void RunTests(TestOutput& output);

		/**
		 * Scans the runner's executable directory for sibling DLLs matching @c bsf*Tests and loads any
		 * that export a parameter-less @c RegisterTestSuites C function. Successfully loaded modules are
		 * pushed onto @c mPluginModules for later invocation during the Core phase.
		 */
		void DiscoverPluginModules();

		/** LIFO unloads everything in @c mPluginModules and clears the list. */
		void UnloadPluginModules();

		Vector<PluginTestModule> mPluginModules;
	};
} // namespace b3d
