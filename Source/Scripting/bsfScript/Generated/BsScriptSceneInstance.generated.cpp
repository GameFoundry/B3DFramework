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

	void ScriptSceneInstance::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_getName", (void*)&ScriptSceneInstance::Internal_getName);
		metaData.scriptClass->addInternalCall("Internal_getRoot", (void*)&ScriptSceneInstance::Internal_getRoot);
		metaData.scriptClass->addInternalCall("Internal_isActive", (void*)&ScriptSceneInstance::Internal_isActive);
		metaData.scriptClass->addInternalCall("Internal_getPhysicsScene", (void*)&ScriptSceneInstance::Internal_getPhysicsScene);

	}

	MonoObject* ScriptSceneInstance::create(const SPtr<SceneInstance>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.scriptClass->createInstance("bool", ctorParams);
		new (bs_alloc<ScriptSceneInstance>()) ScriptSceneInstance(managedInstance, value);
		return managedInstance;
	}
	MonoString* ScriptSceneInstance::Internal_getName(ScriptSceneInstance* thisPtr)
	{
		String tmp__output;
		tmp__output = thisPtr->getInternal()->getName();

		MonoString* __output;
		__output = MonoUtil::stringToMono(tmp__output);

		return __output;
	}

	MonoObject* ScriptSceneInstance::Internal_getRoot(ScriptSceneInstance* thisPtr)
	{
		GameObjectHandle<SceneObject> tmp__output;
		tmp__output = thisPtr->getInternal()->getRoot();

		MonoObject* __output;
		ScriptSceneObject* script__output = nullptr;
		if(tmp__output)
		script__output = ScriptGameObjectManager::instance().getOrCreateScriptSceneObject(tmp__output);
		if(script__output != nullptr)
			__output = script__output->getManagedInstance();
		else
			__output = nullptr;

		return __output;
	}

	bool ScriptSceneInstance::Internal_isActive(ScriptSceneInstance* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->getInternal()->isActive();

		bool __output;
		__output = tmp__output;

		return __output;
	}

	MonoObject* ScriptSceneInstance::Internal_getPhysicsScene(ScriptSceneInstance* thisPtr)
	{
		SPtr<PhysicsScene> tmp__output;
		tmp__output = thisPtr->getInternal()->getPhysicsScene();

		MonoObject* __output;
		__output = ScriptPhysicsScene::create(tmp__output);

		return __output;
	}
}
