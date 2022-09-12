//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptSceneInstance.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Scene/BsSceneManager.h"
#include "BsScriptGameObjectManager.h"
#include "Wrappers/BsScriptSceneObject.h"
#include "BsScriptPhysicsScene.generated.h"

namespace bs
{
	ScriptSceneInstance::ScriptSceneInstance(MonoObject* managedInstance, const SPtr<SceneInstance>& value)
		:ScriptObject(managedInstance), mInternal(value)
	{
	}

	void ScriptSceneInstance::InitRuntimeData()
	{
		metaData.scriptClass->AddInternalCall("Internal_getName", (void*)&ScriptSceneInstance::Internal_getName);
		metaData.scriptClass->AddInternalCall("Internal_getRoot", (void*)&ScriptSceneInstance::Internal_getRoot);
		metaData.scriptClass->AddInternalCall("Internal_isActive", (void*)&ScriptSceneInstance::Internal_isActive);
		metaData.scriptClass->AddInternalCall("Internal_getPhysicsScene", (void*)&ScriptSceneInstance::Internal_getPhysicsScene);

	}

	MonoObject* ScriptSceneInstance::create(const SPtr<SceneInstance>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.scriptClass->CreateInstance("bool", ctorParams);
		new (bs_alloc<ScriptSceneInstance>()) ScriptSceneInstance(managedInstance, value);
		return managedInstance;
	}
	MonoString* ScriptSceneInstance::Internal_getName(ScriptSceneInstance* thisPtr)
	{
		String tmp__output;
		tmp__output = thisPtr->GetInternal()->getName();

		MonoString* __output;
		__output = MonoUtil::stringToMono(tmp__output);

		return __output;
	}

	MonoObject* ScriptSceneInstance::Internal_getRoot(ScriptSceneInstance* thisPtr)
	{
		GameObjectHandle<SceneObject> tmp__output;
		tmp__output = thisPtr->GetInternal()->getRoot();

		MonoObject* __output;
		ScriptSceneObject* script__output = nullptr;
		if(tmp__output)
		script__output = ScriptGameObjectManager::instance().GetOrCreateScriptSceneObject(tmp__output);
		if(script__output != nullptr)
			__output = script__output->GetManagedInstance();
		else
			__output = nullptr;

		return __output;
	}

	bool ScriptSceneInstance::Internal_isActive(ScriptSceneInstance* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetInternal()->isActive();

		bool __output;
		__output = tmp__output;

		return __output;
	}

	MonoObject* ScriptSceneInstance::Internal_getPhysicsScene(ScriptSceneInstance* thisPtr)
	{
		SPtr<PhysicsScene> tmp__output;
		tmp__output = thisPtr->GetInternal()->getPhysicsScene();

		MonoObject* __output;
		__output = ScriptPhysicsScene::create(tmp__output);

		return __output;
	}
}
