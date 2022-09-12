//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptParticleEmitter.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "Reflection/BsRTTIType.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEmitter.h"
#include "BsScriptParticleEmitterHemisphereShape.generated.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEmitter.h"
#include "BsScriptParticleEmitterCircleShape.generated.h"
#include "BsScriptParticleEmitterShape.generated.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEmitter.h"
#include "BsScriptParticleEmitterStaticMeshShape.generated.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEmitter.h"
#include "BsScriptParticleEmitterSkinnedMeshShape.generated.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEmitter.h"
#include "BsScriptParticleEmitterBoxShape.generated.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEmitter.h"
#include "BsScriptParticleEmitterConeShape.generated.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEmitter.h"
#include "BsScriptParticleEmitterSphereShape.generated.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEmitter.h"
#include "BsScriptParticleEmitterLineShape.generated.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEmitter.h"
#include "BsScriptParticleEmitterRectShape.generated.h"
#include "BsScriptTDistribution.generated.h"
#include "BsScriptParticleBurst.generated.h"
#include "BsScriptTDistribution.generated.h"
#include "BsScriptTColorDistribution.generated.h"
#include "BsScriptParticleEmitter.generated.h"

namespace bs
{
	ScriptParticleEmitter::ScriptParticleEmitter(MonoObject* managedInstance, const SPtr<ParticleEmitter>& value)
		:TScriptReflectable(managedInstance, value)
	{
	}

	void ScriptParticleEmitter::InitRuntimeData()
	{
		metaData.scriptClass->AddInternalCall("Internal_setShape", (void*)&ScriptParticleEmitter::Internal_setShape);
		metaData.scriptClass->AddInternalCall("Internal_getShape", (void*)&ScriptParticleEmitter::Internal_getShape);
		metaData.scriptClass->AddInternalCall("Internal_setEmissionRate", (void*)&ScriptParticleEmitter::Internal_setEmissionRate);
		metaData.scriptClass->AddInternalCall("Internal_getEmissionRate", (void*)&ScriptParticleEmitter::Internal_getEmissionRate);
		metaData.scriptClass->AddInternalCall("Internal_setEmissionBursts", (void*)&ScriptParticleEmitter::Internal_setEmissionBursts);
		metaData.scriptClass->AddInternalCall("Internal_getEmissionBursts", (void*)&ScriptParticleEmitter::Internal_getEmissionBursts);
		metaData.scriptClass->AddInternalCall("Internal_setInitialLifetime", (void*)&ScriptParticleEmitter::Internal_setInitialLifetime);
		metaData.scriptClass->AddInternalCall("Internal_getInitialLifetime", (void*)&ScriptParticleEmitter::Internal_getInitialLifetime);
		metaData.scriptClass->AddInternalCall("Internal_setInitialSpeed", (void*)&ScriptParticleEmitter::Internal_setInitialSpeed);
		metaData.scriptClass->AddInternalCall("Internal_getInitialSpeed", (void*)&ScriptParticleEmitter::Internal_getInitialSpeed);
		metaData.scriptClass->AddInternalCall("Internal_setInitialSize", (void*)&ScriptParticleEmitter::Internal_setInitialSize);
		metaData.scriptClass->AddInternalCall("Internal_getInitialSize", (void*)&ScriptParticleEmitter::Internal_getInitialSize);
		metaData.scriptClass->AddInternalCall("Internal_setInitialSize3D", (void*)&ScriptParticleEmitter::Internal_setInitialSize3D);
		metaData.scriptClass->AddInternalCall("Internal_getInitialSize3D", (void*)&ScriptParticleEmitter::Internal_getInitialSize3D);
		metaData.scriptClass->AddInternalCall("Internal_setUse3DSize", (void*)&ScriptParticleEmitter::Internal_setUse3DSize);
		metaData.scriptClass->AddInternalCall("Internal_getUse3DSize", (void*)&ScriptParticleEmitter::Internal_getUse3DSize);
		metaData.scriptClass->AddInternalCall("Internal_setInitialRotation", (void*)&ScriptParticleEmitter::Internal_setInitialRotation);
		metaData.scriptClass->AddInternalCall("Internal_getInitialRotation", (void*)&ScriptParticleEmitter::Internal_getInitialRotation);
		metaData.scriptClass->AddInternalCall("Internal_setInitialRotation3D", (void*)&ScriptParticleEmitter::Internal_setInitialRotation3D);
		metaData.scriptClass->AddInternalCall("Internal_getInitialRotation3D", (void*)&ScriptParticleEmitter::Internal_getInitialRotation3D);
		metaData.scriptClass->AddInternalCall("Internal_setUse3DRotation", (void*)&ScriptParticleEmitter::Internal_setUse3DRotation);
		metaData.scriptClass->AddInternalCall("Internal_getUse3DRotation", (void*)&ScriptParticleEmitter::Internal_getUse3DRotation);
		metaData.scriptClass->AddInternalCall("Internal_setInitialColor", (void*)&ScriptParticleEmitter::Internal_setInitialColor);
		metaData.scriptClass->AddInternalCall("Internal_getInitialColor", (void*)&ScriptParticleEmitter::Internal_getInitialColor);
		metaData.scriptClass->AddInternalCall("Internal_setRandomOffset", (void*)&ScriptParticleEmitter::Internal_setRandomOffset);
		metaData.scriptClass->AddInternalCall("Internal_getRandomOffset", (void*)&ScriptParticleEmitter::Internal_getRandomOffset);
		metaData.scriptClass->AddInternalCall("Internal_setFlipU", (void*)&ScriptParticleEmitter::Internal_setFlipU);
		metaData.scriptClass->AddInternalCall("Internal_getFlipU", (void*)&ScriptParticleEmitter::Internal_getFlipU);
		metaData.scriptClass->AddInternalCall("Internal_setFlipV", (void*)&ScriptParticleEmitter::Internal_setFlipV);
		metaData.scriptClass->AddInternalCall("Internal_getFlipV", (void*)&ScriptParticleEmitter::Internal_getFlipV);
		metaData.scriptClass->AddInternalCall("Internal_create", (void*)&ScriptParticleEmitter::Internal_create);

	}

	MonoObject* ScriptParticleEmitter::create(const SPtr<ParticleEmitter>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.scriptClass->CreateInstance("bool", ctorParams);
		new (bs_alloc<ScriptParticleEmitter>()) ScriptParticleEmitter(managedInstance, value);
		return managedInstance;
	}
	void ScriptParticleEmitter::Internal_setShape(ScriptParticleEmitter* thisPtr, MonoObject* shape)
	{
		SPtr<ParticleEmitterShape> tmpshape;
		ScriptParticleEmitterShapeBase* scriptshape;
		scriptshape = (ScriptParticleEmitterShapeBase*)ScriptParticleEmitterShape::toNative(shape);
		if(scriptshape != nullptr)
			tmpshape = scriptshape->GetInternal();
		thisPtr->GetInternal()->setShape(tmpshape);
	}

	MonoObject* ScriptParticleEmitter::Internal_getShape(ScriptParticleEmitter* thisPtr)
	{
		SPtr<ParticleEmitterShape> tmp__output;
		tmp__output = thisPtr->GetInternal()->getShape();

		MonoObject* __output;
		if(tmp__output)
		{
			if(rtti_is_of_type<ParticleEmitterStaticMeshShape>(tmp__output))
				__output = ScriptParticleEmitterStaticMeshShape::create(std::static_pointer_cast<ParticleEmitterStaticMeshShape>(tmp__output));
			else If(rtti_is_of_type<ParticleEmitterCircleShape>(tmp__output))
				__output = ScriptParticleEmitterCircleShape::create(std::static_pointer_cast<ParticleEmitterCircleShape>(tmp__output));
			else If(rtti_is_of_type<ParticleEmitterConeShape>(tmp__output))
				__output = ScriptParticleEmitterConeShape::create(std::static_pointer_cast<ParticleEmitterConeShape>(tmp__output));
			else If(rtti_is_of_type<ParticleEmitterSphereShape>(tmp__output))
				__output = ScriptParticleEmitterSphereShape::create(std::static_pointer_cast<ParticleEmitterSphereShape>(tmp__output));
			else If(rtti_is_of_type<ParticleEmitterHemisphereShape>(tmp__output))
				__output = ScriptParticleEmitterHemisphereShape::create(std::static_pointer_cast<ParticleEmitterHemisphereShape>(tmp__output));
			else If(rtti_is_of_type<ParticleEmitterBoxShape>(tmp__output))
				__output = ScriptParticleEmitterBoxShape::create(std::static_pointer_cast<ParticleEmitterBoxShape>(tmp__output));
			else If(rtti_is_of_type<ParticleEmitterLineShape>(tmp__output))
				__output = ScriptParticleEmitterLineShape::create(std::static_pointer_cast<ParticleEmitterLineShape>(tmp__output));
			else If(rtti_is_of_type<ParticleEmitterRectShape>(tmp__output))
				__output = ScriptParticleEmitterRectShape::create(std::static_pointer_cast<ParticleEmitterRectShape>(tmp__output));
			else If(rtti_is_of_type<ParticleEmitterSkinnedMeshShape>(tmp__output))
				__output = ScriptParticleEmitterSkinnedMeshShape::create(std::static_pointer_cast<ParticleEmitterSkinnedMeshShape>(tmp__output));
			else
				__output = ScriptParticleEmitterShape::create(tmp__output);
		}
		else
			__output = ScriptParticleEmitterShape::create(tmp__output);

		return __output;
	}

	void ScriptParticleEmitter::Internal_setEmissionRate(ScriptParticleEmitter* thisPtr, MonoObject* value)
	{
		SPtr<TDistribution<float>> tmpvalue;
		ScriptTDistributionfloat* scriptvalue;
		scriptvalue = ScriptTDistributionfloat::toNative(value);
		if(scriptvalue != nullptr)
			tmpvalue = scriptvalue->GetInternal();
		thisPtr->GetInternal()->setEmissionRate(*tmpvalue);
	}

	MonoObject* ScriptParticleEmitter::Internal_getEmissionRate(ScriptParticleEmitter* thisPtr)
	{
		SPtr<TDistribution<float>> tmp__output = bs_shared_ptr_new<TDistribution<float>>();
		*tmp__output = thisPtr->GetInternal()->getEmissionRate();

		MonoObject* __output;
		__output = ScriptTDistributionfloat::create(tmp__output);

		return __output;
	}

	void ScriptParticleEmitter::Internal_setEmissionBursts(ScriptParticleEmitter* thisPtr, MonoArray* bursts)
	{
		Vector<ParticleBurst> vecbursts;
		if(bursts != nullptr)
		{
			ScriptArray Arraybursts(bursts);
			vecbursts.Resize(arraybursts.size());
			for(int i = 0; i < (int)arraybursts.Size(); i++)
			{
				vecbursts[i] = ScriptParticleBurst::fromInterop(arraybursts.get<__ParticleBurstInterop>(i));
			}
		}
		thisPtr->GetInternal()->setEmissionBursts(vecbursts);
	}

	MonoArray* ScriptParticleEmitter::Internal_getEmissionBursts(ScriptParticleEmitter* thisPtr)
	{
		Vector<ParticleBurst> vec__output;
		vec__output = thisPtr->GetInternal()->getEmissionBursts();

		MonoArray* __output;
		int arraySize__output = (int)vec__output.Size();
		ScriptArray array__output = ScriptArray::create<ScriptParticleBurst>(arraySize__output);
		for(int i = 0; i < arraySize__output; i++)
		{
			array__output.Set(i, ScriptParticleBurst::toInterop(vec__output[i]));
		}
		__output = array__output.GetInternal();

		return __output;
	}

	void ScriptParticleEmitter::Internal_setInitialLifetime(ScriptParticleEmitter* thisPtr, MonoObject* value)
	{
		SPtr<TDistribution<float>> tmpvalue;
		ScriptTDistributionfloat* scriptvalue;
		scriptvalue = ScriptTDistributionfloat::toNative(value);
		if(scriptvalue != nullptr)
			tmpvalue = scriptvalue->GetInternal();
		thisPtr->GetInternal()->setInitialLifetime(*tmpvalue);
	}

	MonoObject* ScriptParticleEmitter::Internal_getInitialLifetime(ScriptParticleEmitter* thisPtr)
	{
		SPtr<TDistribution<float>> tmp__output = bs_shared_ptr_new<TDistribution<float>>();
		*tmp__output = thisPtr->GetInternal()->getInitialLifetime();

		MonoObject* __output;
		__output = ScriptTDistributionfloat::create(tmp__output);

		return __output;
	}

	void ScriptParticleEmitter::Internal_setInitialSpeed(ScriptParticleEmitter* thisPtr, MonoObject* value)
	{
		SPtr<TDistribution<float>> tmpvalue;
		ScriptTDistributionfloat* scriptvalue;
		scriptvalue = ScriptTDistributionfloat::toNative(value);
		if(scriptvalue != nullptr)
			tmpvalue = scriptvalue->GetInternal();
		thisPtr->GetInternal()->setInitialSpeed(*tmpvalue);
	}

	MonoObject* ScriptParticleEmitter::Internal_getInitialSpeed(ScriptParticleEmitter* thisPtr)
	{
		SPtr<TDistribution<float>> tmp__output = bs_shared_ptr_new<TDistribution<float>>();
		*tmp__output = thisPtr->GetInternal()->getInitialSpeed();

		MonoObject* __output;
		__output = ScriptTDistributionfloat::create(tmp__output);

		return __output;
	}

	void ScriptParticleEmitter::Internal_setInitialSize(ScriptParticleEmitter* thisPtr, MonoObject* value)
	{
		SPtr<TDistribution<float>> tmpvalue;
		ScriptTDistributionfloat* scriptvalue;
		scriptvalue = ScriptTDistributionfloat::toNative(value);
		if(scriptvalue != nullptr)
			tmpvalue = scriptvalue->GetInternal();
		thisPtr->GetInternal()->setInitialSize(*tmpvalue);
	}

	MonoObject* ScriptParticleEmitter::Internal_getInitialSize(ScriptParticleEmitter* thisPtr)
	{
		SPtr<TDistribution<float>> tmp__output = bs_shared_ptr_new<TDistribution<float>>();
		*tmp__output = thisPtr->GetInternal()->getInitialSize();

		MonoObject* __output;
		__output = ScriptTDistributionfloat::create(tmp__output);

		return __output;
	}

	void ScriptParticleEmitter::Internal_setInitialSize3D(ScriptParticleEmitter* thisPtr, MonoObject* value)
	{
		SPtr<TDistribution<Vector3>> tmpvalue;
		ScriptTDistributionVector3* scriptvalue;
		scriptvalue = ScriptTDistributionVector3::toNative(value);
		if(scriptvalue != nullptr)
			tmpvalue = scriptvalue->GetInternal();
		thisPtr->GetInternal()->setInitialSize3D(*tmpvalue);
	}

	MonoObject* ScriptParticleEmitter::Internal_getInitialSize3D(ScriptParticleEmitter* thisPtr)
	{
		SPtr<TDistribution<Vector3>> tmp__output = bs_shared_ptr_new<TDistribution<Vector3>>();
		*tmp__output = thisPtr->GetInternal()->getInitialSize3D();

		MonoObject* __output;
		__output = ScriptTDistributionVector3::create(tmp__output);

		return __output;
	}

	void ScriptParticleEmitter::Internal_setUse3DSize(ScriptParticleEmitter* thisPtr, bool value)
	{
		thisPtr->GetInternal()->setUse3DSize(value);
	}

	bool ScriptParticleEmitter::Internal_getUse3DSize(ScriptParticleEmitter* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetInternal()->getUse3DSize();

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptParticleEmitter::Internal_setInitialRotation(ScriptParticleEmitter* thisPtr, MonoObject* value)
	{
		SPtr<TDistribution<float>> tmpvalue;
		ScriptTDistributionfloat* scriptvalue;
		scriptvalue = ScriptTDistributionfloat::toNative(value);
		if(scriptvalue != nullptr)
			tmpvalue = scriptvalue->GetInternal();
		thisPtr->GetInternal()->setInitialRotation(*tmpvalue);
	}

	MonoObject* ScriptParticleEmitter::Internal_getInitialRotation(ScriptParticleEmitter* thisPtr)
	{
		SPtr<TDistribution<float>> tmp__output = bs_shared_ptr_new<TDistribution<float>>();
		*tmp__output = thisPtr->GetInternal()->getInitialRotation();

		MonoObject* __output;
		__output = ScriptTDistributionfloat::create(tmp__output);

		return __output;
	}

	void ScriptParticleEmitter::Internal_setInitialRotation3D(ScriptParticleEmitter* thisPtr, MonoObject* value)
	{
		SPtr<TDistribution<Vector3>> tmpvalue;
		ScriptTDistributionVector3* scriptvalue;
		scriptvalue = ScriptTDistributionVector3::toNative(value);
		if(scriptvalue != nullptr)
			tmpvalue = scriptvalue->GetInternal();
		thisPtr->GetInternal()->setInitialRotation3D(*tmpvalue);
	}

	MonoObject* ScriptParticleEmitter::Internal_getInitialRotation3D(ScriptParticleEmitter* thisPtr)
	{
		SPtr<TDistribution<Vector3>> tmp__output = bs_shared_ptr_new<TDistribution<Vector3>>();
		*tmp__output = thisPtr->GetInternal()->getInitialRotation3D();

		MonoObject* __output;
		__output = ScriptTDistributionVector3::create(tmp__output);

		return __output;
	}

	void ScriptParticleEmitter::Internal_setUse3DRotation(ScriptParticleEmitter* thisPtr, bool value)
	{
		thisPtr->GetInternal()->setUse3DRotation(value);
	}

	bool ScriptParticleEmitter::Internal_getUse3DRotation(ScriptParticleEmitter* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetInternal()->getUse3DRotation();

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptParticleEmitter::Internal_setInitialColor(ScriptParticleEmitter* thisPtr, MonoObject* value)
	{
		SPtr<TColorDistribution<ColorGradient>> tmpvalue;
		ScriptTColorDistributionColorGradient* scriptvalue;
		scriptvalue = ScriptTColorDistributionColorGradient::toNative(value);
		if(scriptvalue != nullptr)
			tmpvalue = scriptvalue->GetInternal();
		thisPtr->GetInternal()->setInitialColor(*tmpvalue);
	}

	MonoObject* ScriptParticleEmitter::Internal_getInitialColor(ScriptParticleEmitter* thisPtr)
	{
		SPtr<TColorDistribution<ColorGradient>> tmp__output = bs_shared_ptr_new<TColorDistribution<ColorGradient>>();
		*tmp__output = thisPtr->GetInternal()->getInitialColor();

		MonoObject* __output;
		__output = ScriptTColorDistributionColorGradient::create(tmp__output);

		return __output;
	}

	void ScriptParticleEmitter::Internal_setRandomOffset(ScriptParticleEmitter* thisPtr, float value)
	{
		thisPtr->GetInternal()->setRandomOffset(value);
	}

	float ScriptParticleEmitter::Internal_getRandomOffset(ScriptParticleEmitter* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetInternal()->getRandomOffset();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptParticleEmitter::Internal_setFlipU(ScriptParticleEmitter* thisPtr, float value)
	{
		thisPtr->GetInternal()->setFlipU(value);
	}

	float ScriptParticleEmitter::Internal_getFlipU(ScriptParticleEmitter* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetInternal()->getFlipU();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptParticleEmitter::Internal_setFlipV(ScriptParticleEmitter* thisPtr, float value)
	{
		thisPtr->GetInternal()->setFlipV(value);
	}

	float ScriptParticleEmitter::Internal_getFlipV(ScriptParticleEmitter* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetInternal()->getFlipV();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptParticleEmitter::Internal_create(MonoObject* managedInstance)
	{
		SPtr<ParticleEmitter> instance = ParticleEmitter::create();
		new (bs_alloc<ScriptParticleEmitter>())ScriptParticleEmitter(managedInstance, instance);
	}
}
