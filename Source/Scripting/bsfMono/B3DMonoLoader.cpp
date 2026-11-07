//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DMonoLoader.h"
#include "Utility/B3DDynamicLibraryManager.h"
#include "Utility/B3DDynamicLibrary.h"

// Function pointer variable definitions
#define MONO_API_FUNCTION(ret, name, args) FNPTR_##name name;
#include "B3DMonoLoaderFunctions.h"
#undef MONO_API_FUNCTION

using namespace b3d;

void MonoLoader::Load()
{
	if(!B3D_ENSURE(mLibrary == nullptr))
		return;

	DynamicLibrary* const library = DynamicLibraryManager::Instance().Load("coreclr");
	if(!B3D_ENSURE(library))
		return;

	// Function loading
	#define MONO_API_FUNCTION(ret, name, args) name = reinterpret_cast<FNPTR_##name>(library->GetSymbol(#name));
	#include "B3DMonoLoaderFunctions.h"
	#undef MONO_API_FUNCTION

	// The runtime can't be fully torn down: its threads outlive mono_jit_cleanup, and it leaves its signal and exception
	// handlers installed. All of them need the library's code to stay mapped.
	library->KeepLoadedUntilExit();

	mLibrary = library;
}

void MonoLoader::Unload()
{
	if(mLibrary != nullptr)
	{
		// Clear all function pointers
		#define MONO_API_FUNCTION(ret, name, args) name = nullptr;
		#include "B3DMonoLoaderFunctions.h"
		#undef MONO_API_FUNCTION

		DynamicLibraryManager::Instance().Unload(mLibrary);
		mLibrary = nullptr;
	}
}

