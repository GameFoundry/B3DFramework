//************************************ B3D Framework - Copyright 2019 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsApplication.h"
#include "CoreObject/BsRenderThread.h"

/** Provides an entry point for executables. */
int bs_main(int argc, char* argv[]);

#ifndef B3D_CODEGEN // Needed to avoid including windows.h, as it includes macros that use commonly used names
#if B3D_PLATFORM == B3D_PLATFORM_ID_WIN32
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
#if B3D_SWAP_RENDER_AND_MAIN_THREAD
	Thread thread([argc = __argc, argv = __argv]()
				  { bs_main(argc, argv); });
	RenderThread::RunInternal();
	thread.join();
#else
	bs_main(__argc, __argv);
#endif
	return 0;
}
#endif
