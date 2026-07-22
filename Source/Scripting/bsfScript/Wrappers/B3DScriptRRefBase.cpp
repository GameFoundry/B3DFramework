//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptRRefBase.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "Resources/B3DResources.h"
#include "Serialization/B3DScriptAssemblyManager.h"
#include "B3DScriptResourceManager.h"

using namespace b3d;
ScriptRRefBase::ScriptRRefBase(const HResource& nativeObject)
	: TScriptValueTypeWrapper(nativeObject)
{ }

void ScriptRRefBase::SetupScriptBindings()
{
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_IsLoaded", (void*)&ScriptRRefBase::InternalIsLoaded);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetResource", (void*)&ScriptRRefBase::InternalGetResource);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetUUID", (void*)&ScriptRRefBase::InternalGetUuid);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_CastAs", (void*)&ScriptRRefBase::InternalCastAs);
}

MonoObject* ScriptRRefBase::CreateScriptObject(const HResource& handle, ::MonoClass* rawType)
{
	MonoClass* type = nullptr;
	if(rawType == nullptr)
		type = sInteropMetaData.ScriptClass;
	else
	{
		type = MonoManager::Instance().FindClass(rawType);
		if(type == nullptr)
			type = sInteropMetaData.ScriptClass;
		else
		{
			B3D_ASSERT(type->IsSubClassOf(sInteropMetaData.ScriptClass));
		}
	}

	MonoObject* scriptObject = type->CreateInstance();
	ScriptObjectWrapper::Create<ScriptRRefBase>(handle, scriptObject);

	return scriptObject;
}

::MonoClass* ScriptRRefBase::BindGenericParam(::MonoClass* param)
{
	MonoClass* rrefClass = ScriptAssemblyManager::Instance().GetBuiltinClasses().GenericRRefClass;

	::MonoClass* params[1] = { param };
	return MonoUtil::BindGenericParameters(rrefClass->GetInternalClass(), params, 1);
}

bool ScriptRRefBase::InternalIsLoaded(ScriptRRefBase* self)
{
	return self->GetNativeObject().IsLoaded(false);
}

MonoObject* ScriptRRefBase::InternalGetResource(ScriptRRefBase* self)
{
	const HResource resource = self->GetNativeObject();
	if(resource == nullptr)
		return nullptr;

	if(resource.IsLoaded(false))
		return ScriptResourceWrapper::GetOrCreateScriptObject(resource);

	ResourceLoadOptions loadOptions;
	loadOptions.AsynchronousLoad = false;
	loadOptions.LoadDependencies = true;

	const HResource loadedResource = GetResources().Load(resource.GetId(), loadOptions);
	return ScriptResourceWrapper::GetOrCreateScriptObject(loadedResource);
}

void ScriptRRefBase::InternalGetUuid(ScriptRRefBase* self, UUID* uuid)
{
	*uuid = self->GetNativeObject().GetId();
}

MonoObject* ScriptRRefBase::InternalCastAs(ScriptRRefBase* self, MonoReflectionType* type)
{
	::MonoClass* rawResType = MonoUtil::GetClass(type);

	MonoClass* resType = MonoManager::Instance().FindClass(rawResType);
	if(resType == nullptr)
		return nullptr; // Not a valid type

	::MonoClass* rrefType = nullptr;
	if(resType == ScriptResource::GetMetaData()->ScriptClass || resType->IsSubClassOf(ScriptResource::GetMetaData()->ScriptClass))
		rrefType = BindGenericParam(rawResType);

	return CreateScriptObject(self->GetNativeObject(), rrefType);
}

void ScriptRRefBase::NotifyScriptObjectDestroyed(bool isDestroyedDueToScriptReload)
{
	ScriptResourceManager::Instance().NotifyScriptRRefScriptObjectDestroyed(this);

	Super::NotifyScriptObjectDestroyed(isDestroyedDueToScriptReload);
}

