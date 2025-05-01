//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptParticleForce.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "BsScriptPARTICLE_FORCE_DESC.generated.h"
#include "BsScriptParticleForce.generated.h"

namespace bs
{
	ScriptParticleForce::ScriptParticleForce(const SPtr<ParticleForce>& nativeObject)
		:TScriptReflectableWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptParticleForce::~ScriptParticleForce()
	{
		UnregisterEvents();
	}

	void ScriptParticleForce::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetOptions", (void*)&ScriptParticleForce::InternalSetOptions);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetOptions", (void*)&ScriptParticleForce::InternalGetOptions);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create", (void*)&ScriptParticleForce::InternalCreate);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create0", (void*)&ScriptParticleForce::InternalCreate0);

	}

	MonoObject* ScriptParticleForce::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	void ScriptParticleForce::InternalSetOptions(ScriptParticleForce* self, __PARTICLE_FORCE_DESCInterop* options)
	{
		if(!self->IsNativeObjectValid())
			return;

		PARTICLE_FORCE_DESC tmpoptions;
		tmpoptions = ScriptParticleForceOptions::FromInterop(*options);
		static_cast<ParticleForce*>(self->GetNativeObject())->SetOptions(tmpoptions);
	}

	void ScriptParticleForce::InternalGetOptions(ScriptParticleForce* self, __PARTICLE_FORCE_DESCInterop* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		PARTICLE_FORCE_DESC tmp__output;
		tmp__output = static_cast<ParticleForce*>(self->GetNativeObject())->GetOptions();

		__PARTICLE_FORCE_DESCInterop interop__output;
		interop__output = ScriptParticleForceOptions::ToInterop(tmp__output);
		MonoUtil::ValueCopy(__output, &interop__output, ScriptParticleForceOptions::GetMetaData()->ScriptClass->GetInternalClass());
	}

	void ScriptParticleForce::InternalCreate(MonoObject* scriptObject, __PARTICLE_FORCE_DESCInterop* desc)
	{
		PARTICLE_FORCE_DESC tmpdesc;
		tmpdesc = ScriptParticleForceOptions::FromInterop(*desc);
		SPtr<ParticleForce> nativeObject = ParticleForce::Create(tmpdesc);
		ScriptObjectWrapper::Create<ScriptParticleForce>(nativeObject, scriptObject);
	}

	void ScriptParticleForce::InternalCreate0(MonoObject* scriptObject)
	{
		SPtr<ParticleForce> nativeObject = ParticleForce::Create();
		ScriptObjectWrapper::Create<ScriptParticleForce>(nativeObject, scriptObject);
	}
}
