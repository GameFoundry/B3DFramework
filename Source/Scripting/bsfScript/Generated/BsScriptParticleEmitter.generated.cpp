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
	ScriptParticleEmitter::ScriptParticleEmitter(const SPtr<ParticleEmitter>& nativeObject, MonoObject* scriptObject)
		:TScriptReflectableWrapper(nativeObject, scriptObject)
	{
		RegisterEvents();
	}

	void ScriptParticleEmitter::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetShape", (void*)&ScriptParticleEmitter::InternalSetShape);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetShape", (void*)&ScriptParticleEmitter::InternalGetShape);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetEmissionRate", (void*)&ScriptParticleEmitter::InternalSetEmissionRate);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetEmissionRate", (void*)&ScriptParticleEmitter::InternalGetEmissionRate);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetEmissionBursts", (void*)&ScriptParticleEmitter::InternalSetEmissionBursts);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetEmissionBursts", (void*)&ScriptParticleEmitter::InternalGetEmissionBursts);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetInitialLifetime", (void*)&ScriptParticleEmitter::InternalSetInitialLifetime);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetInitialLifetime", (void*)&ScriptParticleEmitter::InternalGetInitialLifetime);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetInitialSpeed", (void*)&ScriptParticleEmitter::InternalSetInitialSpeed);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetInitialSpeed", (void*)&ScriptParticleEmitter::InternalGetInitialSpeed);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetInitialSize", (void*)&ScriptParticleEmitter::InternalSetInitialSize);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetInitialSize", (void*)&ScriptParticleEmitter::InternalGetInitialSize);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetInitialSize3D", (void*)&ScriptParticleEmitter::InternalSetInitialSize3D);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetInitialSize3D", (void*)&ScriptParticleEmitter::InternalGetInitialSize3D);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetUse3DSize", (void*)&ScriptParticleEmitter::InternalSetUse3DSize);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetUse3DSize", (void*)&ScriptParticleEmitter::InternalGetUse3DSize);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetInitialRotation", (void*)&ScriptParticleEmitter::InternalSetInitialRotation);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetInitialRotation", (void*)&ScriptParticleEmitter::InternalGetInitialRotation);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetInitialRotation3D", (void*)&ScriptParticleEmitter::InternalSetInitialRotation3D);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetInitialRotation3D", (void*)&ScriptParticleEmitter::InternalGetInitialRotation3D);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetUse3DRotation", (void*)&ScriptParticleEmitter::InternalSetUse3DRotation);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetUse3DRotation", (void*)&ScriptParticleEmitter::InternalGetUse3DRotation);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetInitialColor", (void*)&ScriptParticleEmitter::InternalSetInitialColor);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetInitialColor", (void*)&ScriptParticleEmitter::InternalGetInitialColor);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetRandomOffset", (void*)&ScriptParticleEmitter::InternalSetRandomOffset);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetRandomOffset", (void*)&ScriptParticleEmitter::InternalGetRandomOffset);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetFlipU", (void*)&ScriptParticleEmitter::InternalSetFlipU);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetFlipU", (void*)&ScriptParticleEmitter::InternalGetFlipU);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetFlipV", (void*)&ScriptParticleEmitter::InternalSetFlipV);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetFlipV", (void*)&ScriptParticleEmitter::InternalGetFlipV);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create", (void*)&ScriptParticleEmitter::InternalCreate);

	}

	MonoObject* ScriptParticleEmitter::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	void ScriptParticleEmitter::InternalSetShape(ScriptParticleEmitter* self, MonoObject* shape)
	{
		SPtr<ParticleEmitterShape> tmpshape;
		ScriptParticleEmitterShapeWrapperBase* scriptObjectWrappershape;
		scriptObjectWrappershape = (ScriptParticleEmitterShapeWrapperBase*)ScriptParticleEmitterShape::GetScriptObjectWrapper(shape);
		if(scriptObjectWrappershape != nullptr)
			tmpshape = std::static_pointer_cast<ParticleEmitterShape>(scriptObjectWrappershape->GetBaseNativeObjectAsShared());
		static_cast<ParticleEmitter*>(self->GetNativeObject())->SetShape(tmpshape);
	}

	MonoObject* ScriptParticleEmitter::InternalGetShape(ScriptParticleEmitter* self)
	{
		SPtr<ParticleEmitterShape> tmp__output;
		tmp__output = static_cast<ParticleEmitter*>(self->GetNativeObject())->GetShape();

		MonoObject* __output;
		if(tmp__output)
		{
			if(B3DRTTIIsOfType<ParticleEmitterStaticMeshShape>(tmp__output))
				__output = ScriptParticleEmitterStaticMeshShape::GetOrCreateScriptObject(std::static_pointer_cast<ParticleEmitterStaticMeshShape>(tmp__output));
			else if(B3DRTTIIsOfType<ParticleEmitterCircleShape>(tmp__output))
				__output = ScriptParticleEmitterCircleShape::GetOrCreateScriptObject(std::static_pointer_cast<ParticleEmitterCircleShape>(tmp__output));
			else if(B3DRTTIIsOfType<ParticleEmitterConeShape>(tmp__output))
				__output = ScriptParticleEmitterConeShape::GetOrCreateScriptObject(std::static_pointer_cast<ParticleEmitterConeShape>(tmp__output));
			else if(B3DRTTIIsOfType<ParticleEmitterSphereShape>(tmp__output))
				__output = ScriptParticleEmitterSphereShape::GetOrCreateScriptObject(std::static_pointer_cast<ParticleEmitterSphereShape>(tmp__output));
			else if(B3DRTTIIsOfType<ParticleEmitterHemisphereShape>(tmp__output))
				__output = ScriptParticleEmitterHemisphereShape::GetOrCreateScriptObject(std::static_pointer_cast<ParticleEmitterHemisphereShape>(tmp__output));
			else if(B3DRTTIIsOfType<ParticleEmitterBoxShape>(tmp__output))
				__output = ScriptParticleEmitterBoxShape::GetOrCreateScriptObject(std::static_pointer_cast<ParticleEmitterBoxShape>(tmp__output));
			else if(B3DRTTIIsOfType<ParticleEmitterLineShape>(tmp__output))
				__output = ScriptParticleEmitterLineShape::GetOrCreateScriptObject(std::static_pointer_cast<ParticleEmitterLineShape>(tmp__output));
			else if(B3DRTTIIsOfType<ParticleEmitterRectShape>(tmp__output))
				__output = ScriptParticleEmitterRectShape::GetOrCreateScriptObject(std::static_pointer_cast<ParticleEmitterRectShape>(tmp__output));
			else if(B3DRTTIIsOfType<ParticleEmitterSkinnedMeshShape>(tmp__output))
				__output = ScriptParticleEmitterSkinnedMeshShape::GetOrCreateScriptObject(std::static_pointer_cast<ParticleEmitterSkinnedMeshShape>(tmp__output));
			else
				__output = ScriptParticleEmitterShape::GetOrCreateScriptObject(tmp__output);
		}
		else
			__output = ScriptParticleEmitterShape::GetOrCreateScriptObject(tmp__output);

		return __output;
	}

	void ScriptParticleEmitter::InternalSetEmissionRate(ScriptParticleEmitter* self, MonoObject* value)
	{
		SPtr<TDistribution<float>> tmpvalue;
		ScriptFloatDistribution* scriptObjectWrappervalue;
		scriptObjectWrappervalue = ScriptFloatDistribution::ToNative(value);
		if(scriptObjectWrappervalue != nullptr)
			tmpvalue = scriptObjectWrappervalue->GetInternal();
		static_cast<ParticleEmitter*>(self->GetNativeObject())->SetEmissionRate(*tmpvalue);
	}

	MonoObject* ScriptParticleEmitter::InternalGetEmissionRate(ScriptParticleEmitter* self)
	{
		SPtr<TDistribution<float>> tmp__output = B3DMakeShared<TDistribution<float>>();
		*tmp__output = static_cast<ParticleEmitter*>(self->GetNativeObject())->GetEmissionRate();

		MonoObject* __output;
		__output = ScriptFloatDistribution::Create(tmp__output);

		return __output;
	}

	void ScriptParticleEmitter::InternalSetEmissionBursts(ScriptParticleEmitter* self, MonoArray* bursts)
	{
		Vector<ParticleBurst> nativeArraybursts;
		if(bursts != nullptr)
		{
			ScriptArray scriptArraybursts(bursts);
			nativeArraybursts.resize(scriptArraybursts.Size());
			for(int elementIndex = 0; elementIndex < (int)scriptArraybursts.Size(); elementIndex++)
			{
				nativeArraybursts[elementIndex] = ScriptParticleBurst::FromInterop(scriptArraybursts.Get<__ParticleBurstInterop>(elementIndex));
			}
		}
		static_cast<ParticleEmitter*>(self->GetNativeObject())->SetEmissionBursts(nativeArraybursts);
	}

	MonoArray* ScriptParticleEmitter::InternalGetEmissionBursts(ScriptParticleEmitter* self)
	{
		Vector<ParticleBurst> nativeArray__output;
		nativeArray__output = static_cast<ParticleEmitter*>(self->GetNativeObject())->GetEmissionBursts();

		MonoArray* __output;
		int elementCount__output = (int)nativeArray__output.size();
		ScriptArray scriptArray__output = ScriptArray::Create<ScriptParticleBurst>(elementCount__output);
		for(int elementIndex = 0; elementIndex < elementCount__output; elementIndex++)
		{
			scriptArray__output.Set(elementIndex, ScriptParticleBurst::ToInterop(nativeArray__output[elementIndex]));
		}
		__output = scriptArray__output.GetInternal();

		return __output;
	}

	void ScriptParticleEmitter::InternalSetInitialLifetime(ScriptParticleEmitter* self, MonoObject* value)
	{
		SPtr<TDistribution<float>> tmpvalue;
		ScriptFloatDistribution* scriptObjectWrappervalue;
		scriptObjectWrappervalue = ScriptFloatDistribution::ToNative(value);
		if(scriptObjectWrappervalue != nullptr)
			tmpvalue = scriptObjectWrappervalue->GetInternal();
		static_cast<ParticleEmitter*>(self->GetNativeObject())->SetInitialLifetime(*tmpvalue);
	}

	MonoObject* ScriptParticleEmitter::InternalGetInitialLifetime(ScriptParticleEmitter* self)
	{
		SPtr<TDistribution<float>> tmp__output = B3DMakeShared<TDistribution<float>>();
		*tmp__output = static_cast<ParticleEmitter*>(self->GetNativeObject())->GetInitialLifetime();

		MonoObject* __output;
		__output = ScriptFloatDistribution::Create(tmp__output);

		return __output;
	}

	void ScriptParticleEmitter::InternalSetInitialSpeed(ScriptParticleEmitter* self, MonoObject* value)
	{
		SPtr<TDistribution<float>> tmpvalue;
		ScriptFloatDistribution* scriptObjectWrappervalue;
		scriptObjectWrappervalue = ScriptFloatDistribution::ToNative(value);
		if(scriptObjectWrappervalue != nullptr)
			tmpvalue = scriptObjectWrappervalue->GetInternal();
		static_cast<ParticleEmitter*>(self->GetNativeObject())->SetInitialSpeed(*tmpvalue);
	}

	MonoObject* ScriptParticleEmitter::InternalGetInitialSpeed(ScriptParticleEmitter* self)
	{
		SPtr<TDistribution<float>> tmp__output = B3DMakeShared<TDistribution<float>>();
		*tmp__output = static_cast<ParticleEmitter*>(self->GetNativeObject())->GetInitialSpeed();

		MonoObject* __output;
		__output = ScriptFloatDistribution::Create(tmp__output);

		return __output;
	}

	void ScriptParticleEmitter::InternalSetInitialSize(ScriptParticleEmitter* self, MonoObject* value)
	{
		SPtr<TDistribution<float>> tmpvalue;
		ScriptFloatDistribution* scriptObjectWrappervalue;
		scriptObjectWrappervalue = ScriptFloatDistribution::ToNative(value);
		if(scriptObjectWrappervalue != nullptr)
			tmpvalue = scriptObjectWrappervalue->GetInternal();
		static_cast<ParticleEmitter*>(self->GetNativeObject())->SetInitialSize(*tmpvalue);
	}

	MonoObject* ScriptParticleEmitter::InternalGetInitialSize(ScriptParticleEmitter* self)
	{
		SPtr<TDistribution<float>> tmp__output = B3DMakeShared<TDistribution<float>>();
		*tmp__output = static_cast<ParticleEmitter*>(self->GetNativeObject())->GetInitialSize();

		MonoObject* __output;
		__output = ScriptFloatDistribution::Create(tmp__output);

		return __output;
	}

	void ScriptParticleEmitter::InternalSetInitialSize3D(ScriptParticleEmitter* self, MonoObject* value)
	{
		SPtr<TDistribution<TVector3<float>>> tmpvalue;
		ScriptVector3Distribution* scriptObjectWrappervalue;
		scriptObjectWrappervalue = ScriptVector3Distribution::ToNative(value);
		if(scriptObjectWrappervalue != nullptr)
			tmpvalue = scriptObjectWrappervalue->GetInternal();
		static_cast<ParticleEmitter*>(self->GetNativeObject())->SetInitialSize3D(*tmpvalue);
	}

	MonoObject* ScriptParticleEmitter::InternalGetInitialSize3D(ScriptParticleEmitter* self)
	{
		SPtr<TDistribution<TVector3<float>>> tmp__output = B3DMakeShared<TDistribution<TVector3<float>>>();
		*tmp__output = static_cast<ParticleEmitter*>(self->GetNativeObject())->GetInitialSize3D();

		MonoObject* __output;
		__output = ScriptVector3Distribution::Create(tmp__output);

		return __output;
	}

	void ScriptParticleEmitter::InternalSetUse3DSize(ScriptParticleEmitter* self, bool value)
	{
		static_cast<ParticleEmitter*>(self->GetNativeObject())->SetUse3DSize(value);
	}

	bool ScriptParticleEmitter::InternalGetUse3DSize(ScriptParticleEmitter* self)
	{
		bool tmp__output;
		tmp__output = static_cast<ParticleEmitter*>(self->GetNativeObject())->GetUse3DSize();

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptParticleEmitter::InternalSetInitialRotation(ScriptParticleEmitter* self, MonoObject* value)
	{
		SPtr<TDistribution<float>> tmpvalue;
		ScriptFloatDistribution* scriptObjectWrappervalue;
		scriptObjectWrappervalue = ScriptFloatDistribution::ToNative(value);
		if(scriptObjectWrappervalue != nullptr)
			tmpvalue = scriptObjectWrappervalue->GetInternal();
		static_cast<ParticleEmitter*>(self->GetNativeObject())->SetInitialRotation(*tmpvalue);
	}

	MonoObject* ScriptParticleEmitter::InternalGetInitialRotation(ScriptParticleEmitter* self)
	{
		SPtr<TDistribution<float>> tmp__output = B3DMakeShared<TDistribution<float>>();
		*tmp__output = static_cast<ParticleEmitter*>(self->GetNativeObject())->GetInitialRotation();

		MonoObject* __output;
		__output = ScriptFloatDistribution::Create(tmp__output);

		return __output;
	}

	void ScriptParticleEmitter::InternalSetInitialRotation3D(ScriptParticleEmitter* self, MonoObject* value)
	{
		SPtr<TDistribution<TVector3<float>>> tmpvalue;
		ScriptVector3Distribution* scriptObjectWrappervalue;
		scriptObjectWrappervalue = ScriptVector3Distribution::ToNative(value);
		if(scriptObjectWrappervalue != nullptr)
			tmpvalue = scriptObjectWrappervalue->GetInternal();
		static_cast<ParticleEmitter*>(self->GetNativeObject())->SetInitialRotation3D(*tmpvalue);
	}

	MonoObject* ScriptParticleEmitter::InternalGetInitialRotation3D(ScriptParticleEmitter* self)
	{
		SPtr<TDistribution<TVector3<float>>> tmp__output = B3DMakeShared<TDistribution<TVector3<float>>>();
		*tmp__output = static_cast<ParticleEmitter*>(self->GetNativeObject())->GetInitialRotation3D();

		MonoObject* __output;
		__output = ScriptVector3Distribution::Create(tmp__output);

		return __output;
	}

	void ScriptParticleEmitter::InternalSetUse3DRotation(ScriptParticleEmitter* self, bool value)
	{
		static_cast<ParticleEmitter*>(self->GetNativeObject())->SetUse3DRotation(value);
	}

	bool ScriptParticleEmitter::InternalGetUse3DRotation(ScriptParticleEmitter* self)
	{
		bool tmp__output;
		tmp__output = static_cast<ParticleEmitter*>(self->GetNativeObject())->GetUse3DRotation();

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptParticleEmitter::InternalSetInitialColor(ScriptParticleEmitter* self, MonoObject* value)
	{
		SPtr<TColorDistribution<ColorGradient>> tmpvalue;
		ScriptColorDistribution* scriptObjectWrappervalue;
		scriptObjectWrappervalue = ScriptColorDistribution::ToNative(value);
		if(scriptObjectWrappervalue != nullptr)
			tmpvalue = scriptObjectWrappervalue->GetInternal();
		static_cast<ParticleEmitter*>(self->GetNativeObject())->SetInitialColor(*tmpvalue);
	}

	MonoObject* ScriptParticleEmitter::InternalGetInitialColor(ScriptParticleEmitter* self)
	{
		SPtr<TColorDistribution<ColorGradient>> tmp__output = B3DMakeShared<TColorDistribution<ColorGradient>>();
		*tmp__output = static_cast<ParticleEmitter*>(self->GetNativeObject())->GetInitialColor();

		MonoObject* __output;
		__output = ScriptColorDistribution::Create(tmp__output);

		return __output;
	}

	void ScriptParticleEmitter::InternalSetRandomOffset(ScriptParticleEmitter* self, float value)
	{
		static_cast<ParticleEmitter*>(self->GetNativeObject())->SetRandomOffset(value);
	}

	float ScriptParticleEmitter::InternalGetRandomOffset(ScriptParticleEmitter* self)
	{
		float tmp__output;
		tmp__output = static_cast<ParticleEmitter*>(self->GetNativeObject())->GetRandomOffset();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptParticleEmitter::InternalSetFlipU(ScriptParticleEmitter* self, float value)
	{
		static_cast<ParticleEmitter*>(self->GetNativeObject())->SetFlipU(value);
	}

	float ScriptParticleEmitter::InternalGetFlipU(ScriptParticleEmitter* self)
	{
		float tmp__output;
		tmp__output = static_cast<ParticleEmitter*>(self->GetNativeObject())->GetFlipU();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptParticleEmitter::InternalSetFlipV(ScriptParticleEmitter* self, float value)
	{
		static_cast<ParticleEmitter*>(self->GetNativeObject())->SetFlipV(value);
	}

	float ScriptParticleEmitter::InternalGetFlipV(ScriptParticleEmitter* self)
	{
		float tmp__output;
		tmp__output = static_cast<ParticleEmitter*>(self->GetNativeObject())->GetFlipV();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptParticleEmitter::InternalCreate(MonoObject* scriptObject)
	{
		SPtr<ParticleEmitter> nativeObject = ParticleEmitter::Create();
		B3DNew<ScriptParticleEmitter>(nativeObject, scriptObject);
	}
}
