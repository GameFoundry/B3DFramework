//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
//
// Headless, deviceless asset cook tool. Shader cooking is its first job: it stands up the engine on the Null GPU
// backend (no real device) and registers the real GPU bytecode compilers so shader bytecode (for example SPIR-V) can
// be baked ahead of time. This Phase 2 scaffold only stands up the pipeline and verifies it; the cook orchestration
// lands in Phase 3.
#include "B3DApplication.h"
#include "Material/B3DShaderCompiler.h"
#include "GpuBackend/B3DGpuProgram.h"
#include "GpuBackend/B3DGpuBackend.h"
#include "Utility/B3DCommandLine.h"
#include "String/B3DString.h"
#include "Debug/B3DDebug.h"

using namespace b3d;

namespace
{
	/** Splits a comma-separated option value into trimmed, non-empty entries (for example "vksl, nullsl" -> {"vksl", "nullsl"}). */
	Vector<String> ParseList(const String& value)
	{
		Vector<String> result;
		for(const String& part : StringUtility::Split(value, ","))
		{
			const String trimmed = StringUtility::Trim(part);
			if(!trimmed.empty())
				result.push_back(trimmed);
		}

		return result;
	}

	/** Joins the provided strings into a single comma-separated string, for logging. */
	String JoinStrings(const Vector<String>& values)
	{
		String result;
		for(size_t i = 0; i < values.size(); ++i)
		{
			if(i > 0)
				result += ", ";

			result += values[i];
		}

		return result;
	}

	/**
	 * Compiles a trivial shader to SPIR-V through the registered vksl bytecode compiler, confirming the headless,
	 * deviceless bytecode path is wired end-to-end. Returns 0 on success, 1 on failure.
	 */
	int RunBytecodeSmokeTest()
	{
		const TShared<IGpuBytecodeCompiler> compiler = ShaderCompilers::Instance().GetBytecodeCompiler(kGpuProgramLanguageVksl);
		if(compiler == nullptr)
		{
			B3D_LOG(Error, LogGeneric, "Smoke test failed: no bytecode compiler registered for '{0}'.", kGpuProgramLanguageVksl);
			return 1;
		}

		GpuProgramCreateInformation programInfo;
		programInfo.Name = "CookToolSmokeTest";
		programInfo.EntryPoint = "main";
		programInfo.Language = kGpuProgramLanguageVksl;
		programInfo.Type = GPT_VERTEX_PROGRAM;
		programInfo.Source =
			"#version 450\n"
			"void main()\n"
			"{\n"
			"\tgl_Position = vec4(0.0, 0.0, 0.0, 1.0);\n"
			"}\n";

		const TShared<GpuProgramBytecode> bytecode = compiler->CompileBytecode(programInfo);
		if(bytecode == nullptr || bytecode->Instructions.Size == 0)
		{
			const String messages = bytecode != nullptr ? bytecode->Messages : String("compiler returned null");
			B3D_LOG(Error, LogGeneric, "Smoke test failed: '{0}' produced no bytecode. {1}", kGpuProgramLanguageVksl, messages);
			return 1;
		}

		B3D_LOG(Info, LogGeneric, "Smoke test passed: '{0}' compiled {1} bytes of SPIR-V (compiler '{2}' v{3}).",
			kGpuProgramLanguageVksl, bytecode->Instructions.Size, bytecode->CompilerId, bytecode->CompilerVersion);
		return 0;
	}
}

int main(int argc, char* argv[])
{
	CommandLine::Initialize(argc, argv);

	// CLI stub for the eventual cook (Phase 3): an output directory, the set of shading languages to cook, and the set
	// of low-level bytecode targets to bake. -lang/-bytecode each accept a comma-separated list (for example
	// "-lang vksl,nullsl"). The cook consumes these later; for now we only stand up and exercise the headless,
	// deviceless bytecode pipeline.
	const Path outputFolder = CommandLine::GetParameterValue("output");

	Vector<String> languages = ParseList(CommandLine::GetParameterValue("lang"));
	if(languages.empty())
		languages.push_back(kGpuProgramLanguageVksl);

	Vector<String> bytecodeTargets = ParseList(CommandLine::GetParameterValue("bytecode"));
	if(bytecodeTargets.empty())
		bytecodeTargets.push_back(kGpuProgramLanguageVksl);

	const bool force = CommandLine::HasParameter("force");

	// Headless, deviceless start-up: the cook runs on the Null GPU backend (no real device) paired with the Null
	// renderer (it does no rendering), regardless of the runtime backend/renderer the engine was configured with. This
	// mirrors the asset import tool's headless start-up. The crash handler is owned by Application (it is started as the
	// first thing in start-up, so it already covers engine initialization); the cook tool is non-interactive, so request
	// popup suppression so a fatal error writes the log + crash dump and terminates instead of blocking on a modal box.
	ApplicationCreateInformation createInformation = Application::BuildCreateInformation(VideoMode(64, 64), "Banshee Cook Tool", false);
	createInformation.GpuBackend = "bsfNullGpuBackend";
	createInformation.Renderer = "bsfNullRenderer";
	createInformation.PrimaryWindow.Headless = true;
	createInformation.CrashHandling.SuppressErrorPopup = true;

	Application::StartUp(createInformation);

	// Start-up loads bsfSL (one of the default importers), which registers every built GPU bytecode compiler (for example
	// vksl -> SPIR-V) into the shared ShaderCompilers module, so the cook can bake shader bytecode without a device.

	B3D_LOG(Info, LogGeneric, "Banshee Cook Tool started (output '{0}', force {1}). Cook languages: [{2}]; bytecode targets: [{3}].",
		outputFolder.ToString(), force ? "yes" : "no", JoinStrings(languages), JoinStrings(bytecodeTargets));

	int exitCode = 0;

	// Phase 2 smoke test (opt-in via -smoketest): verify the deviceless bytecode path end-to-end. The real cook
	// orchestration replaces this in Phase 3.
	if(CommandLine::HasParameter("smoketest"))
		exitCode = RunBytecodeSmokeTest();

	Application::ShutDown();
	return exitCode;
}
