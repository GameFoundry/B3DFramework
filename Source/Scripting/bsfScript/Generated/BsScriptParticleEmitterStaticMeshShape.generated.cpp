//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptParticleEmitterStaticMeshShape.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "BsScriptPARTICLE_STATIC_MESH_SHAPE_DESC.generated.h"
#include "BsScriptParticleEmitterStaticMeshShape.generated.h"

namespace bs
{
	ScriptParticleEmitterStaticMeshShape::ScriptParticleEmitterStaticMeshShape(MonoObject* managedInstance, const SPtr<ParticleEmitterStaticMeshShape>& value)
		:TScriptReflectable(managedInstance, value)
	{
		mInternal = value;
	}

	void ScriptParticleEmitterStaticMeshShape::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_setOptions", (void*)&ScriptParticleEmitterStaticMeshShape::Internal_setOptions);
		metaData.scriptClass->addInternalCall("Internal_getOptions", (void*)&ScriptParticleEmitterStaticMeshShape::Internal_getOptions);
		metaData.scriptClass->addInternalCall("Internal_create", (void*)&ScriptParticleEmitterStaticMeshShape::Internal_create);
		metaData.scriptClass->addInternalCall("Internal_create0", (void*)&ScriptParticleEmitterStaticMeshShape::Internal_create0);

	}

	MonoObject* ScriptParticleEmitterStaticMeshShape::create(const SPtr<ParticleEmitterStaticMeshShape>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.scriptClass->createInstance("bool", ctorParams);
		new (bs_alloc<ScriptParticleEmitterStaticMeshShape>()) ScriptParticleEmitterStaticMeshShape(managedInstance, value);
		return managedInstance;
	}
	void ScriptParticleEmitterStaticMeshShape::Internal_setOptions(ScriptParticleEmitterStaticMeshShape* thisPtr, __PARTICLE_STATIC_MESH_SHAPE_DESCInterop* options)
	{
		PARTICLE_STATIC_MESH_SHAPE_DESC tmpoptions;
		tmpoptions = ScriptPARTICLE_STATIC_MESH_SHAPE_DESC::fromInterop(*options);
		thisPtr->getInternal()->setOptions(tmpoptions);
	}

	void ScriptParticleEmitterStaticMeshShape::Internal_getOptions(ScriptParticleEmitterStaticMeshShape* thisPtr, __PARTICLE_STATIC_MESH_SHAPE_DESCInterop* __output)
	{
		PARTICLE_STATIC_MESH_SHAPE_DESC tmp__output;
		tmp__output = thisPtr->getInternal()->getOptions();

		__PARTICLE_STATIC_MESH_SHAPE_DESCInterop interop__output;
		interop__output = ScriptPARTICLE_STATIC_MESH_SHAPE_DESC::toInterop(tmp__output);
		MonoUtil::valueCopy(__output, &interop__output, ScriptPARTICLE_STATIC_MESH_SHAPE_DESC::getMetaData()->scriptClass->_getInternalClass());
	}

	void ScriptParticleEmitterStaticMeshShape::Internal_create(MonoObject* managedInstance, __PARTICLE_STATIC_MESH_SHAPE_DESCInterop* desc)
	{
		PARTICLE_STATIC_MESH_SHAPE_DESC tmpdesc;
		tmpdesc = ScriptPARTICLE_STATIC_MESH_SHAPE_DESC::fromInterop(*desc);
		SPtr<ParticleEmitterStaticMeshShape> instance = ParticleEmitterStaticMeshShape::create(tmpdesc);
		new (bs_alloc<ScriptParticleEmitterStaticMeshShape>())ScriptParticleEmitterStaticMeshShape(managedInstance, instance);
	}

	void ScriptParticleEmitterStaticMeshShape::Internal_create0(MonoObject* managedInstance)
	{
		SPtr<ParticleEmitterStaticMeshShape> instance = ParticleEmitterStaticMeshShape::create();
		new (bs_alloc<ScriptParticleEmitterStaticMeshShape>())ScriptParticleEmitterStaticMeshShape(managedInstance, instance);
	}
}
