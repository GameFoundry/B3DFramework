//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsOISPrerequisites.h"
#include "BsInputHandlerOIS.h"
#include "BsRenderWindow.h"
#include "BsInput.h"

namespace bs
{
	/**	Returns a name of the plugin. */
	extern "C" BS_OIS_EXPORT const char* getPluginName()
	{
		static const char* pluginName = "OISInput";
		return pluginName;
	}

	/**	Entry point to the plugin. Called by the engine when the plugin is loaded. */
	extern "C" BS_OIS_EXPORT void* loadPlugin(void* primaryWindowPtr)
	{
		RenderWindow* primaryWindow = (RenderWindow*)primaryWindowPtr;

		if (primaryWindow == nullptr)
			assert(false && "Unable to get window handle. No active window exists!");

		UINT64 windowId = 0;
		primaryWindow->getCustomAttribute("WINDOW", &windowId);

		// TODO - Window handles in Windows are 64 bits when compiled as x64, but OIS only accepts a 32bit value. Is this okay?
		SPtr<RawInputHandler> inputHandler = bs_shared_ptr_new<InputHandlerOIS>((UINT32)windowId);

		gInput()._registerRawInputHandler(inputHandler);

		return nullptr;
	}
}