//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptScene.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "../../../Foundation/Core/Scene/B3DScene.h"
#include "B3DScriptResourceManager.h"
#include "Wrappers/B3DScriptRRefBase.h"
#include "Wrappers/B3DScriptSceneObject.h"
#include "B3DScriptSceneInstance.generated.h"
#include "../../../Foundation/Core/Scene/B3DScene.h"

namespace b3d
{
	ScriptScene::ScriptScene(const TResourceHandle<Scene>& nativeObject)
		:TScriptResourceWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptScene::~ScriptScene()
	{
		UnregisterEvents();
	}

	void ScriptScene::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetRef", (void*)&ScriptScene::InternalGetRef);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Instantiate", (void*)&ScriptScene::InternalInstantiate);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create", (void*)&ScriptScene::InternalCreate);

	}

	MonoObject* ScriptScene::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	MonoObject* ScriptScene::InternalGetRef(ScriptScene* self)
	{
		return self->GetOrCreateResourceReference();
	}

	MonoObject* ScriptScene::InternalInstantiate(ScriptScene* self)
	{
		SPtr<SceneInstance> tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<Scene*>(self->GetNativeObject())->Instantiate();

		MonoObject* __output;
		__output = ScriptSceneInstance::GetOrCreateScriptObject(tmp__output);

		return __output;
	}

	void ScriptScene::InternalCreate(MonoObject* scriptObject, MonoObject* sceneObject)
	{
		TGameObjectHandle<SceneObject> tmpsceneObject;
		ScriptSceneObject* scriptObjectWrappersceneObject;
		scriptObjectWrappersceneObject = ScriptSceneObject::GetScriptObjectWrapper(sceneObject);
		if(scriptObjectWrappersceneObject != nullptr)
			tmpsceneObject = B3DStaticGameObjectCast<SceneObject>(scriptObjectWrappersceneObject->GetBaseNativeObjectAsHandle());
		TResourceHandle<Scene> nativeObject = Scene::Create(tmpsceneObject);
		ScriptObjectWrapper::Create<ScriptScene>(nativeObject, scriptObject);
	}
}
