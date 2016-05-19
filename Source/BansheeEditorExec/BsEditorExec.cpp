//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include <iostream>
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include "BsEditorApplication.h"
#include "BsPlatform.h"
#include "BsCrashHandler.h"

#if BS_PLATFORM == BS_PLATFORM_WIN32
#include <windows.h>

using namespace BansheeEngine;

#if BS_DEBUG_MODE
void InitializeDebugConsole()
{
	//Create a console for this application
	AllocConsole();

	// Redirect standard output to console
	freopen("CONIN$", "r", stdin);
	freopen("CONOUT$", "w", stdout);
	freopen("CONOUT$", "w", stderr);

	// Clear streams to ensure they aren't in an error state
	std::wcout.clear();
	std::cout.clear();
	std::wcerr.clear();
	std::cerr.clear();
	std::wcin.clear();
	std::cin.clear();
}

void ShutdownDebugConsole()
{
	//Write "Press any key to exit"
	HANDLE ConsoleOutput = GetStdHandle(STD_OUTPUT_HANDLE);
	DWORD CharsWritten;
	WriteConsole(ConsoleOutput, "\nPress any key to exit", 22, &CharsWritten, 0);
	//Disable line-based input mode so we can get a single character
	HANDLE ConsoleInput = GetStdHandle(STD_INPUT_HANDLE);
	SetConsoleMode(ConsoleInput, 0);
	//Read a single character
	TCHAR InputBuffer;
	DWORD CharsRead;
	ReadConsole(ConsoleInput, &InputBuffer, 1, &CharsRead, 0);
}
#endif // End BS_DEBUG_MODE

int CALLBACK WinMain(
	_In_  HINSTANCE hInstance,
	_In_  HINSTANCE hPrevInstance,
	_In_  LPSTR lpCmdLine,
	_In_  int nCmdShow
	)
{
#if BS_DEBUG_MODE
	InitializeDebugConsole();
#endif

	CrashHandler::startUp();

	__try
	{
		EditorApplication::startUp(EditorRenderAPI::DX11, AudioPlugin::OpenAudio);
		EditorApplication::instance().runMainLoop();
		EditorApplication::shutDown();
	}
	__except (gCrashHandler().reportCrash(GetExceptionInformation()))
	{
		PlatformUtility::terminate(true);
	}

#if BS_DEBUG_MODE
	ShutdownDebugConsole();
#endif

	CrashHandler::shutDown();

	return 0;
}
#endif // End BS_PLATFORM