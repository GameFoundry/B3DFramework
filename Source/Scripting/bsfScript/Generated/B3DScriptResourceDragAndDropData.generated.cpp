//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptResourceDragAndDropData.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"

namespace b3d
{
	ScriptResourceDragAndDropData::ScriptResourceDragAndDropData(const TShared<ResourceDragAndDropData>& nativeObject)
		:TScriptReflectableWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptResourceDragAndDropData::~ScriptResourceDragAndDropData()
	{
		UnregisterEvents();
	}

	void ScriptResourceDragAndDropData::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_ResourceDragAndDropData", (void*)&ScriptResourceDragAndDropData::InternalResourceDragAndDropData);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_ResourceDragAndDropData0", (void*)&ScriptResourceDragAndDropData::InternalResourceDragAndDropData0);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_ResourceDragAndDropData1", (void*)&ScriptResourceDragAndDropData::InternalResourceDragAndDropData1);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetRelativeResourcePaths", (void*)&ScriptResourceDragAndDropData::InternalGetRelativeResourcePaths);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetRelativeResourcePaths", (void*)&ScriptResourceDragAndDropData::InternalSetRelativeResourcePaths);

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
		TShared<ResourceDragAndDropData> nativeObject = B3DMakeShared<ResourceDragAndDropData>();
		ScriptObjectWrapper::Create<ScriptResourceDragAndDropData>(nativeObject, scriptObject);
	}

	void ScriptResourceDragAndDropData::InternalResourceDragAndDropData0(MonoObject* scriptObject, MonoString* relativeResourcePath)
	{
		Path tmprelativeResourcePath;
		tmprelativeResourcePath = MonoUtil::MonoToString(relativeResourcePath);
		TShared<ResourceDragAndDropData> nativeObject = B3DMakeShared<ResourceDragAndDropData>(tmprelativeResourcePath);
		ScriptObjectWrapper::Create<ScriptResourceDragAndDropData>(nativeObject, scriptObject);
	}

	void ScriptResourceDragAndDropData::InternalResourceDragAndDropData1(MonoObject* scriptObject, MonoArray* relativeResourcePaths)
	{
		Vector<Path> nativeArrayrelativeResourcePaths;
		if(relativeResourcePaths != nullptr)
		{
			ScriptArray scriptArrayrelativeResourcePaths(relativeResourcePaths);
			nativeArrayrelativeResourcePaths.resize(scriptArrayrelativeResourcePaths.Size());
			for(int elementIndex = 0; elementIndex < (int)scriptArrayrelativeResourcePaths.Size(); elementIndex++)
			{
				nativeArrayrelativeResourcePaths[elementIndex] = scriptArrayrelativeResourcePaths.Get<Path>(elementIndex);
			}
		}
		TShared<ResourceDragAndDropData> nativeObject = B3DMakeShared<ResourceDragAndDropData>(nativeArrayrelativeResourcePaths);
		ScriptObjectWrapper::Create<ScriptResourceDragAndDropData>(nativeObject, scriptObject);
	}

	MonoArray* ScriptResourceDragAndDropData::InternalGetRelativeResourcePaths(ScriptResourceDragAndDropData* self)
	{
		Vector<Path> nativeArray__output;
		if(!self->IsNativeObjectValid())
			return {};

		nativeArray__output = static_cast<ResourceDragAndDropData*>(self->GetNativeObject())->RelativeResourcePaths;

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

	void ScriptResourceDragAndDropData::InternalSetRelativeResourcePaths(ScriptResourceDragAndDropData* self, MonoArray* value)
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
		static_cast<ResourceDragAndDropData*>(self->GetNativeObject())->RelativeResourcePaths = nativeArrayvalue;
	}
}
