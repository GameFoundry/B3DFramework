//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptParticleRotation.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "BsScriptPARTICLE_ROTATION_DESC.generated.h"
#include "BsScriptParticleRotation.generated.h"

namespace bs
{
	ScriptParticleRotation::ScriptParticleRotation(const SPtr<ParticleRotation>& nativeObject, MonoObject* scriptObject)
		:TScriptReflectableWrapper(nativeObject, scriptObject)
	{
	}

	void ScriptParticleRotation::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetOptions", (void*)&ScriptParticleRotation::InternalSetOptions);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetOptions", (void*)&ScriptParticleRotation::InternalGetOptions);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create", (void*)&ScriptParticleRotation::InternalCreate);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create0", (void*)&ScriptParticleRotation::InternalCreate0);

	}

	MonoObject* ScriptParticleRotation::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	void ScriptParticleRotation::InternalSetOptions(ScriptParticleRotation* self, __PARTICLE_ROTATION_DESCInterop* options)
	{
		PARTICLE_ROTATION_DESC tmpoptions;
		tmpoptions = ScriptParticleRotationOptions::FromInterop(*options);
		static_cast<ParticleRotation*>(self->GetNativeObject())->SetOptions(tmpoptions);
	}

	void ScriptParticleRotation::InternalGetOptions(ScriptParticleRotation* self, __PARTICLE_ROTATION_DESCInterop* __output)
	{
		PARTICLE_ROTATION_DESC tmp__output;
		tmp__output = static_cast<ParticleRotation*>(self->GetNativeObject())->GetOptions();

		__PARTICLE_ROTATION_DESCInterop interop__output;
		interop__output = ScriptParticleRotationOptions::ToInterop(tmp__output);
		MonoUtil::ValueCopy(__output, &interop__output, ScriptParticleRotationOptions::GetMetaData()->ScriptClass->GetInternalClass());
	}

	void ScriptParticleRotation::InternalCreate(MonoObject* scriptObject, __PARTICLE_ROTATION_DESCInterop* desc)
	{
		PARTICLE_ROTATION_DESC tmpdesc;
		tmpdesc = ScriptParticleRotationOptions::FromInterop(*desc);
		SPtr<ParticleRotation> nativeObject = ParticleRotation::Create(tmpdesc);
		B3DNew<ScriptParticleRotation>(nativeObject, scriptObject);
	}

	void ScriptParticleRotation::InternalCreate0(MonoObject* scriptObject)
	{
		SPtr<ParticleRotation> nativeObject = ParticleRotation::Create();
		B3DNew<ScriptParticleRotation>(nativeObject, scriptObject);
	}
}
