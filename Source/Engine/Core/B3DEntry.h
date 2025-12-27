//************************************ B3D Framework - Copyright 2019 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DApplication.h"
#include "CoreObject/B3DRenderThread.h"
#include "Utility/B3DCommandLine.h"

/** Provides an entry point for executables. */
int B3DMain();

#ifndef B3D_CODEGEN // Needed to avoid including windows.h, as it includes macros that use commonly used names
#if B3D_PLATFORM_WIN32

#define WINAPI __stdcall

typedef void* HANDLE;
typedef HANDLE HINSTANCE;
typedef char* LPSTR;

extern "C"
{
	LPSTR WINAPI GetCommandLineA(void);
}

int WINAPI WinMain(
	_In_ HINSTANCE hInstance,
	_In_ HINSTANCE hPrevInstance,
	_In_ LPSTR lpCmdLine,
	_In_ int nCmdShow)
#else
int main(int __argc, char* __argv[])
#endif
{
	using namespace b3d;

	CrashHandler::StartUp();

#if B3D_PLATFORM_WIN32
	CommandLine::Initialize(::GetCommandLineA());
#else
	CommandLine::Initialize(__argc, __argv);
#endif

	const int returnValue = B3DMain();
	CrashHandler::ShutDown();

	return returnValue;
}
#endif
