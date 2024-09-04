//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptRRefBase.h"
#include "BsScriptMeta.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "Resources/BsResources.h"
#include "Serialization/BsScriptAssemblyManager.h"

using namespace bs;
ScriptRRefBase::ScriptRRefBase(const TResourceHandle<Resource>& nativeObject, MonoObject* scriptObject)
	: TScriptResourceWrapper(nativeObject, scriptObject)
{}

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
	B3DNew<ScriptRRefBase>(handle, scriptObject);

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
	if(!self->IsNativeObjectValid())
		return false;

	return self->GetNativeObjectAsHandle().IsLoaded(false);
}

MonoObject* ScriptRRefBase::InternalGetResource(ScriptRRefBase* self)
{
	if(!self->IsNativeObjectValid())
		return nullptr;

	const HResource resource = self->GetNativeObjectAsHandle();
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
	if(!self->IsNativeObjectValid())
		*uuid = UUID::kEmpty;

	*uuid = self->GetNativeObjectAsHandle().GetId();
}

MonoObject* ScriptRRefBase::InternalCastAs(ScriptRRefBase* self, MonoReflectionType* type)
{
	if(!self->IsNativeObjectValid())
		return nullptr;

	::MonoClass* rawResType = MonoUtil::GetClass(type);

	MonoClass* resType = MonoManager::Instance().FindClass(rawResType);
	if(resType == nullptr)
		return nullptr; // Not a valid type

	::MonoClass* rrefType = nullptr;
	if(resType == ScriptResource::GetMetaData()->ScriptClass || resType->IsSubClassOf(ScriptResource::GetMetaData()->ScriptClass))
		rrefType = BindGenericParam(rawResType);

	return CreateScriptObject(self->GetNativeObjectAsHandle(), rrefType);
}
