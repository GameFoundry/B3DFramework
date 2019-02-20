//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptParticleEmitterSkinnedMeshShape.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "BsScriptPARTICLE_SKINNED_MESH_SHAPE_DESC.generated.h"
#include "BsScriptParticleEmitterSkinnedMeshShape.generated.h"

namespace bs
{
	ScriptParticleEmitterSkinnedMeshShape::ScriptParticleEmitterSkinnedMeshShape(MonoObject* managedInstance, const SPtr<ParticleEmitterSkinnedMeshShape>& value)
		:TScriptReflectable(managedInstance, value)
	{
		mInternal = value;
	}

	void ScriptParticleEmitterSkinnedMeshShape::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_setOptions", (void*)&ScriptParticleEmitterSkinnedMeshShape::Internal_setOptions);
		metaData.scriptClass->addInternalCall("Internal_getOptions", (void*)&ScriptParticleEmitterSkinnedMeshShape::Internal_getOptions);
		metaData.scriptClass->addInternalCall("Internal_create", (void*)&ScriptParticleEmitterSkinnedMeshShape::Internal_create);
		metaData.scriptClass->addInternalCall("Internal_create0", (void*)&ScriptParticleEmitterSkinnedMeshShape::Internal_create0);

	}

	MonoObject* ScriptParticleEmitterSkinnedMeshShape::create(const SPtr<ParticleEmitterSkinnedMeshShape>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.scriptClass->createInstance("bool", ctorParams);
		new (bs_alloc<ScriptParticleEmitterSkinnedMeshShape>()) ScriptParticleEmitterSkinnedMeshShape(managedInstance, value);
		return managedInstance;
	}
	void ScriptParticleEmitterSkinnedMeshShape::Internal_setOptions(ScriptParticleEmitterSkinnedMeshShape* thisPtr, __PARTICLE_SKINNED_MESH_SHAPE_DESCInterop* options)
	{
		PARTICLE_SKINNED_MESH_SHAPE_DESC tmpoptions;
		tmpoptions = ScriptPARTICLE_SKINNED_MESH_SHAPE_DESC::fromInterop(*options);
		thisPtr->getInternal()->setOptions(tmpoptions);
	}

	void ScriptParticleEmitterSkinnedMeshShape::Internal_getOptions(ScriptParticleEmitterSkinnedMeshShape* thisPtr, __PARTICLE_SKINNED_MESH_SHAPE_DESCInterop* __output)
	{
		PARTICLE_SKINNED_MESH_SHAPE_DESC tmp__output;
		tmp__output = thisPtr->getInternal()->getOptions();

		__PARTICLE_SKINNED_MESH_SHAPE_DESCInterop interop__output;
		interop__output = ScriptPARTICLE_SKINNED_MESH_SHAPE_DESC::toInterop(tmp__output);
		MonoUtil::valueCopy(__output, &interop__output, ScriptPARTICLE_SKINNED_MESH_SHAPE_DESC::getMetaData()->scriptClass->_getInternalClass());
	}

	void ScriptParticleEmitterSkinnedMeshShape::Internal_create(MonoObject* managedInstance, __PARTICLE_SKINNED_MESH_SHAPE_DESCInterop* desc)
	{
		PARTICLE_SKINNED_MESH_SHAPE_DESC tmpdesc;
		tmpdesc = ScriptPARTICLE_SKINNED_MESH_SHAPE_DESC::fromInterop(*desc);
		SPtr<ParticleEmitterSkinnedMeshShape> instance = ParticleEmitterSkinnedMeshShape::create(tmpdesc);
		new (bs_alloc<ScriptParticleEmitterSkinnedMeshShape>())ScriptParticleEmitterSkinnedMeshShape(managedInstance, instance);
	}

	void ScriptParticleEmitterSkinnedMeshShape::Internal_create0(MonoObject* managedInstance)
	{
		SPtr<ParticleEmitterSkinnedMeshShape> instance = ParticleEmitterSkinnedMeshShape::create();
		new (bs_alloc<ScriptParticleEmitterSkinnedMeshShape>())ScriptParticleEmitterSkinnedMeshShape(managedInstance, instance);
	}
}
