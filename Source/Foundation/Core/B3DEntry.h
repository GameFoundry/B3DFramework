//************************************ B3D Framework - Copyright 2019 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DApplication.h"
#include "CoreObject/B3DRenderThread.h"

/** Provides an entry point for executables. */
int B3DMain();

#ifndef B3D_CODEGEN // Needed to avoid including windows.h, as it includes macros that use commonly used names
#if B3D_PLATFORM_WIN32
#include <windows.h>

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
	return B3DMain();
}
#endif
