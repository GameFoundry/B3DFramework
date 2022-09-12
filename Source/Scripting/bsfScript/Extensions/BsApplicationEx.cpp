//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Extensions/BsApplicationEx.h"

using namespace std::placeholders;

namespace bs
{
	void ApplicationEx::StartUp(const START_UP_DESC& desc)
	{
		Application::startUp(desc);
	}

	void ApplicationEx::StartUp(VideoMode videoMode, const String& title, bool fullscreen)
	{
		Application::startUp(videoMode, title, fullscreen);
	}

	void ApplicationEx::RunMainLoop()
	{
		Application::instance().RunMainLoop();
	}

	void ApplicationEx::ShutDown()
	{
		Application::shutDown();
	}
}
