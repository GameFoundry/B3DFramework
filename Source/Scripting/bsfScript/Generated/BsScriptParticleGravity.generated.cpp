//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptParticleGravity.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "BsScriptPARTICLE_GRAVITY_DESC.generated.h"
#include "BsScriptParticleGravity.generated.h"

namespace bs
{
	ScriptParticleGravity::ScriptParticleGravity(const SPtr<ParticleGravity>& nativeObject, MonoObject* scriptObject)
		:TScriptReflectableWrapper(nativeObject, scriptObject)
	{
	}

	void ScriptParticleGravity::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetOptions", (void*)&ScriptParticleGravity::InternalSetOptions);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetOptions", (void*)&ScriptParticleGravity::InternalGetOptions);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create", (void*)&ScriptParticleGravity::InternalCreate);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create0", (void*)&ScriptParticleGravity::InternalCreate0);

	}

	MonoObject* ScriptParticleGravity::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	void ScriptParticleGravity::InternalSetOptions(ScriptParticleGravity* self, PARTICLE_GRAVITY_DESC* options)
	{
		static_cast<ParticleGravity*>(self->GetNativeObject())->SetOptions(*options);
	}

	void ScriptParticleGravity::InternalGetOptions(ScriptParticleGravity* self, PARTICLE_GRAVITY_DESC* __output)
	{
		PARTICLE_GRAVITY_DESC tmp__output;
		tmp__output = static_cast<ParticleGravity*>(self->GetNativeObject())->GetOptions();

		*__output = tmp__output;
	}

	void ScriptParticleGravity::InternalCreate(MonoObject* scriptObject, PARTICLE_GRAVITY_DESC* desc)
	{
		SPtr<ParticleGravity> nativeObject = ParticleGravity::Create(*desc);
		B3DNew<ScriptParticleGravity>(nativeObject, scriptObject);
	}

	void ScriptParticleGravity::InternalCreate0(MonoObject* scriptObject)
	{
		SPtr<ParticleGravity> nativeObject = ParticleGravity::Create();
		B3DNew<ScriptParticleGravity>(nativeObject, scriptObject);
	}
}
