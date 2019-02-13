//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Script/BsScriptManager.h"

namespace bs
{
	void ScriptManager::initialize()
	{
		if (mScriptLibrary != nullptr)
			mScriptLibrary->initialize();
	}

	void ScriptManager::update()
	{
		if(mScriptLibrary)
			mScriptLibrary->update();
	}

	void ScriptManager::reload()
	{
		if (mScriptLibrary != nullptr)
			mScriptLibrary->reload();
	}

	void ScriptManager::onShutDown()
	{
		if (mScriptLibrary != nullptr)
			mScriptLibrary->destroy();
	}

	void ScriptManager::_setScriptLibrary(const SPtr<ScriptLibrary>& library)
	{
		mScriptLibrary = library;
	}
}