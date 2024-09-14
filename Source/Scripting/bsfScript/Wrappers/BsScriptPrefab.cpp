//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Wrappers/BsScriptPrefab.h"
#include "BsScriptResourceManager.h"
#include "BsMonoClass.h"
#include "Scene/BsSceneManager.h"
#include "Wrappers/BsScriptSceneObject.h"

using namespace bs;
ScriptPrefab::ScriptPrefab(const HPrefab& nativeObject)
	: TScriptResourceWrapper(nativeObject)
{
	RegisterEvents();
}

void ScriptPrefab::SetupScriptBindings()
{
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_CreateInstance", (void*)&ScriptPrefab::InternalCreateInstance);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_Instantiate", (void*)&ScriptPrefab::InternalInstantiate);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_IsScene", (void*)&ScriptPrefab::InternalIsScene);
}

MonoObject* ScriptPrefab::CreateScriptObject(bool construct)
{
	return sInteropMetaData.ScriptClass->CreateInstance(construct);
}

Prefab* ScriptPrefab::GetNativeObject() const
{
	return static_cast<Prefab*>(TScriptResourceWrapper::GetNativeObject());
}

void ScriptPrefab::InternalCreateInstance(MonoObject* scriptObject, ScriptSceneObject* so, bool isScene)
{
	HPrefab prefab = Prefab::Create(so->GetNativeObjectAsHandle(), isScene);
	ScriptObjectWrapper::Create<ScriptPrefab>(prefab, scriptObject);
}

MonoObject* ScriptPrefab::InternalInstantiate(ScriptPrefab* self)
{
	if(!self->IsNativeObjectValid())
		return nullptr;

	HSceneObject instance = self->GetNativeObject()->Instantiate(GetSceneManager().GetMainScene());
	return ScriptSceneObject::GetOrCreateScriptObject(instance);
}

bool ScriptPrefab::InternalIsScene(ScriptPrefab* self)
{
	if(!self->IsNativeObjectValid())
		return false;

	return self->GetNativeObject()->IsScene();
}
