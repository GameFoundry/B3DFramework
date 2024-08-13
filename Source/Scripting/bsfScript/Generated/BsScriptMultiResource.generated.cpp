//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptMultiResource.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Importer/BsImporter.h"
#include "BsScriptSubResource.generated.h"

namespace bs
{
#if !B3D_IS_ENGINE
	ScriptMultiResource::ScriptMultiResource(MonoObject* managedInstance, const SPtr<MultiResource>& value)
		:ScriptObject(managedInstance), mInternal(value)
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
		new (B3DAllocate<ScriptMultiResource>()) ScriptMultiResource(managedInstance, value);
		return managedInstance;
	}
	void ScriptMultiResource::InternalMultiResource(MonoObject* managedInstance)
	{
		SPtr<MultiResource> nativeObject = B3DMakeShared<MultiResource>();
		new (B3DAllocate<ScriptMultiResource>())ScriptMultiResource(managedInstance, nativeObject);
	}

	void ScriptMultiResource::InternalMultiResource0(MonoObject* managedInstance, MonoArray* entries)
	{
		Vector<SubResource> nativeArrayentries;
		if(entries != nullptr)
		{
			ScriptArray scriptArrayentries(entries);
			nativeArrayentries.resize(scriptArrayentries.Size());
			for(int elementIndex = 0; elementIndex < (int)scriptArrayentries.Size(); elementIndex++)
			{
				nativeArrayentries[elementIndex] = ScriptSubResource::FromInterop(scriptArrayentries.Get<__SubResourceInterop>(elementIndex));
			}
		}
		SPtr<MultiResource> nativeObject = B3DMakeShared<MultiResource>(nativeArrayentries);
		new (B3DAllocate<ScriptMultiResource>())ScriptMultiResource(managedInstance, nativeObject);
	}

	MonoArray* ScriptMultiResource::InternalGetEntries(ScriptMultiResource* self)
	{
		Vector<SubResource> nativeArray__output;
		nativeArray__output = self->GetInternal()->Entries;

		MonoArray* __output;
		int elementCount__output = (int)nativeArray__output.size();
		ScriptArray scriptArray__output = ScriptArray::Create<ScriptSubResource>(elementCount__output);
		for(int elementIndex = 0; elementIndex < elementCount__output; elementIndex++)
		{
			scriptArray__output.Set(elementIndex, ScriptSubResource::ToInterop(nativeArray__output[elementIndex]));
		}
		__output = scriptArray__output.GetInternal();

		return __output;
	}

	void ScriptMultiResource::InternalSetEntries(ScriptMultiResource* self, MonoArray* value)
	{
		Vector<SubResource> nativeArrayvalue;
		if(value != nullptr)
		{
			ScriptArray scriptArrayvalue(value);
			nativeArrayvalue.resize(scriptArrayvalue.Size());
			for(int elementIndex = 0; elementIndex < (int)scriptArrayvalue.Size(); elementIndex++)
			{
				nativeArrayvalue[elementIndex] = ScriptSubResource::FromInterop(scriptArrayvalue.Get<__SubResourceInterop>(elementIndex));
			}

		}
		self->GetInternal()->Entries = nativeArrayvalue;
	}
#endif
}
