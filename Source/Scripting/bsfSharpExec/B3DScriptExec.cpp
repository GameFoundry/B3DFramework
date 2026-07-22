//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DMonoManager.h"
#include "B3DMonoAssembly.h"
#include "B3DMonoClass.h"
#include "FileSystem/B3DFileSystem.h"
#include "B3DEngineScriptLibrary.h"

/** Main entry point into the application. */
#if B3D_PLATFORM_WIN32
#	include <windows.h>

int CALLBACK WinMain(
	_In_ HINSTANCE hInstance,
	_In_ HINSTANCE hPrevInstance,
	_In_ LPSTR lpCmdLine,
	_In_ int nCmdShow)
#else
int main(int __argc, char* __argv[])
#endif
{
	using namespace b3d;

	// No assembly to run, or Mono directory not provided
	if(__argc < 2)
	{
		B3D_LOG(Error, LogScript, "No assembly provided");
		return 0;
	}

	MemStack::BeginThread();
	MonoManager::StartUp();

	TShared<EngineScriptLibrary> library = B3DMakeShared<EngineScriptLibrary>();
	ScriptManager::SetScriptLibrary(library);

	Path engineAssemblyPath = library->GetEngineAssemblyPath();

	auto& monoManager = MonoManager::Instance();
	b3d::MonoAssembly& bsfAssembly = monoManager.LoadAssembly(engineAssemblyPath, kEngineAssembly);
	b3d::MonoAssembly& gameAssembly = monoManager.LoadAssembly(Path(__argv[1]), __argv[1]);
	gameAssembly.Invoke("Program::Start");

	MonoManager::ShutDown();
	MemStack::EndThread();

	return 0;
}
