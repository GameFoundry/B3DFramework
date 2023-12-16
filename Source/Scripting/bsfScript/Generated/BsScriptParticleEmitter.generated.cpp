//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
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
		metaData.ScriptClass->AddInternalCall("Internal_SetShape", (void*)&ScriptParticleEmitter::InternalSetShape);
		metaData.ScriptClass->AddInternalCall("Internal_GetShape", (void*)&ScriptParticleEmitter::InternalGetShape);
		metaData.ScriptClass->AddInternalCall("Internal_SetEmissionRate", (void*)&ScriptParticleEmitter::InternalSetEmissionRate);
		metaData.ScriptClass->AddInternalCall("Internal_GetEmissionRate", (void*)&ScriptParticleEmitter::InternalGetEmissionRate);
		metaData.ScriptClass->AddInternalCall("Internal_SetEmissionBursts", (void*)&ScriptParticleEmitter::InternalSetEmissionBursts);
		metaData.ScriptClass->AddInternalCall("Internal_GetEmissionBursts", (void*)&ScriptParticleEmitter::InternalGetEmissionBursts);
		metaData.ScriptClass->AddInternalCall("Internal_SetInitialLifetime", (void*)&ScriptParticleEmitter::InternalSetInitialLifetime);
		metaData.ScriptClass->AddInternalCall("Internal_GetInitialLifetime", (void*)&ScriptParticleEmitter::InternalGetInitialLifetime);
		metaData.ScriptClass->AddInternalCall("Internal_SetInitialSpeed", (void*)&ScriptParticleEmitter::InternalSetInitialSpeed);
		metaData.ScriptClass->AddInternalCall("Internal_GetInitialSpeed", (void*)&ScriptParticleEmitter::InternalGetInitialSpeed);
		metaData.ScriptClass->AddInternalCall("Internal_SetInitialSize", (void*)&ScriptParticleEmitter::InternalSetInitialSize);
		metaData.ScriptClass->AddInternalCall("Internal_GetInitialSize", (void*)&ScriptParticleEmitter::InternalGetInitialSize);
		metaData.ScriptClass->AddInternalCall("Internal_SetInitialSize3D", (void*)&ScriptParticleEmitter::InternalSetInitialSize3D);
		metaData.ScriptClass->AddInternalCall("Internal_GetInitialSize3D", (void*)&ScriptParticleEmitter::InternalGetInitialSize3D);
		metaData.ScriptClass->AddInternalCall("Internal_SetUse3DSize", (void*)&ScriptParticleEmitter::InternalSetUse3DSize);
		metaData.ScriptClass->AddInternalCall("Internal_GetUse3DSize", (void*)&ScriptParticleEmitter::InternalGetUse3DSize);
		metaData.ScriptClass->AddInternalCall("Internal_SetInitialRotation", (void*)&ScriptParticleEmitter::InternalSetInitialRotation);
		metaData.ScriptClass->AddInternalCall("Internal_GetInitialRotation", (void*)&ScriptParticleEmitter::InternalGetInitialRotation);
		metaData.ScriptClass->AddInternalCall("Internal_SetInitialRotation3D", (void*)&ScriptParticleEmitter::InternalSetInitialRotation3D);
		metaData.ScriptClass->AddInternalCall("Internal_GetInitialRotation3D", (void*)&ScriptParticleEmitter::InternalGetInitialRotation3D);
		metaData.ScriptClass->AddInternalCall("Internal_SetUse3DRotation", (void*)&ScriptParticleEmitter::InternalSetUse3DRotation);
		metaData.ScriptClass->AddInternalCall("Internal_GetUse3DRotation", (void*)&ScriptParticleEmitter::InternalGetUse3DRotation);
		metaData.ScriptClass->AddInternalCall("Internal_SetInitialColor", (void*)&ScriptParticleEmitter::InternalSetInitialColor);
		metaData.ScriptClass->AddInternalCall("Internal_GetInitialColor", (void*)&ScriptParticleEmitter::InternalGetInitialColor);
		metaData.ScriptClass->AddInternalCall("Internal_SetRandomOffset", (void*)&ScriptParticleEmitter::InternalSetRandomOffset);
		metaData.ScriptClass->AddInternalCall("Internal_GetRandomOffset", (void*)&ScriptParticleEmitter::InternalGetRandomOffset);
		metaData.ScriptClass->AddInternalCall("Internal_SetFlipU", (void*)&ScriptParticleEmitter::InternalSetFlipU);
		metaData.ScriptClass->AddInternalCall("Internal_GetFlipU", (void*)&ScriptParticleEmitter::InternalGetFlipU);
		metaData.ScriptClass->AddInternalCall("Internal_SetFlipV", (void*)&ScriptParticleEmitter::InternalSetFlipV);
		metaData.ScriptClass->AddInternalCall("Internal_GetFlipV", (void*)&ScriptParticleEmitter::InternalGetFlipV);
		metaData.ScriptClass->AddInternalCall("Internal_Create", (void*)&ScriptParticleEmitter::InternalCreate);

	}

	MonoObject* ScriptParticleEmitter::Create(const SPtr<ParticleEmitter>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.ScriptClass->CreateInstance("bool", ctorParams);
		new (B3DAllocate<ScriptParticleEmitter>()) ScriptParticleEmitter(managedInstance, value);
		return managedInstance;
	}
	void ScriptParticleEmitter::InternalSetShape(ScriptParticleEmitter* thisPtr, MonoObject* shape)
	{
		SPtr<ParticleEmitterShape> tmpshape;
		ScriptParticleEmitterShapeBase* scriptshape;
		scriptshape = (ScriptParticleEmitterShapeBase*)ScriptParticleEmitterShape::ToNative(shape);
		if(scriptshape != nullptr)
			tmpshape = scriptshape->GetInternal();
		thisPtr->GetInternal()->SetShape(tmpshape);
	}

	MonoObject* ScriptParticleEmitter::InternalGetShape(ScriptParticleEmitter* thisPtr)
	{
		SPtr<ParticleEmitterShape> tmp__output;
		tmp__output = thisPtr->GetInternal()->GetShape();

		MonoObject* __output;
		if(tmp__output)
		{
			if(B3DRTTIIsOfType<ParticleEmitterStaticMeshShape>(tmp__output))
				__output = ScriptParticleEmitterStaticMeshShape::Create(std::static_pointer_cast<ParticleEmitterStaticMeshShape>(tmp__output));
			else if(B3DRTTIIsOfType<ParticleEmitterSkinnedMeshShape>(tmp__output))
				__output = ScriptParticleEmitterSkinnedMeshShape::Create(std::static_pointer_cast<ParticleEmitterSkinnedMeshShape>(tmp__output));
			else if(B3DRTTIIsOfType<ParticleEmitterCircleShape>(tmp__output))
				__output = ScriptParticleEmitterCircleShape::Create(std::static_pointer_cast<ParticleEmitterCircleShape>(tmp__output));
			else if(B3DRTTIIsOfType<ParticleEmitterConeShape>(tmp__output))
				__output = ScriptParticleEmitterConeShape::Create(std::static_pointer_cast<ParticleEmitterConeShape>(tmp__output));
			else if(B3DRTTIIsOfType<ParticleEmitterSphereShape>(tmp__output))
				__output = ScriptParticleEmitterSphereShape::Create(std::static_pointer_cast<ParticleEmitterSphereShape>(tmp__output));
			else if(B3DRTTIIsOfType<ParticleEmitterHemisphereShape>(tmp__output))
				__output = ScriptParticleEmitterHemisphereShape::Create(std::static_pointer_cast<ParticleEmitterHemisphereShape>(tmp__output));
			else if(B3DRTTIIsOfType<ParticleEmitterBoxShape>(tmp__output))
				__output = ScriptParticleEmitterBoxShape::Create(std::static_pointer_cast<ParticleEmitterBoxShape>(tmp__output));
			else if(B3DRTTIIsOfType<ParticleEmitterLineShape>(tmp__output))
				__output = ScriptParticleEmitterLineShape::Create(std::static_pointer_cast<ParticleEmitterLineShape>(tmp__output));
			else if(B3DRTTIIsOfType<ParticleEmitterRectShape>(tmp__output))
				__output = ScriptParticleEmitterRectShape::Create(std::static_pointer_cast<ParticleEmitterRectShape>(tmp__output));
			else
				__output = ScriptParticleEmitterShape::Create(tmp__output);
		}
		else
			__output = ScriptParticleEmitterShape::Create(tmp__output);

		return __output;
	}

	void ScriptParticleEmitter::InternalSetEmissionRate(ScriptParticleEmitter* thisPtr, MonoObject* value)
	{
		SPtr<TDistribution<float>> tmpvalue;
		ScriptFloatDistribution* scriptvalue;
		scriptvalue = ScriptFloatDistribution::ToNative(value);
		if(scriptvalue != nullptr)
			tmpvalue = scriptvalue->GetInternal();
		thisPtr->GetInternal()->SetEmissionRate(*tmpvalue);
	}

	MonoObject* ScriptParticleEmitter::InternalGetEmissionRate(ScriptParticleEmitter* thisPtr)
	{
		SPtr<TDistribution<float>> tmp__output = B3DMakeShared<TDistribution<float>>();
		*tmp__output = thisPtr->GetInternal()->GetEmissionRate();

		MonoObject* __output;
		__output = ScriptFloatDistribution::Create(tmp__output);

		return __output;
	}

	void ScriptParticleEmitter::InternalSetEmissionBursts(ScriptParticleEmitter* thisPtr, MonoArray* bursts)
	{
		Vector<ParticleBurst> vecbursts;
		if(bursts != nullptr)
		{
			ScriptArray arraybursts(bursts);
			vecbursts.resize(arraybursts.Size());
			for(int i = 0; i < (int)arraybursts.Size(); i++)
			{
				vecbursts[i] = ScriptParticleBurst::FromInterop(arraybursts.Get<__ParticleBurstInterop>(i));
			}
		}
		thisPtr->GetInternal()->SetEmissionBursts(vecbursts);
	}

	MonoArray* ScriptParticleEmitter::InternalGetEmissionBursts(ScriptParticleEmitter* thisPtr)
	{
		Vector<ParticleBurst> vec__output;
		vec__output = thisPtr->GetInternal()->GetEmissionBursts();

		MonoArray* __output;
		int arraySize__output = (int)vec__output.size();
		ScriptArray array__output = ScriptArray::Create<ScriptParticleBurst>(arraySize__output);
		for(int i = 0; i < arraySize__output; i++)
		{
			array__output.Set(i, ScriptParticleBurst::ToInterop(vec__output[i]));
		}
		__output = array__output.GetInternal();

		return __output;
	}

	void ScriptParticleEmitter::InternalSetInitialLifetime(ScriptParticleEmitter* thisPtr, MonoObject* value)
	{
		SPtr<TDistribution<float>> tmpvalue;
		ScriptFloatDistribution* scriptvalue;
		scriptvalue = ScriptFloatDistribution::ToNative(value);
		if(scriptvalue != nullptr)
			tmpvalue = scriptvalue->GetInternal();
		thisPtr->GetInternal()->SetInitialLifetime(*tmpvalue);
	}

	MonoObject* ScriptParticleEmitter::InternalGetInitialLifetime(ScriptParticleEmitter* thisPtr)
	{
		SPtr<TDistribution<float>> tmp__output = B3DMakeShared<TDistribution<float>>();
		*tmp__output = thisPtr->GetInternal()->GetInitialLifetime();

		MonoObject* __output;
		__output = ScriptFloatDistribution::Create(tmp__output);

		return __output;
	}

	void ScriptParticleEmitter::InternalSetInitialSpeed(ScriptParticleEmitter* thisPtr, MonoObject* value)
	{
		SPtr<TDistribution<float>> tmpvalue;
		ScriptFloatDistribution* scriptvalue;
		scriptvalue = ScriptFloatDistribution::ToNative(value);
		if(scriptvalue != nullptr)
			tmpvalue = scriptvalue->GetInternal();
		thisPtr->GetInternal()->SetInitialSpeed(*tmpvalue);
	}

	MonoObject* ScriptParticleEmitter::InternalGetInitialSpeed(ScriptParticleEmitter* thisPtr)
	{
		SPtr<TDistribution<float>> tmp__output = B3DMakeShared<TDistribution<float>>();
		*tmp__output = thisPtr->GetInternal()->GetInitialSpeed();

		MonoObject* __output;
		__output = ScriptFloatDistribution::Create(tmp__output);

		return __output;
	}

	void ScriptParticleEmitter::InternalSetInitialSize(ScriptParticleEmitter* thisPtr, MonoObject* value)
	{
		SPtr<TDistribution<float>> tmpvalue;
		ScriptFloatDistribution* scriptvalue;
		scriptvalue = ScriptFloatDistribution::ToNative(value);
		if(scriptvalue != nullptr)
			tmpvalue = scriptvalue->GetInternal();
		thisPtr->GetInternal()->SetInitialSize(*tmpvalue);
	}

	MonoObject* ScriptParticleEmitter::InternalGetInitialSize(ScriptParticleEmitter* thisPtr)
	{
		SPtr<TDistribution<float>> tmp__output = B3DMakeShared<TDistribution<float>>();
		*tmp__output = thisPtr->GetInternal()->GetInitialSize();

		MonoObject* __output;
		__output = ScriptFloatDistribution::Create(tmp__output);

		return __output;
	}

	void ScriptParticleEmitter::InternalSetInitialSize3D(ScriptParticleEmitter* thisPtr, MonoObject* value)
	{
		SPtr<TDistribution<TVector3<float>>> tmpvalue;
		ScriptVector3Distribution* scriptvalue;
		scriptvalue = ScriptVector3Distribution::ToNative(value);
		if(scriptvalue != nullptr)
			tmpvalue = scriptvalue->GetInternal();
		thisPtr->GetInternal()->SetInitialSize3D(*tmpvalue);
	}

	MonoObject* ScriptParticleEmitter::InternalGetInitialSize3D(ScriptParticleEmitter* thisPtr)
	{
		SPtr<TDistribution<TVector3<float>>> tmp__output = B3DMakeShared<TDistribution<TVector3<float>>>();
		*tmp__output = thisPtr->GetInternal()->GetInitialSize3D();

		MonoObject* __output;
		__output = ScriptVector3Distribution::Create(tmp__output);

		return __output;
	}

	void ScriptParticleEmitter::InternalSetUse3DSize(ScriptParticleEmitter* thisPtr, bool value)
	{
		thisPtr->GetInternal()->SetUse3DSize(value);
	}

	bool ScriptParticleEmitter::InternalGetUse3DSize(ScriptParticleEmitter* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetInternal()->GetUse3DSize();

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptParticleEmitter::InternalSetInitialRotation(ScriptParticleEmitter* thisPtr, MonoObject* value)
	{
		SPtr<TDistribution<float>> tmpvalue;
		ScriptFloatDistribution* scriptvalue;
		scriptvalue = ScriptFloatDistribution::ToNative(value);
		if(scriptvalue != nullptr)
			tmpvalue = scriptvalue->GetInternal();
		thisPtr->GetInternal()->SetInitialRotation(*tmpvalue);
	}

	MonoObject* ScriptParticleEmitter::InternalGetInitialRotation(ScriptParticleEmitter* thisPtr)
	{
		SPtr<TDistribution<float>> tmp__output = B3DMakeShared<TDistribution<float>>();
		*tmp__output = thisPtr->GetInternal()->GetInitialRotation();

		MonoObject* __output;
		__output = ScriptFloatDistribution::Create(tmp__output);

		return __output;
	}

	void ScriptParticleEmitter::InternalSetInitialRotation3D(ScriptParticleEmitter* thisPtr, MonoObject* value)
	{
		SPtr<TDistribution<TVector3<float>>> tmpvalue;
		ScriptVector3Distribution* scriptvalue;
		scriptvalue = ScriptVector3Distribution::ToNative(value);
		if(scriptvalue != nullptr)
			tmpvalue = scriptvalue->GetInternal();
		thisPtr->GetInternal()->SetInitialRotation3D(*tmpvalue);
	}

	MonoObject* ScriptParticleEmitter::InternalGetInitialRotation3D(ScriptParticleEmitter* thisPtr)
	{
		SPtr<TDistribution<TVector3<float>>> tmp__output = B3DMakeShared<TDistribution<TVector3<float>>>();
		*tmp__output = thisPtr->GetInternal()->GetInitialRotation3D();

		MonoObject* __output;
		__output = ScriptVector3Distribution::Create(tmp__output);

		return __output;
	}

	void ScriptParticleEmitter::InternalSetUse3DRotation(ScriptParticleEmitter* thisPtr, bool value)
	{
		thisPtr->GetInternal()->SetUse3DRotation(value);
	}

	bool ScriptParticleEmitter::InternalGetUse3DRotation(ScriptParticleEmitter* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetInternal()->GetUse3DRotation();

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptParticleEmitter::InternalSetInitialColor(ScriptParticleEmitter* thisPtr, MonoObject* value)
	{
		SPtr<TColorDistribution<ColorGradient>> tmpvalue;
		ScriptColorDistribution* scriptvalue;
		scriptvalue = ScriptColorDistribution::ToNative(value);
		if(scriptvalue != nullptr)
			tmpvalue = scriptvalue->GetInternal();
		thisPtr->GetInternal()->SetInitialColor(*tmpvalue);
	}

	MonoObject* ScriptParticleEmitter::InternalGetInitialColor(ScriptParticleEmitter* thisPtr)
	{
		SPtr<TColorDistribution<ColorGradient>> tmp__output = B3DMakeShared<TColorDistribution<ColorGradient>>();
		*tmp__output = thisPtr->GetInternal()->GetInitialColor();

		MonoObject* __output;
		__output = ScriptColorDistribution::Create(tmp__output);

		return __output;
	}

	void ScriptParticleEmitter::InternalSetRandomOffset(ScriptParticleEmitter* thisPtr, float value)
	{
		thisPtr->GetInternal()->SetRandomOffset(value);
	}

	float ScriptParticleEmitter::InternalGetRandomOffset(ScriptParticleEmitter* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetInternal()->GetRandomOffset();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptParticleEmitter::InternalSetFlipU(ScriptParticleEmitter* thisPtr, float value)
	{
		thisPtr->GetInternal()->SetFlipU(value);
	}

	float ScriptParticleEmitter::InternalGetFlipU(ScriptParticleEmitter* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetInternal()->GetFlipU();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptParticleEmitter::InternalSetFlipV(ScriptParticleEmitter* thisPtr, float value)
	{
		thisPtr->GetInternal()->SetFlipV(value);
	}

	float ScriptParticleEmitter::InternalGetFlipV(ScriptParticleEmitter* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetInternal()->GetFlipV();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptParticleEmitter::InternalCreate(MonoObject* managedInstance)
	{
		SPtr<ParticleEmitter> instance = ParticleEmitter::Create();
		new (B3DAllocate<ScriptParticleEmitter>())ScriptParticleEmitter(managedInstance, instance);
	}
}
