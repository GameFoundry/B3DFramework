//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptSceneInstance.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Scene/BsSceneManager.h"
#include "BsScriptSceneInstance.generated.h"
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
		metaData.ScriptClass->AddInternalCall("Internal_GetName", (void*)&ScriptSceneInstance::InternalGetName);
		metaData.ScriptClass->AddInternalCall("Internal_GetRoot", (void*)&ScriptSceneInstance::InternalGetRoot);
		metaData.ScriptClass->AddInternalCall("Internal_IsActive", (void*)&ScriptSceneInstance::InternalIsActive);
		metaData.ScriptClass->AddInternalCall("Internal_GetPhysicsScene", (void*)&ScriptSceneInstance::InternalGetPhysicsScene);
		metaData.ScriptClass->AddInternalCall("Internal_CreateSceneObject", (void*)&ScriptSceneInstance::InternalCreateSceneObject);
		metaData.ScriptClass->AddInternalCall("Internal_Create", (void*)&ScriptSceneInstance::InternalCreate);
		metaData.ScriptClass->AddInternalCall("Internal_Create0", (void*)&ScriptSceneInstance::InternalCreate0);

	}

	MonoObject* ScriptSceneInstance::Create(const SPtr<SceneInstance>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.ScriptClass->CreateInstance("bool", ctorParams);
		new (B3DAllocate<ScriptSceneInstance>()) ScriptSceneInstance(managedInstance, value);
		return managedInstance;
	}
	MonoString* ScriptSceneInstance::InternalGetName(ScriptSceneInstance* self)
	{
		String tmp__output;
		tmp__output = self->GetInternal()->GetName();

		MonoString* __output;
		__output = MonoUtil::StringToMono(tmp__output);

		return __output;
	}

	MonoObject* ScriptSceneInstance::InternalGetRoot(ScriptSceneInstance* self)
	{
		GameObjectHandle<SceneObject> tmp__output;
		tmp__output = self->GetInternal()->GetRoot();

		MonoObject* __output;
		MonoObject* temp__output = nullptr;
		if(tmp__output)
		temp__output = ScriptSceneObject::GetOrCreateScriptObject(tmp__output);
		__output = temp__output;

		return __output;
	}

	bool ScriptSceneInstance::InternalIsActive(ScriptSceneInstance* self)
	{
		bool tmp__output;
		tmp__output = self->GetInternal()->IsActive();

		bool __output;
		__output = tmp__output;

		return __output;
	}

	MonoObject* ScriptSceneInstance::InternalGetPhysicsScene(ScriptSceneInstance* self)
	{
		SPtr<PhysicsScene> tmp__output;
		tmp__output = self->GetInternal()->GetPhysicsScene();

		MonoObject* __output;
		__output = ScriptPhysicsScene::Create(tmp__output);

		return __output;
	}

	MonoObject* ScriptSceneInstance::InternalCreateSceneObject(ScriptSceneInstance* self, MonoString* name)
	{
		GameObjectHandle<SceneObject> tmp__output;
		String tmpname;
		tmpname = MonoUtil::MonoToString(name);
		tmp__output = self->GetInternal()->CreateSceneObject(tmpname);

		MonoObject* __output;
		MonoObject* temp__output = nullptr;
		if(tmp__output)
		temp__output = ScriptSceneObject::GetOrCreateScriptObject(tmp__output);
		__output = temp__output;

		return __output;
	}

	void ScriptSceneInstance::InternalCreate(MonoObject* managedInstance, MonoString* name)
	{
		String tmpname;
		tmpname = MonoUtil::MonoToString(name);
		SPtr<SceneInstance> nativeObject = SceneInstance::Create(tmpname);
		new (B3DAllocate<ScriptSceneInstance>())ScriptSceneInstance(managedInstance, nativeObject);
	}

	void ScriptSceneInstance::InternalCreate0(MonoObject* managedInstance, MonoString* name, MonoObject* root)
	{
		String tmpname;
		tmpname = MonoUtil::MonoToString(name);
		GameObjectHandle<SceneObject> tmproot;
		ScriptSceneObject* scriptObjectWrapperroot;
		scriptObjectWrapperroot = ScriptSceneObject::GetScriptObjectWrapper(root);
		if(scriptObjectWrapperroot != nullptr)
			tmproot = B3DStaticGameObjectCast<SceneObject>(scriptObjectWrapperroot->GetBaseNativeObjectAsHandle());
		SPtr<SceneInstance> nativeObject = SceneInstance::Create(tmpname, tmproot);
		new (B3DAllocate<ScriptSceneInstance>())ScriptSceneInstance(managedInstance, nativeObject);
	}
}
