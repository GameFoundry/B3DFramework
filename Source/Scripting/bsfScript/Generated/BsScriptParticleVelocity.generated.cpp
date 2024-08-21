//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptParticleVelocity.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "BsScriptPARTICLE_VELOCITY_DESC.generated.h"
#include "BsScriptParticleVelocity.generated.h"

namespace bs
{
	ScriptParticleVelocity::ScriptParticleVelocity(const SPtr<ParticleVelocity>& nativeObject, MonoObject* scriptObject)
		:TScriptReflectableWrapper(nativeObject, scriptObject)
	{
	}

	void ScriptParticleVelocity::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetOptions", (void*)&ScriptParticleVelocity::InternalSetOptions);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetOptions", (void*)&ScriptParticleVelocity::InternalGetOptions);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create", (void*)&ScriptParticleVelocity::InternalCreate);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create0", (void*)&ScriptParticleVelocity::InternalCreate0);

	}

	MonoObject* ScriptParticleVelocity::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	void ScriptParticleVelocity::InternalSetOptions(ScriptParticleVelocity* self, __PARTICLE_VELOCITY_DESCInterop* options)
	{
		PARTICLE_VELOCITY_DESC tmpoptions;
		tmpoptions = ScriptParticleVelocityOptions::FromInterop(*options);
		static_cast<ParticleVelocity*>(self->GetNativeObject())->SetOptions(tmpoptions);
	}

	void ScriptParticleVelocity::InternalGetOptions(ScriptParticleVelocity* self, __PARTICLE_VELOCITY_DESCInterop* __output)
	{
		PARTICLE_VELOCITY_DESC tmp__output;
		tmp__output = static_cast<ParticleVelocity*>(self->GetNativeObject())->GetOptions();

		__PARTICLE_VELOCITY_DESCInterop interop__output;
		interop__output = ScriptParticleVelocityOptions::ToInterop(tmp__output);
		MonoUtil::ValueCopy(__output, &interop__output, ScriptParticleVelocityOptions::GetMetaData()->ScriptClass->GetInternalClass());
	}

	void ScriptParticleVelocity::InternalCreate(MonoObject* scriptObject, __PARTICLE_VELOCITY_DESCInterop* desc)
	{
		PARTICLE_VELOCITY_DESC tmpdesc;
		tmpdesc = ScriptParticleVelocityOptions::FromInterop(*desc);
		SPtr<ParticleVelocity> nativeObject = ParticleVelocity::Create(tmpdesc);
		B3DNew<ScriptParticleVelocity>(nativeObject, scriptObject);
	}

	void ScriptParticleVelocity::InternalCreate0(MonoObject* scriptObject)
	{
		SPtr<ParticleVelocity> nativeObject = ParticleVelocity::Create();
		B3DNew<ScriptParticleVelocity>(nativeObject, scriptObject);
	}
}
