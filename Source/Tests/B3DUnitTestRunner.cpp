//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Testing/B3DTestSuiteFactory.h"
#include "Testing/B3DTestSuiteRegistry.h"
#include "Utility/B3DCommandLine.h"
#include "Utility/B3DDynamicLibrary.h"
#include "String/B3DString.h"
#include "B3DFrameworkTestSuiteFactory.h"

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

	ITestSuiteFactory* factory = nullptr;
	FnDestroyFactory fnDestroyFactory = nullptr;
	DynamicLibrary* editorLibrary = nullptr;

	// Try to load EditorTestSuiteFactory if editor tests requested
	bool needsEditor = layers.IsSet(TestLayer::Editor);

	if (needsEditor)
	{
		editorLibrary = B3DNew<DynamicLibrary>("EditorCore");
		editorLibrary->Load();

		auto fnCreateFactory = reinterpret_cast<FnCreateFactory>(editorLibrary->GetSymbol("CreateEditorTestSuiteFactory"));
		fnDestroyFactory = reinterpret_cast<FnDestroyFactory>(editorLibrary->GetSymbol("DestroyTestSuiteFactory"));

		if (fnCreateFactory != nullptr && fnDestroyFactory != nullptr)
			factory = fnCreateFactory();
	}

	// Fallback to framework factory
	if (!factory)
	{
		if (needsEditor && layerStr == "editor")
		{
			std::cerr << "Error: EditorCore not available for editor tests." << std::endl;
			TestSuiteRegistry::ShutDown();
			CrashHandler::ShutDown();
			return 1;
		}

		factory = B3DNew<FrameworkTestSuiteFactory>();
		fnDestroyFactory = [](ITestSuiteFactory* factory) { B3DDelete(factory); };

		// Remove editor layer if not available
		layers = layers & (TestLayer::Utility | TestLayer::Core);
	}

	const i32 exitCode = factory->Run(layers, outputFormat, outputPath);

	fnDestroyFactory(factory);
	if (editorLibrary)
	{
		editorLibrary->Unload();
		B3DDelete(editorLibrary);
	}

	TestSuiteRegistry::ShutDown();
	CrashHandler::ShutDown();

	return exitCode;
}
