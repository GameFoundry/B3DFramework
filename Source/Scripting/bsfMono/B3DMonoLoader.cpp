//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DMonoLoader.h"
#include "Utility/B3DDynamicLibraryManager.h"
#include "Utility/B3DDynamicLibrary.h"

#if B3D_PLATFORM_LINUX || B3D_PLATFORM_MACOS
#include <csignal>
#endif

// Function pointer variable definitions
#define MONO_API_FUNCTION(ret, name, args) FNPTR_##name name;
#include "B3DMonoLoaderFunctions.h"
#undef MONO_API_FUNCTION

using namespace b3d;

#if B3D_PLATFORM_LINUX || B3D_PLATFORM_MACOS
namespace
{
	/** Signal handlers that were active before the Mono runtime installed its own. */
	struct sigaction gSavedSignals[NSIG];
} // namespace
#endif

void MonoLoader::Load()
{
	if(!B3D_ENSURE(mLibrary == nullptr))
		return;

#if B3D_PLATFORM_LINUX || B3D_PLATFORM_MACOS
	for(i32 signal = 1; signal < NSIG; signal++)
		sigaction(signal, nullptr, &gSavedSignals[signal]);
#endif

	DynamicLibrary* const library = DynamicLibraryManager::Instance().Load("coreclr");
	if(!B3D_ENSURE(library))
		return;

	// Function loading
	#define MONO_API_FUNCTION(ret, name, args) name = reinterpret_cast<FNPTR_##name>(library->GetSymbol(#name));
	#include "B3DMonoLoaderFunctions.h"
	#undef MONO_API_FUNCTION

	mLibrary = library;
}

void MonoLoader::Unload()
{
	if(mLibrary != nullptr)
	{
#if B3D_PLATFORM_LINUX || B3D_PLATFORM_MACOS
		// The runtime doesn't uninstall its signal handlers on cleanup, and they're about to get unmapped. A fault after
		// that would jump into unmapped code, fault again, and hang instead of crashing.
		for(i32 signal = 1; signal < NSIG; signal++)
		{
			if(signal != SIGKILL && signal != SIGSTOP)
				sigaction(signal, &gSavedSignals[signal], nullptr);
		}
#endif

		// Clear all function pointers
		#define MONO_API_FUNCTION(ret, name, args) name = nullptr;
		#include "B3DMonoLoaderFunctions.h"
		#undef MONO_API_FUNCTION

		DynamicLibraryManager::Instance().Unload(mLibrary);
		mLibrary = nullptr;
	}
}

