//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsMonoManager.h"
#include "BsMonoAssembly.h"
#include "BsMonoClass.h"
#include "FileSystem/BsFileSystem.h"
#include "BsEngineScriptLibrary.h"

/** Main entry point into the application. */
#if BS_PLATFORM == BS_PLATFORM_WIN32
#include <windows.h>

int CALLBACK WinMain(
	_In_  HINSTANCE hInstance,
	_In_  HINSTANCE hPrevInstance,
	_In_  LPSTR lpCmdLine,
	_In_  int nCmdShow
)
#else
int main(int __argc, char* __argv[])
#endif
{
	using namespace bs;

	// No assembly to run, or Mono directory not provided
	if(__argc < 2)
	{
		BS_LOG(Error, Script, "No assembly provided");
		return 0;
	}

	MemStack::beginThread();
	MonoManager::startUp();

	SPtr<EngineScriptLibrary> library = bs_shared_ptr_new<EngineScriptLibrary>();
	ScriptManager::_setScriptLibrary(library);

	Path engineAssemblyPath = library->getEngineAssemblyPath();

	auto& monoManager = MonoManager::instance();
	bs::MonoAssembly& bsfAssembly = monoManager.loadAssembly(engineAssemblyPath, ENGINE_ASSEMBLY);
	bs::MonoAssembly& gameAssembly = monoManager.loadAssembly(Path(__argv[1]), __argv[1]);
	gameAssembly.invoke("Program::Start");

	MonoManager::shutDown();
	MemStack::endThread();

	return 0;
}
