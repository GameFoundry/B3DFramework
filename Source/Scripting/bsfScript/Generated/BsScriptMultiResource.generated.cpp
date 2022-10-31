//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptMultiResource.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Importer/BsImporter.h"
#include "BsScriptSubResource.generated.h"

using namespace bs;
#if !BS_IS_BANSHEE3D
ScriptMultiResource::ScriptMultiResource(MonoObject* managedInstance, const SPtr<MultiResource>& value)
	: ScriptObject(managedInstance), mInternal(value)
{
}

void ScriptMultiResource::InitRuntimeData()
{
	metaData.ScriptClass->AddInternalCall("Internal_MultiResource", (void*)&ScriptMultiResource::InternalMultiResource);
	metaData.ScriptClass->AddInternalCall("Internal_MultiResource0", (void*)&ScriptMultiResource::InternalMultiResource0);
	metaData.ScriptClass->AddInternalCall("Internal_GetEntries", (void*)&ScriptMultiResource::InternalGetEntries);
	metaData.ScriptClass->AddInternalCall("Internal_SetEntries", (void*)&ScriptMultiResource::InternalSetEntries);
}

MonoObject* ScriptMultiResource::Create(const SPtr<MultiResource>& value)
{
	if(value == nullptr) return nullptr;

	bool dummy = false;
	void* ctorParams[1] = { &dummy };

	MonoObject* managedInstance = metaData.ScriptClass->CreateInstance("bool", ctorParams);
	new(B3DAllocate<ScriptMultiResource>()) ScriptMultiResource(managedInstance, value);
	return managedInstance;
}

void ScriptMultiResource::InternalMultiResource(MonoObject* managedInstance)
{
	SPtr<MultiResource> instance = B3DMakeShared<MultiResource>();
	new(B3DAllocate<ScriptMultiResource>()) ScriptMultiResource(managedInstance, instance);
}

void ScriptMultiResource::InternalMultiResource0(MonoObject* managedInstance, MonoArray* entries)
{
	Vector<SubResource> vecentries;
	if(entries != nullptr)
	{
		ScriptArray arrayentries(entries);
		vecentries.resize(arrayentries.Size());
		for(int i = 0; i < (int)arrayentries.Size(); i++)
		{
			vecentries[i] = ScriptSubResource::FromInterop(arrayentries.Get<__SubResourceInterop>(i));
		}
	}
	SPtr<MultiResource> instance = B3DMakeShared<MultiResource>(vecentries);
	new(B3DAllocate<ScriptMultiResource>()) ScriptMultiResource(managedInstance, instance);
}

MonoArray* ScriptMultiResource::InternalGetEntries(ScriptMultiResource* thisPtr)
{
	Vector<SubResource> vec__output;
	vec__output = thisPtr->GetInternal()->Entries;

	MonoArray* __output;
	int arraySize__output = (int)vec__output.size();
	ScriptArray array__output = ScriptArray::Create<ScriptSubResource>(arraySize__output);
	for(int i = 0; i < arraySize__output; i++)
	{
		array__output.Set(i, ScriptSubResource::ToInterop(vec__output[i]));
	}
	__output = array__output.GetInternal();

	return __output;
}

void ScriptMultiResource::InternalSetEntries(ScriptMultiResource* thisPtr, MonoArray* value)
{
	Vector<SubResource> vecvalue;
	if(value != nullptr)
	{
		ScriptArray arrayvalue(value);
		vecvalue.resize(arrayvalue.Size());
		for(int i = 0; i < (int)arrayvalue.Size(); i++)
		{
			vecvalue[i] = ScriptSubResource::FromInterop(arrayvalue.Get<__SubResourceInterop>(i));
		}
	}
	thisPtr->GetInternal()->Entries = vecvalue;
}
#endif
