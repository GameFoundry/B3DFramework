//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScriptManager.h"

namespace bs
{
	void ScriptManager::initialize()
	{
		if (mScriptLibrary != nullptr)
			mScriptLibrary->initialize();
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