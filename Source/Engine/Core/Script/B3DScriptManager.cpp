//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Script/B3DScriptManager.h"

using namespace b3d;

SPtr<ScriptLibrary> ScriptManager::sScriptLibrary;

ScriptManager::ScriptManager()
{
	if(sScriptLibrary)
		sScriptLibrary->Initialize();
}

ScriptManager::~ScriptManager()
{
	if(sScriptLibrary)
		sScriptLibrary->Destroy();
}

void ScriptManager::Update()
{
	if(sScriptLibrary)
		sScriptLibrary->Update();
}

void ScriptManager::Reload()
{
	if(sScriptLibrary)
		sScriptLibrary->Reload();
}
