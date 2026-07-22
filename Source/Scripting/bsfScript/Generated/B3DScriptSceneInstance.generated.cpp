//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptSceneInstance.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "../../../Engine/Core/Scene/B3DSceneInstance.h"
#include "Reflection/B3DRTTIType.h"
#include "B3DScriptSceneInstance.generated.h"
#include "B3DScriptIEditorSceneInstance.generated.h"
#include "Wrappers/B3DScriptSceneObject.h"
#include "B3DScriptResourceWrapper.h"
#include "B3DScriptPhysicsScene.generated.h"
#include "../../../Engine/Core/Components/B3DCamera.h"
#include "B3DScriptCamera.generated.h"

namespace b3d
{
	ScriptSceneInstance::ScriptSceneInstance(const TShared<SceneInstance>& nativeObject)
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
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetMainCamera", (void*)&ScriptSceneInstance::InternalGetMainCamera);
#if B3D_WITH_EDITOR
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetEditorSceneInstance", (void*)&ScriptSceneInstance::InternalGetEditorSceneInstance);
#endif
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
		TGameObjectHandle<SceneObject> tmp__output;
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
		TShared<PhysicsScene> tmp__output;
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

	MonoObject* ScriptSceneInstance::InternalGetMainCamera(ScriptSceneInstance* self)
	{
		TGameObjectHandle<Camera> tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<SceneInstance*>(self->GetNativeObject())->GetMainCamera();

		MonoObject* __output;
		MonoObject* temp__output = nullptr;
		if(tmp__output)
			temp__output = ScriptComponent::GetOrCreateScriptObject(tmp__output);
		__output = temp__output;

		return __output;
	}

#if B3D_WITH_EDITOR
	MonoObject* ScriptSceneInstance::InternalGetEditorSceneInstance(ScriptSceneInstance* self)
	{
		TShared<IEditorSceneInstance> tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<SceneInstance*>(self->GetNativeObject())->GetEditorSceneInstance();

		MonoObject* __output;
		__output = ScriptIEditorSceneInstance::GetOrCreateScriptObject(tmp__output);

		return __output;
	}
#endif

	void ScriptSceneInstance::InternalClear(ScriptSceneInstance* self, bool forceAll)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<SceneInstance*>(self->GetNativeObject())->Clear(forceAll);
	}

	MonoObject* ScriptSceneInstance::InternalCreateSceneObject(ScriptSceneInstance* self, MonoString* name, uint32_t flags)
	{
		TGameObjectHandle<SceneObject> tmp__output;
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
		TShared<SceneInstance> nativeObject = SceneInstance::Create(tmpname);
		ScriptObjectWrapper::Create<ScriptSceneInstance>(nativeObject, scriptObject);
	}

	void ScriptSceneInstance::InternalCreate0(MonoObject* scriptObject, MonoString* name, MonoObject* root)
	{
		String tmpname;
		tmpname = MonoUtil::MonoToString(name);
		TGameObjectHandle<SceneObject> tmproot;
		ScriptSceneObject* scriptObjectWrapperroot;
		scriptObjectWrapperroot = ScriptSceneObject::GetScriptObjectWrapper(root);
		if(scriptObjectWrapperroot != nullptr)
			tmproot = B3DStaticGameObjectCast<SceneObject>(scriptObjectWrapperroot->GetBaseNativeObjectAsHandle());
		TShared<SceneInstance> nativeObject = SceneInstance::Create(tmpname, tmproot);
		ScriptObjectWrapper::Create<ScriptSceneInstance>(nativeObject, scriptObject);
	}
}
