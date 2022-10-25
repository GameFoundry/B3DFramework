//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
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
		: TScriptReflectable(managedInstance, value)
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
		new(bs_alloc<ScriptParticleColor>()) ScriptParticleColor(managedInstance, value);
		return managedInstance;
	}

	void ScriptParticleColor::InternalSetOptions(ScriptParticleColor* thisPtr, __PARTICLE_COLOR_DESCInterop* options)
	{
		PARTICLE_COLOR_DESC tmpoptions;
		tmpoptions = ScriptPARTICLE_COLOR_DESC::FromInterop(*options);
		thisPtr->GetInternal()->SetOptions(tmpoptions);
	}

	void ScriptParticleColor::InternalGetOptions(ScriptParticleColor* thisPtr, __PARTICLE_COLOR_DESCInterop* __output)
	{
		PARTICLE_COLOR_DESC tmp__output;
		tmp__output = thisPtr->GetInternal()->GetOptions();

		__PARTICLE_COLOR_DESCInterop interop__output;
		interop__output = ScriptPARTICLE_COLOR_DESC::ToInterop(tmp__output);
		MonoUtil::ValueCopy(__output, &interop__output, ScriptPARTICLE_COLOR_DESC::GetMetaData()->ScriptClass->GetInternalClassInternal());
	}

	void ScriptParticleColor::InternalCreate(MonoObject* managedInstance, __PARTICLE_COLOR_DESCInterop* desc)
	{
		PARTICLE_COLOR_DESC tmpdesc;
		tmpdesc = ScriptPARTICLE_COLOR_DESC::FromInterop(*desc);
		SPtr<ParticleColor> instance = ParticleColor::Create(tmpdesc);
		new(bs_alloc<ScriptParticleColor>()) ScriptParticleColor(managedInstance, instance);
	}

	void ScriptParticleColor::InternalCreate0(MonoObject* managedInstance)
	{
		SPtr<ParticleColor> instance = ParticleColor::Create();
		new(bs_alloc<ScriptParticleColor>()) ScriptParticleColor(managedInstance, instance);
	}
} // namespace bs
