//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptResourceDragAndDropData.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"

namespace bs
{
	ScriptResourceDragAndDropData::ScriptResourceDragAndDropData(const SPtr<ResourceDragAndDropData>& nativeObject)
		:TScriptReflectableWrapper(nativeObject)
	{
		RegisterEvents();
	}

	void ScriptResourceDragAndDropData::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_ResourceDragAndDropData", (void*)&ScriptResourceDragAndDropData::InternalResourceDragAndDropData);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_ResourceDragAndDropData0", (void*)&ScriptResourceDragAndDropData::InternalResourceDragAndDropData0);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_ResourceDragAndDropData1", (void*)&ScriptResourceDragAndDropData::InternalResourceDragAndDropData1);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetResourcePaths", (void*)&ScriptResourceDragAndDropData::InternalGetResourcePaths);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetResourcePaths", (void*)&ScriptResourceDragAndDropData::InternalSetResourcePaths);

	}

	MonoObject* ScriptResourceDragAndDropData::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	void ScriptResourceDragAndDropData::InternalResourceDragAndDropData(MonoObject* scriptObject)
	{
		SPtr<ResourceDragAndDropData> nativeObject = B3DMakeShared<ResourceDragAndDropData>();
		ScriptObjectWrapper::Create<ScriptResourceDragAndDropData>(nativeObject, scriptObject);
	}

	void ScriptResourceDragAndDropData::InternalResourceDragAndDropData0(MonoObject* scriptObject, MonoString* resourcePath)
	{
		Path tmpresourcePath;
		tmpresourcePath = MonoUtil::MonoToString(resourcePath);
		SPtr<ResourceDragAndDropData> nativeObject = B3DMakeShared<ResourceDragAndDropData>(tmpresourcePath);
		ScriptObjectWrapper::Create<ScriptResourceDragAndDropData>(nativeObject, scriptObject);
	}

	void ScriptResourceDragAndDropData::InternalResourceDragAndDropData1(MonoObject* scriptObject, MonoArray* resourcePaths)
	{
		Vector<Path> nativeArrayresourcePaths;
		if(resourcePaths != nullptr)
		{
			ScriptArray scriptArrayresourcePaths(resourcePaths);
			nativeArrayresourcePaths.resize(scriptArrayresourcePaths.Size());
			for(int elementIndex = 0; elementIndex < (int)scriptArrayresourcePaths.Size(); elementIndex++)
			{
				nativeArrayresourcePaths[elementIndex] = scriptArrayresourcePaths.Get<Path>(elementIndex);
			}
		}
		SPtr<ResourceDragAndDropData> nativeObject = B3DMakeShared<ResourceDragAndDropData>(nativeArrayresourcePaths);
		ScriptObjectWrapper::Create<ScriptResourceDragAndDropData>(nativeObject, scriptObject);
	}

	MonoArray* ScriptResourceDragAndDropData::InternalGetResourcePaths(ScriptResourceDragAndDropData* self)
	{
		Vector<Path> nativeArray__output;
		if(!self->IsNativeObjectValid())
			return {};

		nativeArray__output = static_cast<ResourceDragAndDropData*>(self->GetNativeObject())->ResourcePaths;

		MonoArray* __output;
		int elementCount__output = (int)nativeArray__output.size();
		ScriptArray scriptArray__output = ScriptArray::Create<Path>(elementCount__output);
		for(int elementIndex = 0; elementIndex < elementCount__output; elementIndex++)
		{
			scriptArray__output.Set(elementIndex, nativeArray__output[elementIndex]);
		}
		__output = scriptArray__output.GetInternal();

		return __output;
	}

	void ScriptResourceDragAndDropData::InternalSetResourcePaths(ScriptResourceDragAndDropData* self, MonoArray* value)
	{
		if(!self->IsNativeObjectValid())
			return;

		Vector<Path> nativeArrayvalue;
		if(value != nullptr)
		{
			ScriptArray scriptArrayvalue(value);
			nativeArrayvalue.resize(scriptArrayvalue.Size());
			for(int elementIndex = 0; elementIndex < (int)scriptArrayvalue.Size(); elementIndex++)
			{
				nativeArrayvalue[elementIndex] = scriptArrayvalue.Get<Path>(elementIndex);
			}

		}
		static_cast<ResourceDragAndDropData*>(self->GetNativeObject())->ResourcePaths = nativeArrayvalue;
	}
}
