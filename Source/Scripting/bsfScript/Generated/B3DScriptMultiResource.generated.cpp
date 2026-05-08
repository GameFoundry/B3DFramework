//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptMultiResource.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "../../../Engine/Core/Importer/B3DImporter.h"
#include "B3DScriptSubResource.generated.h"

namespace b3d
{
#if !B3D_IS_ENGINE
	ScriptMultiResource::ScriptMultiResource(const TShared<MultiResource>& nativeObject)
		:TScriptNonReflectableWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptMultiResource::~ScriptMultiResource()
	{
		UnregisterEvents();
	}

	void ScriptMultiResource::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_MultiResource", (void*)&ScriptMultiResource::InternalMultiResource);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_MultiResource0", (void*)&ScriptMultiResource::InternalMultiResource0);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetEntries", (void*)&ScriptMultiResource::InternalGetEntries);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetEntries", (void*)&ScriptMultiResource::InternalSetEntries);

	}

	MonoObject* ScriptMultiResource::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	void ScriptMultiResource::InternalMultiResource(MonoObject* scriptObject)
	{
		TShared<MultiResource> nativeObject = B3DMakeShared<MultiResource>();
		ScriptObjectWrapper::Create<ScriptMultiResource>(nativeObject, scriptObject);
	}

	void ScriptMultiResource::InternalMultiResource0(MonoObject* scriptObject, MonoArray* entries)
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
		TShared<MultiResource> nativeObject = B3DMakeShared<MultiResource>(nativeArrayentries);
		ScriptObjectWrapper::Create<ScriptMultiResource>(nativeObject, scriptObject);
	}

	MonoArray* ScriptMultiResource::InternalGetEntries(ScriptMultiResource* self)
	{
		Vector<SubResource> nativeArray__output;
		if(!self->IsNativeObjectValid())
			return {};

		nativeArray__output = static_cast<MultiResource*>(self->GetNativeObject())->Entries;

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
		if(!self->IsNativeObjectValid())
			return;

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
		static_cast<MultiResource*>(self->GetNativeObject())->Entries = nativeArrayvalue;
	}
#endif
}
