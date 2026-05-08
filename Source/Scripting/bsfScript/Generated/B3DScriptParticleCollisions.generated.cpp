//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptParticleCollisions.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "B3DScriptTPlane.generated.h"
#include "Wrappers/B3DScriptSceneObject.h"
#include "B3DScriptParticleCollisionSettings.generated.h"
#include "B3DScriptParticleCollisions.generated.h"

namespace b3d
{
	ScriptParticleCollisions::ScriptParticleCollisions(const TShared<ParticleCollisions>& nativeObject)
		:TScriptReflectableWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptParticleCollisions::~ScriptParticleCollisions()
	{
		UnregisterEvents();
	}

	void ScriptParticleCollisions::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetPlanes", (void*)&ScriptParticleCollisions::InternalSetPlanes);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetPlanes", (void*)&ScriptParticleCollisions::InternalGetPlanes);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetPlaneObjects", (void*)&ScriptParticleCollisions::InternalSetPlaneObjects);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetPlaneObjects", (void*)&ScriptParticleCollisions::InternalGetPlaneObjects);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetSettings", (void*)&ScriptParticleCollisions::InternalSetSettings);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetSettings", (void*)&ScriptParticleCollisions::InternalGetSettings);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create", (void*)&ScriptParticleCollisions::InternalCreate);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create0", (void*)&ScriptParticleCollisions::InternalCreate0);

	}

	MonoObject* ScriptParticleCollisions::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	void ScriptParticleCollisions::InternalSetPlanes(ScriptParticleCollisions* self, MonoArray* planes)
	{
		if(!self->IsNativeObjectValid())
			return;

		Vector<TPlane<float>> nativeArrayplanes;
		if(planes != nullptr)
		{
			ScriptArray scriptArrayplanes(planes);
			nativeArrayplanes.resize(scriptArrayplanes.Size());
			for(int elementIndex = 0; elementIndex < (int)scriptArrayplanes.Size(); elementIndex++)
			{
				nativeArrayplanes[elementIndex] = ScriptPlane::FromInterop(scriptArrayplanes.Get<__TPlane_float_Interop>(elementIndex));
			}
		}
		static_cast<ParticleCollisions*>(self->GetNativeObject())->SetPlanes(nativeArrayplanes);
	}

	MonoArray* ScriptParticleCollisions::InternalGetPlanes(ScriptParticleCollisions* self)
	{
		Vector<TPlane<float>> nativeArray__output;
		if(!self->IsNativeObjectValid())
			return {};

		nativeArray__output = static_cast<ParticleCollisions*>(self->GetNativeObject())->GetPlanes();

		MonoArray* __output;
		int elementCount__output = (int)nativeArray__output.size();
		ScriptArray scriptArray__output = ScriptArray::Create<ScriptPlane>(elementCount__output);
		for(int elementIndex = 0; elementIndex < elementCount__output; elementIndex++)
		{
			scriptArray__output.Set(elementIndex, ScriptPlane::ToInterop(nativeArray__output[elementIndex]));
		}
		__output = scriptArray__output.GetInternal();

		return __output;
	}

	void ScriptParticleCollisions::InternalSetPlaneObjects(ScriptParticleCollisions* self, MonoArray* objects)
	{
		if(!self->IsNativeObjectValid())
			return;

		Vector<TGameObjectHandle<SceneObject>> nativeArrayobjects;
		if(objects != nullptr)
		{
			ScriptArray scriptArrayobjects(objects);
			nativeArrayobjects.resize(scriptArrayobjects.Size());
			for(int elementIndex = 0; elementIndex < (int)scriptArrayobjects.Size(); elementIndex++)
			{
				TGameObjectHandle<SceneObject> arrayElementPointerobjects;
				ScriptSceneObject* scriptObjectWrapperobjects;
				scriptObjectWrapperobjects = ScriptSceneObject::GetScriptObjectWrapper(scriptArrayobjects.Get<MonoObject*>(elementIndex));
				if(scriptObjectWrapperobjects != nullptr)
				{
					arrayElementPointerobjects = B3DStaticGameObjectCast<SceneObject>(scriptObjectWrapperobjects->GetBaseNativeObjectAsHandle());
					nativeArrayobjects[elementIndex] = arrayElementPointerobjects;
				}
			}
		}
		static_cast<ParticleCollisions*>(self->GetNativeObject())->SetPlaneObjects(nativeArrayobjects);
	}

	MonoArray* ScriptParticleCollisions::InternalGetPlaneObjects(ScriptParticleCollisions* self)
	{
		Vector<TGameObjectHandle<SceneObject>> nativeArray__output;
		if(!self->IsNativeObjectValid())
			return {};

		nativeArray__output = static_cast<ParticleCollisions*>(self->GetNativeObject())->GetPlaneObjects();

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

	void ScriptParticleCollisions::InternalSetSettings(ScriptParticleCollisions* self, ParticleCollisionSettings* settings)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<ParticleCollisions*>(self->GetNativeObject())->SetSettings(*settings);
	}

	void ScriptParticleCollisions::InternalGetSettings(ScriptParticleCollisions* self, ParticleCollisionSettings* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		ParticleCollisionSettings tmp__output;
		tmp__output = static_cast<ParticleCollisions*>(self->GetNativeObject())->GetSettings();

		*__output = tmp__output;
	}

	void ScriptParticleCollisions::InternalCreate(MonoObject* scriptObject, ParticleCollisionSettings* settings)
	{
		TShared<ParticleCollisions> nativeObject = ParticleCollisions::Create(*settings);
		ScriptObjectWrapper::Create<ScriptParticleCollisions>(nativeObject, scriptObject);
	}

	void ScriptParticleCollisions::InternalCreate0(MonoObject* scriptObject)
	{
		TShared<ParticleCollisions> nativeObject = ParticleCollisions::Create();
		ScriptObjectWrapper::Create<ScriptParticleCollisions>(nativeObject, scriptObject);
	}
}
