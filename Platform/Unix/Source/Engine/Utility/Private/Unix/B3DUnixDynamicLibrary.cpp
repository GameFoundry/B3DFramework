//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Utility/B3DDynamicLibrary.h"

#include "Debug/B3DDebug.h"

#include <dlfcn.h>

using namespace b3d;

void DynamicLibrary::Load()
{
	if(mHandle)
		return;

	// RTLD_GLOBAL exports the plugin's symbols process-wide, which cross-plugin code may rely on (shared RTTI,
	// exceptions across module boundaries). Consider RTLD_LOCAL if plugin symbol collisions ever surface.
	mHandle = dlopen(mName.c_str(), RTLD_LAZY | RTLD_GLOBAL);

	if(!mHandle)
	{
		B3D_LOG(Error, LogGeneric, "Could not load dynamic library {0}. System Error: {1}", mName, DynlibError());
	}
}

void DynamicLibrary::Unload()
{
	if(!mHandle)
		return;

	if(dlclose(mHandle))
	{
		B3D_LOG(Error, LogGeneric, "Could not unload dynamic library {0}. System Error: {1}", mName, DynlibError());
	}

	mHandle = nullptr;
}

void* DynamicLibrary::GetSymbol(const char* name) const
{
	if(!mHandle)
		return nullptr;

	return dlsym(mHandle, name);
}

String DynamicLibrary::DynlibError()
{
	return String(dlerror());
}
