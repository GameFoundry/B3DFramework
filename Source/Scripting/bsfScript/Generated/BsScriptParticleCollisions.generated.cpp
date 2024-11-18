//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptParticleCollisions.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "BsScriptTPlane.generated.h"
#include "Wrappers/BsScriptSceneObject.h"
#include "BsScriptPARTICLE_COLLISIONS_DESC.generated.h"
#include "BsScriptParticleCollisions.generated.h"

namespace bs
{
	ScriptParticleCollisions::ScriptParticleCollisions(const SPtr<ParticleCollisions>& nativeObject)
		:TScriptReflectableWrapper(nativeObject)
	{
		RegisterEvents();
	}

	void ScriptParticleCollisions::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetPlanes", (void*)&ScriptParticleCollisions::InternalSetPlanes);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetPlanes", (void*)&ScriptParticleCollisions::InternalGetPlanes);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetPlaneObjects", (void*)&ScriptParticleCollisions::InternalSetPlaneObjects);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetPlaneObjects", (void*)&ScriptParticleCollisions::InternalGetPlaneObjects);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetOptions", (void*)&ScriptParticleCollisions::InternalSetOptions);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetOptions", (void*)&ScriptParticleCollisions::InternalGetOptions);
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

		Vector<GameObjectHandle<SceneObject>> nativeArrayobjects;
		if(objects != nullptr)
		{
			ScriptArray scriptArrayobjects(objects);
			nativeArrayobjects.resize(scriptArrayobjects.Size());
			for(int elementIndex = 0; elementIndex < (int)scriptArrayobjects.Size(); elementIndex++)
			{
				ScriptSceneObject* scriptObjectWrapperobjects;
				scriptObjectWrapperobjects = ScriptSceneObject::GetScriptObjectWrapper(scriptArrayobjects.Get<MonoObject*>(elementIndex));
				if(scriptObjectWrapperobjects != nullptr)
				{
					GameObjectHandle<SceneObject> arrayElementPointerobjects = B3DStaticGameObjectCast<SceneObject>(scriptObjectWrapperobjects->GetBaseNativeObjectAsHandle());
					nativeArrayobjects[elementIndex] = arrayElementPointerobjects;
				}
			}
		}
		static_cast<ParticleCollisions*>(self->GetNativeObject())->SetPlaneObjects(nativeArrayobjects);
	}

	MonoArray* ScriptParticleCollisions::InternalGetPlaneObjects(ScriptParticleCollisions* self)
	{
		Vector<GameObjectHandle<SceneObject>> nativeArray__output;
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

	void ScriptParticleCollisions::InternalSetOptions(ScriptParticleCollisions* self, PARTICLE_COLLISIONS_DESC* options)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<ParticleCollisions*>(self->GetNativeObject())->SetOptions(*options);
	}

	void ScriptParticleCollisions::InternalGetOptions(ScriptParticleCollisions* self, PARTICLE_COLLISIONS_DESC* __output)
	{
		if(!self->IsNativeObjectValid())
			{
				__output = {};
				return;
			}

		PARTICLE_COLLISIONS_DESC tmp__output;
		tmp__output = static_cast<ParticleCollisions*>(self->GetNativeObject())->GetOptions();

		*__output = tmp__output;
	}

	void ScriptParticleCollisions::InternalCreate(MonoObject* scriptObject, PARTICLE_COLLISIONS_DESC* desc)
	{
		SPtr<ParticleCollisions> nativeObject = ParticleCollisions::Create(*desc);
		ScriptObjectWrapper::Create<ScriptParticleCollisions>(nativeObject, scriptObject);
	}

	void ScriptParticleCollisions::InternalCreate0(MonoObject* scriptObject)
	{
		SPtr<ParticleCollisions> nativeObject = ParticleCollisions::Create();
		ScriptObjectWrapper::Create<ScriptParticleCollisions>(nativeObject, scriptObject);
	}
}
