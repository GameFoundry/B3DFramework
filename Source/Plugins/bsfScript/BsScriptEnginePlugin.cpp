//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObjectManager.h"
#include "BsEngineScriptLibrary.h"
#include "Script/BsScriptManager.h"
#include "Wrappers/GUI/BsScriptGUI.h"
#include "BsPlayInEditorManager.h"
#include "Wrappers/BsScriptScene.h"

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
		ScriptScene::update();
		PlayInEditorManager::instance().update();
		ScriptObjectManager::instance().update();
		ScriptGUI::update();
	}
}
