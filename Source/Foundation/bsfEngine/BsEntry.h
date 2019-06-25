//************************************ bs::framework - Copyright 2019 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsApplication.h"
#include "CoreThread/BsCoreThread.h"

/** Provides an entry point for executables. */
int bs_main(int argc, char* argv[]);

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
#if BS_CORE_THREAD_IS_MAIN
	Thread thread([argc = __argc, argv = __argv](){ bs_main(argc, argv); });
	CoreThread::_run();
	thread.join();
#else
	bs_main(__argc, __argv);
#endif
	return 0;
}