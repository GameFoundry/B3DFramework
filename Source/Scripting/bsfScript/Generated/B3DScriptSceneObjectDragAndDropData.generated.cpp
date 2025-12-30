//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptSceneObjectDragAndDropData.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "Wrappers/B3DScriptSceneObject.h"

namespace b3d
{
	ScriptSceneObjectDragAndDropData::ScriptSceneObjectDragAndDropData(const SPtr<SceneObjectDragAndDropData>& nativeObject)
		:TScriptReflectableWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptSceneObjectDragAndDropData::~ScriptSceneObjectDragAndDropData()
	{
		UnregisterEvents();
	}

	void ScriptSceneObjectDragAndDropData::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SceneObjectDragAndDropData", (void*)&ScriptSceneObjectDragAndDropData::InternalSceneObjectDragAndDropData);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SceneObjectDragAndDropData0", (void*)&ScriptSceneObjectDragAndDropData::InternalSceneObjectDragAndDropData0);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SceneObjectDragAndDropData1", (void*)&ScriptSceneObjectDragAndDropData::InternalSceneObjectDragAndDropData1);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetSceneObjects", (void*)&ScriptSceneObjectDragAndDropData::InternalGetSceneObjects);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetSceneObjects", (void*)&ScriptSceneObjectDragAndDropData::InternalSetSceneObjects);

	}

	MonoObject* ScriptSceneObjectDragAndDropData::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	void ScriptSceneObjectDragAndDropData::InternalSceneObjectDragAndDropData(MonoObject* scriptObject)
	{
		SPtr<SceneObjectDragAndDropData> nativeObject = B3DMakeShared<SceneObjectDragAndDropData>();
		ScriptObjectWrapper::Create<ScriptSceneObjectDragAndDropData>(nativeObject, scriptObject);
	}

	void ScriptSceneObjectDragAndDropData::InternalSceneObjectDragAndDropData0(MonoObject* scriptObject, MonoObject* sceneObject)
	{
		TGameObjectHandle<SceneObject> tmpsceneObject;
		ScriptSceneObject* scriptObjectWrappersceneObject;
		scriptObjectWrappersceneObject = ScriptSceneObject::GetScriptObjectWrapper(sceneObject);
		if(scriptObjectWrappersceneObject != nullptr)
			tmpsceneObject = B3DStaticGameObjectCast<SceneObject>(scriptObjectWrappersceneObject->GetBaseNativeObjectAsHandle());
		SPtr<SceneObjectDragAndDropData> nativeObject = B3DMakeShared<SceneObjectDragAndDropData>(tmpsceneObject);
		ScriptObjectWrapper::Create<ScriptSceneObjectDragAndDropData>(nativeObject, scriptObject);
	}

	void ScriptSceneObjectDragAndDropData::InternalSceneObjectDragAndDropData1(MonoObject* scriptObject, MonoArray* sceneObjects)
	{
		Vector<TGameObjectHandle<SceneObject>> nativeArraysceneObjects;
		if(sceneObjects != nullptr)
		{
			ScriptArray scriptArraysceneObjects(sceneObjects);
			nativeArraysceneObjects.resize(scriptArraysceneObjects.Size());
			for(int elementIndex = 0; elementIndex < (int)scriptArraysceneObjects.Size(); elementIndex++)
			{
				TGameObjectHandle<SceneObject> arrayElementPointersceneObjects;
				ScriptSceneObject* scriptObjectWrappersceneObjects;
				scriptObjectWrappersceneObjects = ScriptSceneObject::GetScriptObjectWrapper(scriptArraysceneObjects.Get<MonoObject*>(elementIndex));
				if(scriptObjectWrappersceneObjects != nullptr)
				{
					arrayElementPointersceneObjects = B3DStaticGameObjectCast<SceneObject>(scriptObjectWrappersceneObjects->GetBaseNativeObjectAsHandle());
					nativeArraysceneObjects[elementIndex] = arrayElementPointersceneObjects;
				}
			}
		}
		SPtr<SceneObjectDragAndDropData> nativeObject = B3DMakeShared<SceneObjectDragAndDropData>(nativeArraysceneObjects);
		ScriptObjectWrapper::Create<ScriptSceneObjectDragAndDropData>(nativeObject, scriptObject);
	}

	MonoArray* ScriptSceneObjectDragAndDropData::InternalGetSceneObjects(ScriptSceneObjectDragAndDropData* self)
	{
		Vector<TGameObjectHandle<SceneObject>> nativeArray__output;
		if(!self->IsNativeObjectValid())
			return {};

		nativeArray__output = static_cast<SceneObjectDragAndDropData*>(self->GetNativeObject())->SceneObjects;

		MonoArray* __output;
		int elementCount__output = (int)nativeArray__output.size();
		ScriptArray scriptArray__output = ScriptArray::Create<ScriptSceneObject>(elementCount__output);
		for(int elementIndex = 0; elementIndex < elementCount__output; elementIndex++)
		{
			MonoObject* tempscriptArray__output = nullptr;
			if(nativeArray__output[elementIndex])
			tempscriptArray__output = ScriptSceneObject::GetOrCreateScriptObject(nativeArray__output[elementIndex]);
			scriptArray__output.Set(elementIndex, tempscriptArray__output);
		}
		__output = scriptArray__output.GetInternal();

		return __output;
	}

	void ScriptSceneObjectDragAndDropData::InternalSetSceneObjects(ScriptSceneObjectDragAndDropData* self, MonoArray* value)
	{
		if(!self->IsNativeObjectValid())
			return;

		Vector<TGameObjectHandle<SceneObject>> nativeArrayvalue;
		if(value != nullptr)
		{
			ScriptArray scriptArrayvalue(value);
			nativeArrayvalue.resize(scriptArrayvalue.Size());
			for(int elementIndex = 0; elementIndex < (int)scriptArrayvalue.Size(); elementIndex++)
			{
				TGameObjectHandle<SceneObject> arrayElementPointervalue;
				ScriptSceneObject* scriptObjectWrappervalue;
				scriptObjectWrappervalue = ScriptSceneObject::GetScriptObjectWrapper(scriptArrayvalue.Get<MonoObject*>(elementIndex));
				if(scriptObjectWrappervalue != nullptr)
				{
					arrayElementPointervalue = B3DStaticGameObjectCast<SceneObject>(scriptObjectWrappervalue->GetBaseNativeObjectAsHandle());
					nativeArrayvalue[elementIndex] = arrayElementPointervalue;
				}
			}

		}
		static_cast<SceneObjectDragAndDropData*>(self->GetNativeObject())->SceneObjects = nativeArrayvalue;
	}
}
