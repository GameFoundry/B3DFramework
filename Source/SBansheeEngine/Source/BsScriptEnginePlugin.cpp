//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObjectManager.h"
#include "BsEngineScriptLibrary.h"
#include "BsScriptManager.h"
#include "BsScriptGUI.h"
#include "BsPlayInEditorManager.h"

namespace bs
{
	extern "C" BS_SCR_BE_EXPORT const char* getPluginName()
	{
		static const char* pluginName = "SBansheeEngine";
		return pluginName;
	}

	extern "C" BS_SCR_BE_EXPORT void* loadPlugin()
	{
		SPtr<EngineScriptLibrary> library = bs_shared_ptr_new<EngineScriptLibrary>();
		ScriptManager::instance()._setScriptLibrary(library);

		return nullptr;
	}

	extern "C" BS_SCR_BE_EXPORT void updatePlugin()
	{
		PlayInEditorManager::instance().update();
		ScriptObjectManager::instance().update();
		ScriptGUI::update();
	}
}