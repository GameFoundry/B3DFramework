//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Utility/B3DDynamicLibrary.h"
#include "Debug/B3DDebug.h"

#define WIN32_LEAN_AND_MEAN
#if !defined(NOMINMAX) && defined(_MSC_VER)
#	define NOMINMAX // required to stop windows.h messing up std::min
#endif
#include <windows.h>

using namespace b3d;

void DynamicLibrary::Load()
{
	if(mHandle)
		return;

	mHandle = (void*)LoadLibraryEx(mName.c_str(), NULL, LOAD_WITH_ALTERED_SEARCH_PATH);

	if(!mHandle)
	{
		B3D_LOG(Error, LogGeneric, "Could not load dynamic library {0}. System Error: {1}", mName, DynlibError());
	}
}

void DynamicLibrary::Unload()
{
	if(!mHandle)
		return;

	if(!FreeLibrary((HMODULE)mHandle))
	{
		B3D_LOG(Error, LogGeneric, "Could not unload dynamic library {0}. System Error: {1}", mName, DynlibError());
	}

	mHandle = nullptr;
}

void* DynamicLibrary::GetSymbol(const char* name) const
{
	if(!mHandle)
		return nullptr;

	return (void*)GetProcAddress((HMODULE)mHandle, name);
}

String DynamicLibrary::DynlibError()
{
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
}
