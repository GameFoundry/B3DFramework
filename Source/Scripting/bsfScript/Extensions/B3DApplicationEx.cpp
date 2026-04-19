//********************************* B3D Framework - Copyright 2026 Marko Pintera *****************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Extensions/B3DApplicationEx.h"

using namespace b3d;
void ApplicationEx::StartUp(const ApplicationCreateInformation& desc)
{
	Application::StartUp(desc);
}

void ApplicationEx::StartUp(VideoMode videoMode, const String& title, bool fullscreen)
{
	Application::StartUp(videoMode, title, fullscreen);
}

void ApplicationEx::RunMainLoop()
{
	Application::Instance().RunMainLoop();
}

void ApplicationEx::ShutDown()
{
	Application::ShutDown();
}
