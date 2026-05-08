//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptParticleEmitter.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "Reflection/B3DRTTIType.h"
#include "B3DScriptParticleEmitterShape.generated.h"
#include "B3DScriptTColorDistribution.generated.h"
#include "B3DScriptParticleBurst.generated.h"
#include "B3DScriptTDistribution.generated.h"
#include "B3DScriptTDistribution.generated.h"
#include "B3DScriptParticleEmitter.generated.h"

namespace b3d
{
	ScriptParticleEmitter::ScriptParticleEmitter(const TShared<ParticleEmitter>& nativeObject)
		:TScriptReflectableWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptParticleEmitter::~ScriptParticleEmitter()
	{
		UnregisterEvents();
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
		if(!self->IsNativeObjectValid())
			return;

		TShared<ParticleEmitterShape> tmpshape;
		ScriptParticleEmitterShapeWrapperBase* scriptObjectWrappershape;
		scriptObjectWrappershape = (ScriptParticleEmitterShapeWrapperBase*)ScriptParticleEmitterShape::GetScriptObjectWrapper(shape);
		if(scriptObjectWrappershape != nullptr)
			tmpshape = std::static_pointer_cast<ParticleEmitterShape>(scriptObjectWrappershape->GetBaseNativeObjectAsShared());
		static_cast<ParticleEmitter*>(self->GetNativeObject())->SetShape(tmpshape);
	}

	MonoObject* ScriptParticleEmitter::InternalGetShape(ScriptParticleEmitter* self)
	{
		TShared<ParticleEmitterShape> tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<ParticleEmitter*>(self->GetNativeObject())->GetShape();

		MonoObject* __output;
		__output = ScriptParticleEmitterShape::GetOrCreateScriptObject(tmp__output);

		return __output;
	}

	void ScriptParticleEmitter::InternalSetEmissionRate(ScriptParticleEmitter* self, MonoObject* value)
	{
		if(!self->IsNativeObjectValid())
			return;

		TShared<TDistribution<float>> tmpvalue;
		ScriptFloatDistribution* scriptObjectWrappervalue;
		scriptObjectWrappervalue = ScriptFloatDistribution::GetScriptObjectWrapper(value);
		if(scriptObjectWrappervalue != nullptr)
			tmpvalue = std::static_pointer_cast<TDistribution<float>>(scriptObjectWrappervalue->GetBaseNativeObjectAsShared());
		static_cast<ParticleEmitter*>(self->GetNativeObject())->SetEmissionRate(*tmpvalue);
	}

	MonoObject* ScriptParticleEmitter::InternalGetEmissionRate(ScriptParticleEmitter* self)
	{
		TShared<TDistribution<float>> tmp__output = B3DMakeShared<TDistribution<float>>();
		if(!self->IsNativeObjectValid())
			return {};

		*tmp__output = static_cast<ParticleEmitter*>(self->GetNativeObject())->GetEmissionRate();

		MonoObject* __output;
		__output = ScriptFloatDistribution::GetOrCreateScriptObject(tmp__output);

		return __output;
	}

	void ScriptParticleEmitter::InternalSetEmissionBursts(ScriptParticleEmitter* self, MonoArray* bursts)
	{
		if(!self->IsNativeObjectValid())
			return;

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
		if(!self->IsNativeObjectValid())
			return {};

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
		if(!self->IsNativeObjectValid())
			return;

		TShared<TDistribution<float>> tmpvalue;
		ScriptFloatDistribution* scriptObjectWrappervalue;
		scriptObjectWrappervalue = ScriptFloatDistribution::GetScriptObjectWrapper(value);
		if(scriptObjectWrappervalue != nullptr)
			tmpvalue = std::static_pointer_cast<TDistribution<float>>(scriptObjectWrappervalue->GetBaseNativeObjectAsShared());
		static_cast<ParticleEmitter*>(self->GetNativeObject())->SetInitialLifetime(*tmpvalue);
	}

	MonoObject* ScriptParticleEmitter::InternalGetInitialLifetime(ScriptParticleEmitter* self)
	{
		TShared<TDistribution<float>> tmp__output = B3DMakeShared<TDistribution<float>>();
		if(!self->IsNativeObjectValid())
			return {};

		*tmp__output = static_cast<ParticleEmitter*>(self->GetNativeObject())->GetInitialLifetime();

		MonoObject* __output;
		__output = ScriptFloatDistribution::GetOrCreateScriptObject(tmp__output);

		return __output;
	}

	void ScriptParticleEmitter::InternalSetInitialSpeed(ScriptParticleEmitter* self, MonoObject* value)
	{
		if(!self->IsNativeObjectValid())
			return;

		TShared<TDistribution<float>> tmpvalue;
		ScriptFloatDistribution* scriptObjectWrappervalue;
		scriptObjectWrappervalue = ScriptFloatDistribution::GetScriptObjectWrapper(value);
		if(scriptObjectWrappervalue != nullptr)
			tmpvalue = std::static_pointer_cast<TDistribution<float>>(scriptObjectWrappervalue->GetBaseNativeObjectAsShared());
		static_cast<ParticleEmitter*>(self->GetNativeObject())->SetInitialSpeed(*tmpvalue);
	}

	MonoObject* ScriptParticleEmitter::InternalGetInitialSpeed(ScriptParticleEmitter* self)
	{
		TShared<TDistribution<float>> tmp__output = B3DMakeShared<TDistribution<float>>();
		if(!self->IsNativeObjectValid())
			return {};

		*tmp__output = static_cast<ParticleEmitter*>(self->GetNativeObject())->GetInitialSpeed();

		MonoObject* __output;
		__output = ScriptFloatDistribution::GetOrCreateScriptObject(tmp__output);

		return __output;
	}

	void ScriptParticleEmitter::InternalSetInitialSize(ScriptParticleEmitter* self, MonoObject* value)
	{
		if(!self->IsNativeObjectValid())
			return;

		TShared<TDistribution<float>> tmpvalue;
		ScriptFloatDistribution* scriptObjectWrappervalue;
		scriptObjectWrappervalue = ScriptFloatDistribution::GetScriptObjectWrapper(value);
		if(scriptObjectWrappervalue != nullptr)
			tmpvalue = std::static_pointer_cast<TDistribution<float>>(scriptObjectWrappervalue->GetBaseNativeObjectAsShared());
		static_cast<ParticleEmitter*>(self->GetNativeObject())->SetInitialSize(*tmpvalue);
	}

	MonoObject* ScriptParticleEmitter::InternalGetInitialSize(ScriptParticleEmitter* self)
	{
		TShared<TDistribution<float>> tmp__output = B3DMakeShared<TDistribution<float>>();
		if(!self->IsNativeObjectValid())
			return {};

		*tmp__output = static_cast<ParticleEmitter*>(self->GetNativeObject())->GetInitialSize();

		MonoObject* __output;
		__output = ScriptFloatDistribution::GetOrCreateScriptObject(tmp__output);

		return __output;
	}

	void ScriptParticleEmitter::InternalSetInitialSize3D(ScriptParticleEmitter* self, MonoObject* value)
	{
		if(!self->IsNativeObjectValid())
			return;

		TShared<TDistribution<TVector3<float>>> tmpvalue;
		ScriptVector3Distribution* scriptObjectWrappervalue;
		scriptObjectWrappervalue = ScriptVector3Distribution::GetScriptObjectWrapper(value);
		if(scriptObjectWrappervalue != nullptr)
			tmpvalue = std::static_pointer_cast<TDistribution<TVector3<float>>>(scriptObjectWrappervalue->GetBaseNativeObjectAsShared());
		static_cast<ParticleEmitter*>(self->GetNativeObject())->SetInitialSize3D(*tmpvalue);
	}

	MonoObject* ScriptParticleEmitter::InternalGetInitialSize3D(ScriptParticleEmitter* self)
	{
		TShared<TDistribution<TVector3<float>>> tmp__output = B3DMakeShared<TDistribution<TVector3<float>>>();
		if(!self->IsNativeObjectValid())
			return {};

		*tmp__output = static_cast<ParticleEmitter*>(self->GetNativeObject())->GetInitialSize3D();

		MonoObject* __output;
		__output = ScriptVector3Distribution::GetOrCreateScriptObject(tmp__output);

		return __output;
	}

	void ScriptParticleEmitter::InternalSetUse3DSize(ScriptParticleEmitter* self, bool value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<ParticleEmitter*>(self->GetNativeObject())->SetUse3DSize(value);
	}

	bool ScriptParticleEmitter::InternalGetUse3DSize(ScriptParticleEmitter* self)
	{
		bool tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<ParticleEmitter*>(self->GetNativeObject())->GetUse3DSize();

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptParticleEmitter::InternalSetInitialRotation(ScriptParticleEmitter* self, MonoObject* value)
	{
		if(!self->IsNativeObjectValid())
			return;

		TShared<TDistribution<float>> tmpvalue;
		ScriptFloatDistribution* scriptObjectWrappervalue;
		scriptObjectWrappervalue = ScriptFloatDistribution::GetScriptObjectWrapper(value);
		if(scriptObjectWrappervalue != nullptr)
			tmpvalue = std::static_pointer_cast<TDistribution<float>>(scriptObjectWrappervalue->GetBaseNativeObjectAsShared());
		static_cast<ParticleEmitter*>(self->GetNativeObject())->SetInitialRotation(*tmpvalue);
	}

	MonoObject* ScriptParticleEmitter::InternalGetInitialRotation(ScriptParticleEmitter* self)
	{
		TShared<TDistribution<float>> tmp__output = B3DMakeShared<TDistribution<float>>();
		if(!self->IsNativeObjectValid())
			return {};

		*tmp__output = static_cast<ParticleEmitter*>(self->GetNativeObject())->GetInitialRotation();

		MonoObject* __output;
		__output = ScriptFloatDistribution::GetOrCreateScriptObject(tmp__output);

		return __output;
	}

	void ScriptParticleEmitter::InternalSetInitialRotation3D(ScriptParticleEmitter* self, MonoObject* value)
	{
		if(!self->IsNativeObjectValid())
			return;

		TShared<TDistribution<TVector3<float>>> tmpvalue;
		ScriptVector3Distribution* scriptObjectWrappervalue;
		scriptObjectWrappervalue = ScriptVector3Distribution::GetScriptObjectWrapper(value);
		if(scriptObjectWrappervalue != nullptr)
			tmpvalue = std::static_pointer_cast<TDistribution<TVector3<float>>>(scriptObjectWrappervalue->GetBaseNativeObjectAsShared());
		static_cast<ParticleEmitter*>(self->GetNativeObject())->SetInitialRotation3D(*tmpvalue);
	}

	MonoObject* ScriptParticleEmitter::InternalGetInitialRotation3D(ScriptParticleEmitter* self)
	{
		TShared<TDistribution<TVector3<float>>> tmp__output = B3DMakeShared<TDistribution<TVector3<float>>>();
		if(!self->IsNativeObjectValid())
			return {};

		*tmp__output = static_cast<ParticleEmitter*>(self->GetNativeObject())->GetInitialRotation3D();

		MonoObject* __output;
		__output = ScriptVector3Distribution::GetOrCreateScriptObject(tmp__output);

		return __output;
	}

	void ScriptParticleEmitter::InternalSetUse3DRotation(ScriptParticleEmitter* self, bool value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<ParticleEmitter*>(self->GetNativeObject())->SetUse3DRotation(value);
	}

	bool ScriptParticleEmitter::InternalGetUse3DRotation(ScriptParticleEmitter* self)
	{
		bool tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<ParticleEmitter*>(self->GetNativeObject())->GetUse3DRotation();

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptParticleEmitter::InternalSetInitialColor(ScriptParticleEmitter* self, MonoObject* value)
	{
		if(!self->IsNativeObjectValid())
			return;

		TShared<TColorDistribution<ColorGradient>> tmpvalue;
		ScriptColorDistribution* scriptObjectWrappervalue;
		scriptObjectWrappervalue = ScriptColorDistribution::GetScriptObjectWrapper(value);
		if(scriptObjectWrappervalue != nullptr)
			tmpvalue = std::static_pointer_cast<TColorDistribution<ColorGradient>>(scriptObjectWrappervalue->GetBaseNativeObjectAsShared());
		static_cast<ParticleEmitter*>(self->GetNativeObject())->SetInitialColor(*tmpvalue);
	}

	MonoObject* ScriptParticleEmitter::InternalGetInitialColor(ScriptParticleEmitter* self)
	{
		TShared<TColorDistribution<ColorGradient>> tmp__output = B3DMakeShared<TColorDistribution<ColorGradient>>();
		if(!self->IsNativeObjectValid())
			return {};

		*tmp__output = static_cast<ParticleEmitter*>(self->GetNativeObject())->GetInitialColor();

		MonoObject* __output;
		__output = ScriptColorDistribution::GetOrCreateScriptObject(tmp__output);

		return __output;
	}

	void ScriptParticleEmitter::InternalSetRandomOffset(ScriptParticleEmitter* self, float value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<ParticleEmitter*>(self->GetNativeObject())->SetRandomOffset(value);
	}

	float ScriptParticleEmitter::InternalGetRandomOffset(ScriptParticleEmitter* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<ParticleEmitter*>(self->GetNativeObject())->GetRandomOffset();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptParticleEmitter::InternalSetFlipU(ScriptParticleEmitter* self, float value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<ParticleEmitter*>(self->GetNativeObject())->SetFlipU(value);
	}

	float ScriptParticleEmitter::InternalGetFlipU(ScriptParticleEmitter* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<ParticleEmitter*>(self->GetNativeObject())->GetFlipU();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptParticleEmitter::InternalSetFlipV(ScriptParticleEmitter* self, float value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<ParticleEmitter*>(self->GetNativeObject())->SetFlipV(value);
	}

	float ScriptParticleEmitter::InternalGetFlipV(ScriptParticleEmitter* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<ParticleEmitter*>(self->GetNativeObject())->GetFlipV();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptParticleEmitter::InternalCreate(MonoObject* scriptObject)
	{
		TShared<ParticleEmitter> nativeObject = ParticleEmitter::Create();
		ScriptObjectWrapper::Create<ScriptParticleEmitter>(nativeObject, scriptObject);
	}
}
