//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Script/BsScriptManager.h"

namespace bs
{
	SPtr<ScriptLibrary> ScriptManager::sScriptLibrary;

	ScriptManager::ScriptManager()
	{
		if (sScriptLibrary)
			sScriptLibrary->initialize();
	}

	ScriptManager::~ScriptManager()
	{
		if (sScriptLibrary)
			sScriptLibrary->destroy();
	}

	void ScriptManager::update()
	{
		if(sScriptLibrary)
			sScriptLibrary->update();
	}

	void ScriptManager::reload()
	{
		if (sScriptLibrary)
			sScriptLibrary->reload();
	}
}