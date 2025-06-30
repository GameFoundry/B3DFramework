//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptSceneInstance.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Scene/BsSceneManager.h"
#include "Reflection/BsRTTIType.h"
#include "BsScriptSceneInstance.generated.h"
#include "BsScriptIEditorSceneInstance.generated.h"
#include "Wrappers/BsScriptSceneObject.h"
#include "BsScriptResourceWrapper.h"
#include "BsScriptPhysicsScene.generated.h"
#include "../../../Foundation/bsfCore/Components/BsCCamera.h"
#include "BsScriptCCamera.generated.h"

namespace b3d
{
	ScriptSceneInstance::ScriptSceneInstance(const SPtr<SceneInstance>& nativeObject)
		:TScriptNonReflectableWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptSceneInstance::~ScriptSceneInstance()
	{
		UnregisterEvents();
	}

	void ScriptSceneInstance::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetName", (void*)&ScriptSceneInstance::InternalGetName);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetRoot", (void*)&ScriptSceneInstance::InternalGetRoot);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_IsActive", (void*)&ScriptSceneInstance::InternalIsActive);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetPhysicsScene", (void*)&ScriptSceneInstance::InternalGetPhysicsScene);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetAssociatedResourceId", (void*)&ScriptSceneInstance::InternalGetAssociatedResourceId);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetMainCameraComponent", (void*)&ScriptSceneInstance::InternalGetMainCameraComponent);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetEditorSceneInstance", (void*)&ScriptSceneInstance::InternalGetEditorSceneInstance);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Clear", (void*)&ScriptSceneInstance::InternalClear);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_CreateSceneObject", (void*)&ScriptSceneInstance::InternalCreateSceneObject);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_IsRunning", (void*)&ScriptSceneInstance::InternalIsRunning);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create", (void*)&ScriptSceneInstance::InternalCreate);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create0", (void*)&ScriptSceneInstance::InternalCreate0);

	}

	MonoObject* ScriptSceneInstance::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	MonoString* ScriptSceneInstance::InternalGetName(ScriptSceneInstance* self)
	{
		String tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<SceneInstance*>(self->GetNativeObject())->GetName();

		MonoString* __output;
		__output = MonoUtil::StringToMono(tmp__output);

		return __output;
	}

	MonoObject* ScriptSceneInstance::InternalGetRoot(ScriptSceneInstance* self)
	{
		GameObjectHandle<SceneObject> tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<SceneInstance*>(self->GetNativeObject())->GetRoot();

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
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<SceneInstance*>(self->GetNativeObject())->IsActive();

		bool __output;
		__output = tmp__output;

		return __output;
	}

	MonoObject* ScriptSceneInstance::InternalGetPhysicsScene(ScriptSceneInstance* self)
	{
		SPtr<PhysicsScene> tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<SceneInstance*>(self->GetNativeObject())->GetPhysicsScene();

		MonoObject* __output;
		__output = ScriptPhysicsScene::GetOrCreateScriptObject(tmp__output);

		return __output;
	}

	void ScriptSceneInstance::InternalGetAssociatedResourceId(ScriptSceneInstance* self, UUID* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		UUID tmp__output;
		tmp__output = static_cast<SceneInstance*>(self->GetNativeObject())->GetAssociatedResourceId();

		*__output = tmp__output;
	}

	MonoObject* ScriptSceneInstance::InternalGetMainCameraComponent(ScriptSceneInstance* self)
	{
		GameObjectHandle<CCamera> tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<SceneInstance*>(self->GetNativeObject())->GetMainCameraComponent();

		MonoObject* __output;
		MonoObject* temp__output = nullptr;
		if(tmp__output)
			temp__output = ScriptComponent::GetOrCreateScriptObject(tmp__output);
		__output = temp__output;

		return __output;
	}

	MonoObject* ScriptSceneInstance::InternalGetEditorSceneInstance(ScriptSceneInstance* self)
	{
		SPtr<IEditorSceneInstance> tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<SceneInstance*>(self->GetNativeObject())->GetEditorSceneInstance();

		MonoObject* __output;
		__output = ScriptIEditorSceneInstance::GetOrCreateScriptObject(tmp__output);

		return __output;
	}

	void ScriptSceneInstance::InternalClear(ScriptSceneInstance* self, bool forceAll)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<SceneInstance*>(self->GetNativeObject())->Clear(forceAll);
	}

	MonoObject* ScriptSceneInstance::InternalCreateSceneObject(ScriptSceneInstance* self, MonoString* name, uint32_t flags)
	{
		GameObjectHandle<SceneObject> tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		String tmpname;
		tmpname = MonoUtil::MonoToString(name);
		tmp__output = static_cast<SceneInstance*>(self->GetNativeObject())->CreateSceneObject(tmpname, flags);

		MonoObject* __output;
		MonoObject* temp__output = nullptr;
		if(tmp__output)
		temp__output = ScriptSceneObject::GetOrCreateScriptObject(tmp__output);
		__output = temp__output;

		return __output;
	}

	bool ScriptSceneInstance::InternalIsRunning(ScriptSceneInstance* self)
	{
		bool tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<SceneInstance*>(self->GetNativeObject())->IsRunning();

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptSceneInstance::InternalCreate(MonoObject* scriptObject, MonoString* name)
	{
		String tmpname;
		tmpname = MonoUtil::MonoToString(name);
		SPtr<SceneInstance> nativeObject = SceneInstance::Create(tmpname);
		ScriptObjectWrapper::Create<ScriptSceneInstance>(nativeObject, scriptObject);
	}

	void ScriptSceneInstance::InternalCreate0(MonoObject* scriptObject, MonoString* name, MonoObject* root)
	{
		String tmpname;
		tmpname = MonoUtil::MonoToString(name);
		GameObjectHandle<SceneObject> tmproot;
		ScriptSceneObject* scriptObjectWrapperroot;
		scriptObjectWrapperroot = ScriptSceneObject::GetScriptObjectWrapper(root);
		if(scriptObjectWrapperroot != nullptr)
			tmproot = B3DStaticGameObjectCast<SceneObject>(scriptObjectWrapperroot->GetBaseNativeObjectAsHandle());
		SPtr<SceneInstance> nativeObject = SceneInstance::Create(tmpname, tmproot);
		ScriptObjectWrapper::Create<ScriptSceneInstance>(nativeObject, scriptObject);
	}
}
