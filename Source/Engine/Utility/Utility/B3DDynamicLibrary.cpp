//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Utility/B3DDynamicLibrary.h"

#include "Debug/B3DDebug.h"

#if B3D_PLATFORM_WIN32
#	define WIN32_LEAN_AND_MEAN
#	if !defined(NOMINMAX) && defined(_MSC_VER)
#		define NOMINMAX // required to stop windows.h messing up std::min
#	endif
#	include <windows.h>
#endif

#if B3D_PLATFORM_MACOS
#	include <dlfcn.h>
#endif

using namespace b3d;

DynamicLibrary::DynamicLibrary(String name)
	: mName(std::move(name))
{
	Load();
}

DynamicLibrary::~DynamicLibrary()
{
	Unload();
}

void DynamicLibrary::Load()
{
	if(mHandle)
		return;

	mHandle = (DYNLIB_HANDLE)DYNLIB_LOAD(mName.c_str());

	if(!mHandle)
	{
		B3D_LOG(Error, LogGeneric, "Could not load dynamic library {0}. System Error: {1}", mName, DynlibError());
	}
}

void DynamicLibrary::Unload()
{
	if(!mHandle)
		return;

	if(DYNLIB_UNLOAD(mHandle))
	{
		B3D_LOG(Error, LogGeneric, "Could not unload dynamic library {0}. System Error: {1}", mName, DynlibError());
	}

	mHandle = nullptr;
}

void* DynamicLibrary::GetSymbol(const char* name) const
{
	if(!mHandle)
		return nullptr;

	return (void*)DYNLIB_GETSYM(mHandle, name);
}

String DynamicLibrary::DynlibError()
{
#if B3D_PLATFORM_WIN32
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		GetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0,
		NULL);

	String ret((char*)lpMsgBuf);

	// Free the buffer.
	LocalFree(lpMsgBuf);
	return ret;
#elif B3D_PLATFORM_LINUX || B3D_PLATFORM_MACOS
	return String(dlerror());
#else
	return String();
#endif
}
