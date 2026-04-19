//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Testing/B3DTestSuiteFactory.h"
#include "Testing/B3DTestSuiteRegistry.h"
#include "Utility/B3DCommandLine.h"
#include "Utility/B3DDynamicLibrary.h"
#include "String/B3DString.h"

#include <iostream>

using namespace b3d;

typedef ITestSuiteFactory* (*FnCreateFactory)();
typedef void (*FnDestroyFactory)(ITestSuiteFactory*);

/** Parses layer string into TestLayers flags. */
static TestLayers ParseLayers(const String& layerStr)
{
	if (layerStr == "all")
		return TestLayer::Utility | TestLayer::Core | TestLayer::Editor;
	if (layerStr == "utility")
		return TestLayer::Utility;
	if (layerStr == "core")
		return TestLayer::Core;
	if (layerStr == "editor")
		return TestLayer::Editor;

	// Support comma-separated: "utility,core"
	TestLayers result;
	Vector<String> parts = StringUtility::Split(layerStr, ",");
	for (const String& part : parts)
	{
		String trimmed = StringUtility::Trim(part);
		if (trimmed == "utility")
			result.Set(TestLayer::Utility);
		else if (trimmed == "core")
			result.Set(TestLayer::Core);
		else if (trimmed == "editor")
			result.Set(TestLayer::Editor);
	}
	return result;
}

/** Parses output format string into TestOutputFormat enum. */
static TestOutputFormat ParseOutputFormat(const String& formatStr)
{
	if (formatStr == "json")
		return TestOutputFormat::JSON;

	return TestOutputFormat::Console;
}

int main(int argc, char* argv[])
{
	CrashHandler::StartUp();
	CommandLine::Initialize(argc, argv);

	String formatStr = CommandLine::GetParameterValue("test-output-format", "console");
	String outputPathStr = CommandLine::GetParameterValue("test-output-path", "");
	String layerStr = CommandLine::GetParameterValue("test-layer", "all");

	TestLayers layers = ParseLayers(layerStr);
	TestOutputFormat outputFormat = ParseOutputFormat(formatStr);
	Path outputPath = outputPathStr.empty() ? Path() : Path(outputPathStr);

	TestSuiteRegistry::StartUp();

	DynamicLibrary* testLibrary = nullptr;
	ITestSuiteFactory* testFactory = nullptr;
	FnDestroyFactory fnDestroyFactory = nullptr;
	i32 exitCode = 0;

	// Try to load EditorTests.dll first (if editor tests requested or running all)
	// EditorTestSuiteFactory can run all tests (utility, core, editor)
	if (layers.IsSet(TestLayer::Editor))
	{
		try
		{
			testLibrary = B3DNew<DynamicLibrary>("EditorTests");
			testLibrary->Load();

			auto fnCreateFactory = reinterpret_cast<FnCreateFactory>(testLibrary->GetSymbol("CreateEditorTestSuiteFactory"));
			fnDestroyFactory = reinterpret_cast<FnDestroyFactory>(testLibrary->GetSymbol("DestroyTestSuiteFactory"));

			if (fnCreateFactory && fnDestroyFactory)
				testFactory = fnCreateFactory();
		}
		catch (...)
		{
			// EditorTests not available
			if (testLibrary)
			{
				B3DDelete(testLibrary);
				testLibrary = nullptr;
			}
		}

		if (!testFactory)
		{
			std::cerr << "Warning: EditorTests not available, skipping editor tests." << std::endl;
			layers = layers & ~TestLayer::Editor;
		}
	}

	// Fall back to FrameworkTests.dll for utility+core tests
	if (!testFactory && layers)
	{
		try
		{
			testLibrary = B3DNew<DynamicLibrary>("FrameworkTests");
			testLibrary->Load();

			auto fnCreateFactory = reinterpret_cast<FnCreateFactory>(testLibrary->GetSymbol("CreateFrameworkTestSuiteFactory"));
			fnDestroyFactory = reinterpret_cast<FnDestroyFactory>(testLibrary->GetSymbol("DestroyTestSuiteFactory"));

			if (fnCreateFactory && fnDestroyFactory)
				testFactory = fnCreateFactory();
		}
		catch (...)
		{
			std::cerr << "Error: Failed to load FrameworkTests library." << std::endl;
		}
	}

	// Run tests
	if (testFactory && layers)
	{
		exitCode = testFactory->Run(layers, outputFormat, outputPath);
		fnDestroyFactory(testFactory);
	}

	// Cleanup
	if (testLibrary)
	{
		testLibrary->Unload();
		B3DDelete(testLibrary);
	}

	TestSuiteRegistry::ShutDown();
	CrashHandler::ShutDown();

	return exitCode;
}
