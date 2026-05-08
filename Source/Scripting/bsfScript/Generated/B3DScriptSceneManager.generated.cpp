//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptSceneManager.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "../../../Engine/Core/Scene/B3DSceneManager.h"
#include "B3DScriptSceneInstance.generated.h"

namespace b3d
{
	ScriptSceneManager::ScriptSceneManager()
		:TScriptTypeDefinition()
	{
	}

	void ScriptSceneManager::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetMainScene", (void*)&ScriptSceneManager::InternalSetMainScene);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetMainScene", (void*)&ScriptSceneManager::InternalGetMainScene);

	}

	void ScriptSceneManager::InternalSetMainScene(MonoObject* scene)
	{
		TShared<SceneInstance> tmpscene;
		ScriptSceneInstance* scriptObjectWrapperscene;
		scriptObjectWrapperscene = ScriptSceneInstance::GetScriptObjectWrapper(scene);
		if(scriptObjectWrapperscene != nullptr)
			tmpscene = std::static_pointer_cast<SceneInstance>(scriptObjectWrapperscene->GetBaseNativeObjectAsShared());
		SceneManager::Instance().SetMainScene(tmpscene);
	}

	MonoObject* ScriptSceneManager::InternalGetMainScene()
	{
		TShared<SceneInstance> tmp__output;
		tmp__output = SceneManager::Instance().GetMainScene();

		MonoObject* __output;
		__output = ScriptSceneInstance::GetOrCreateScriptObject(tmp__output);

		return __output;
	}
}
