//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptParticleColor.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "BsScriptPARTICLE_COLOR_DESC.generated.h"
#include "BsScriptParticleColor.generated.h"

namespace bs
{
	ScriptParticleColor::ScriptParticleColor(MonoObject* managedInstance, const SPtr<ParticleColor>& value)
		:TScriptReflectable(managedInstance, value)
	{
		mInternal = value;
	}

	void ScriptParticleColor::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_SetOptions", (void*)&ScriptParticleColor::InternalSetOptions);
		metaData.ScriptClass->AddInternalCall("Internal_GetOptions", (void*)&ScriptParticleColor::InternalGetOptions);
		metaData.ScriptClass->AddInternalCall("Internal_Create", (void*)&ScriptParticleColor::InternalCreate);
		metaData.ScriptClass->AddInternalCall("Internal_Create0", (void*)&ScriptParticleColor::InternalCreate0);

	}

	MonoObject* ScriptParticleColor::Create(const SPtr<ParticleColor>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.ScriptClass->CreateInstance("bool", ctorParams);
		new (B3DAllocate<ScriptParticleColor>()) ScriptParticleColor(managedInstance, value);
		return managedInstance;
	}
	void ScriptParticleColor::InternalSetOptions(ScriptParticleColor* self, __PARTICLE_COLOR_DESCInterop* options)
	{
		PARTICLE_COLOR_DESC tmpoptions;
		tmpoptions = ScriptParticleColorOptions::FromInterop(*options);
		self->GetInternal()->SetOptions(tmpoptions);
	}

	void ScriptParticleColor::InternalGetOptions(ScriptParticleColor* self, __PARTICLE_COLOR_DESCInterop* __output)
	{
		PARTICLE_COLOR_DESC tmp__output;
		tmp__output = self->GetInternal()->GetOptions();

		__PARTICLE_COLOR_DESCInterop interop__output;
		interop__output = ScriptParticleColorOptions::ToInterop(tmp__output);
		MonoUtil::ValueCopy(__output, &interop__output, ScriptParticleColorOptions::GetMetaData()->ScriptClass->GetInternalClassInternal());
	}

	void ScriptParticleColor::InternalCreate(MonoObject* managedInstance, __PARTICLE_COLOR_DESCInterop* desc)
	{
		PARTICLE_COLOR_DESC tmpdesc;
		tmpdesc = ScriptParticleColorOptions::FromInterop(*desc);
		SPtr<ParticleColor> nativeObject = ParticleColor::Create(tmpdesc);
		new (B3DAllocate<ScriptParticleColor>())ScriptParticleColor(managedInstance, nativeObject);
	}

	void ScriptParticleColor::InternalCreate0(MonoObject* managedInstance)
	{
		SPtr<ParticleColor> nativeObject = ParticleColor::Create();
		new (B3DAllocate<ScriptParticleColor>())ScriptParticleColor(managedInstance, nativeObject);
	}
}
