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
		Vector<Plane> nativeArrayplanes;
		if(planes != nullptr)
		{
			ScriptArray scriptArrayplanes(planes);
			nativeArrayplanes.resize(scriptArrayplanes.Size());
			for(int elementIndex = 0; elementIndex < (int)scriptArrayplanes.Size(); elementIndex++)
			{
				nativeArrayplanes[elementIndex] = scriptArrayplanes.Get<Plane>(elementIndex);
			}
		}
		thisPtr->GetInternal()->SetPlanes(nativeArrayplanes);
	}

	MonoArray* ScriptParticleCollisions::InternalGetPlanes(ScriptParticleCollisions* thisPtr)
	{
		Vector<Plane> nativeArray__output;
		nativeArray__output = thisPtr->GetInternal()->GetPlanes();

		MonoArray* __output;
		int elementCount__output = (int)nativeArray__output.size();
		ScriptArray scriptArray__output = ScriptArray::Create<ScriptPlane>(elementCount__output);
		for(int elementIndex = 0; elementIndex < elementCount__output; elementIndex++)
		{
			scriptArray__output.Set(elementIndex, nativeArray__output[elementIndex]);
		}
		__output = scriptArray__output.GetInternal();

		return __output;
	}

	void ScriptParticleCollisions::InternalSetPlaneObjects(ScriptParticleCollisions* thisPtr, MonoArray* objects)
	{
		Vector<GameObjectHandle<SceneObject>> nativeArrayobjects;
		if(objects != nullptr)
		{
			ScriptArray scriptArrayobjects(objects);
			nativeArrayobjects.resize(scriptArrayobjects.Size());
			for(int elementIndex = 0; elementIndex < (int)scriptArrayobjects.Size(); elementIndex++)
			{
				ScriptSceneObject* scriptObjectWrapperobjects;
				scriptObjectWrapperobjects = ScriptSceneObject::ToNative(scriptArrayobjects.Get<MonoObject*>(elementIndex));
				if(scriptObjectWrapperobjects != nullptr)
				{
					GameObjectHandle<SceneObject> arrayElementPointerobjects = scriptObjectWrapperobjects->GetHandle();
					nativeArrayobjects[elementIndex] = arrayElementPointerobjects;
				}
			}
		}
		thisPtr->GetInternal()->SetPlaneObjects(nativeArrayobjects);
	}

	MonoArray* ScriptParticleCollisions::InternalGetPlaneObjects(ScriptParticleCollisions* thisPtr)
	{
		Vector<GameObjectHandle<SceneObject>> nativeArray__output;
		nativeArray__output = thisPtr->GetInternal()->GetPlaneObjects();

		MonoArray* __output;
		int elementCount__output = (int)nativeArray__output.size();
		ScriptArray scriptArray__output = ScriptArray::Create<ScriptSceneObject>(elementCount__output);
		for(int elementIndex = 0; elementIndex < elementCount__output; elementIndex++)
		{
			ScriptSceneObject* scriptObjectWrapper__output = nullptr;
			if(nativeArray__output[elementIndex])
			scriptObjectWrapper__output = ScriptGameObjectManager::Instance().GetOrCreateScriptSceneObject(nativeArray__output[elementIndex]);
			if(scriptObjectWrapper__output != nullptr)
				scriptArray__output.Set(elementIndex, scriptObjectWrapper__output->GetManagedInstance());
			else
				scriptArray__output.Set(elementIndex, nullptr);
		}
		__output = scriptArray__output.GetInternal();

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
