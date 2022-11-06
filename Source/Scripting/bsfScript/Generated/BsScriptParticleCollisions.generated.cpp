//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptParticleCollisions.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "BsScriptGameObjectManager.h"
#include "Wrappers/BsScriptPlane.h"
#include "Wrappers/BsScriptSceneObject.h"
#include "BsScriptPARTICLE_COLLISIONS_DESC.generated.h"
#include "BsScriptParticleCollisions.generated.h"

namespace bs
{
	ScriptParticleCollisions::ScriptParticleCollisions(MonoObject* managedInstance, const SPtr<ParticleCollisions>& value)
		:TScriptReflectable(managedInstance, value)
	{
		mInternal = value;
	}

	void ScriptParticleCollisions::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_SetPlanes", (void*)&ScriptParticleCollisions::InternalSetPlanes);
		metaData.ScriptClass->AddInternalCall("Internal_GetPlanes", (void*)&ScriptParticleCollisions::InternalGetPlanes);
		metaData.ScriptClass->AddInternalCall("Internal_SetPlaneObjects", (void*)&ScriptParticleCollisions::InternalSetPlaneObjects);
		metaData.ScriptClass->AddInternalCall("Internal_GetPlaneObjects", (void*)&ScriptParticleCollisions::InternalGetPlaneObjects);
		metaData.ScriptClass->AddInternalCall("Internal_SetOptions", (void*)&ScriptParticleCollisions::InternalSetOptions);
		metaData.ScriptClass->AddInternalCall("Internal_GetOptions", (void*)&ScriptParticleCollisions::InternalGetOptions);
		metaData.ScriptClass->AddInternalCall("Internal_Create", (void*)&ScriptParticleCollisions::InternalCreate);
		metaData.ScriptClass->AddInternalCall("Internal_Create0", (void*)&ScriptParticleCollisions::InternalCreate0);

	}

	MonoObject* ScriptParticleCollisions::Create(const SPtr<ParticleCollisions>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.ScriptClass->CreateInstance("bool", ctorParams);
		new (B3DAllocate<ScriptParticleCollisions>()) ScriptParticleCollisions(managedInstance, value);
		return managedInstance;
	}
	void ScriptParticleCollisions::InternalSetPlanes(ScriptParticleCollisions* thisPtr, MonoArray* planes)
	{
		Vector<Plane> vecplanes;
		if(planes != nullptr)
		{
			ScriptArray arrayplanes(planes);
			vecplanes.resize(arrayplanes.Size());
			for(int i = 0; i < (int)arrayplanes.Size(); i++)
			{
				vecplanes[i] = arrayplanes.Get<Plane>(i);
			}
		}
		thisPtr->GetInternal()->SetPlanes(vecplanes);
	}

	MonoArray* ScriptParticleCollisions::InternalGetPlanes(ScriptParticleCollisions* thisPtr)
	{
		Vector<Plane> vec__output;
		vec__output = thisPtr->GetInternal()->GetPlanes();

		MonoArray* __output;
		int arraySize__output = (int)vec__output.size();
		ScriptArray array__output = ScriptArray::Create<ScriptPlane>(arraySize__output);
		for(int i = 0; i < arraySize__output; i++)
		{
			array__output.Set(i, vec__output[i]);
		}
		__output = array__output.GetInternal();

		return __output;
	}

	void ScriptParticleCollisions::InternalSetPlaneObjects(ScriptParticleCollisions* thisPtr, MonoArray* objects)
	{
		Vector<GameObjectHandle<SceneObject>> vecobjects;
		if(objects != nullptr)
		{
			ScriptArray arrayobjects(objects);
			vecobjects.resize(arrayobjects.Size());
			for(int i = 0; i < (int)arrayobjects.Size(); i++)
			{
				ScriptSceneObject* scriptobjects;
				scriptobjects = ScriptSceneObject::ToNative(arrayobjects.Get<MonoObject*>(i));
				if(scriptobjects != nullptr)
				{
					GameObjectHandle<SceneObject> arrayElemPtrobjects = scriptobjects->GetHandle();
					vecobjects[i] = arrayElemPtrobjects;
				}
			}
		}
		thisPtr->GetInternal()->SetPlaneObjects(vecobjects);
	}

	MonoArray* ScriptParticleCollisions::InternalGetPlaneObjects(ScriptParticleCollisions* thisPtr)
	{
		Vector<GameObjectHandle<SceneObject>> vec__output;
		vec__output = thisPtr->GetInternal()->GetPlaneObjects();

		MonoArray* __output;
		int arraySize__output = (int)vec__output.size();
		ScriptArray array__output = ScriptArray::Create<ScriptSceneObject>(arraySize__output);
		for(int i = 0; i < arraySize__output; i++)
		{
			ScriptSceneObject* script__output = nullptr;
			if(vec__output[i])
			script__output = ScriptGameObjectManager::Instance().GetOrCreateScriptSceneObject(vec__output[i]);
			if(script__output != nullptr)
				array__output.Set(i, script__output->GetManagedInstance());
			else
				array__output.Set(i, nullptr);
		}
		__output = array__output.GetInternal();

		return __output;
	}

	void ScriptParticleCollisions::InternalSetOptions(ScriptParticleCollisions* thisPtr, PARTICLE_COLLISIONS_DESC* options)
	{
		thisPtr->GetInternal()->SetOptions(*options);
	}

	void ScriptParticleCollisions::InternalGetOptions(ScriptParticleCollisions* thisPtr, PARTICLE_COLLISIONS_DESC* __output)
	{
		PARTICLE_COLLISIONS_DESC tmp__output;
		tmp__output = thisPtr->GetInternal()->GetOptions();

		*__output = tmp__output;
	}

	void ScriptParticleCollisions::InternalCreate(MonoObject* managedInstance, PARTICLE_COLLISIONS_DESC* desc)
	{
		SPtr<ParticleCollisions> instance = ParticleCollisions::Create(*desc);
		new (B3DAllocate<ScriptParticleCollisions>())ScriptParticleCollisions(managedInstance, instance);
	}

	void ScriptParticleCollisions::InternalCreate0(MonoObject* managedInstance)
	{
		SPtr<ParticleCollisions> instance = ParticleCollisions::Create();
		new (B3DAllocate<ScriptParticleCollisions>())ScriptParticleCollisions(managedInstance, instance);
	}
}
