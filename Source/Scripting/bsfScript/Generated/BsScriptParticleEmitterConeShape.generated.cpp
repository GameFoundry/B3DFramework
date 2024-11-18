//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptParticleEmitterConeShape.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "BsScriptPARTICLE_CONE_SHAPE_DESC.generated.h"
#include "BsScriptParticleEmitterConeShape.generated.h"

namespace bs
{
	ScriptParticleEmitterConeShape::ScriptParticleEmitterConeShape(const SPtr<ParticleEmitterConeShape>& nativeObject)
		:TScriptReflectableWrapper(nativeObject)
	{
		RegisterEvents();
	}

	void ScriptParticleEmitterConeShape::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetOptions", (void*)&ScriptParticleEmitterConeShape::InternalSetOptions);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetOptions", (void*)&ScriptParticleEmitterConeShape::InternalGetOptions);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create", (void*)&ScriptParticleEmitterConeShape::InternalCreate);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create0", (void*)&ScriptParticleEmitterConeShape::InternalCreate0);

	}

	MonoObject* ScriptParticleEmitterConeShape::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	void ScriptParticleEmitterConeShape::InternalSetOptions(ScriptParticleEmitterConeShape* self, __PARTICLE_CONE_SHAPE_DESCInterop* options)
	{
		if(!self->IsNativeObjectValid())
			return;

		PARTICLE_CONE_SHAPE_DESC tmpoptions;
		tmpoptions = ScriptParticleConeShapeOptions::FromInterop(*options);
		static_cast<ParticleEmitterConeShape*>(self->GetNativeObject())->SetOptions(tmpoptions);
	}

	void ScriptParticleEmitterConeShape::InternalGetOptions(ScriptParticleEmitterConeShape* self, __PARTICLE_CONE_SHAPE_DESCInterop* __output)
	{
		if(!self->IsNativeObjectValid())
			{
				__output = {};
				return;
			}

		PARTICLE_CONE_SHAPE_DESC tmp__output;
		tmp__output = static_cast<ParticleEmitterConeShape*>(self->GetNativeObject())->GetOptions();

		__PARTICLE_CONE_SHAPE_DESCInterop interop__output;
		interop__output = ScriptParticleConeShapeOptions::ToInterop(tmp__output);
		MonoUtil::ValueCopy(__output, &interop__output, ScriptParticleConeShapeOptions::GetMetaData()->ScriptClass->GetInternalClass());
	}

	void ScriptParticleEmitterConeShape::InternalCreate(MonoObject* scriptObject, __PARTICLE_CONE_SHAPE_DESCInterop* desc)
	{
		PARTICLE_CONE_SHAPE_DESC tmpdesc;
		tmpdesc = ScriptParticleConeShapeOptions::FromInterop(*desc);
		SPtr<ParticleEmitterConeShape> nativeObject = ParticleEmitterConeShape::Create(tmpdesc);
		ScriptObjectWrapper::Create<ScriptParticleEmitterConeShape>(nativeObject, scriptObject);
	}

	void ScriptParticleEmitterConeShape::InternalCreate0(MonoObject* scriptObject)
	{
		SPtr<ParticleEmitterConeShape> nativeObject = ParticleEmitterConeShape::Create();
		ScriptObjectWrapper::Create<ScriptParticleEmitterConeShape>(nativeObject, scriptObject);
	}
}
