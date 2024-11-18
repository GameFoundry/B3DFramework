//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptParticleSize.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "BsScriptPARTICLE_SIZE_DESC.generated.h"
#include "BsScriptParticleSize.generated.h"

namespace bs
{
	ScriptParticleSize::ScriptParticleSize(const SPtr<ParticleSize>& nativeObject)
		:TScriptReflectableWrapper(nativeObject)
	{
		RegisterEvents();
	}

	void ScriptParticleSize::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetOptions", (void*)&ScriptParticleSize::InternalSetOptions);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetOptions", (void*)&ScriptParticleSize::InternalGetOptions);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create", (void*)&ScriptParticleSize::InternalCreate);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create0", (void*)&ScriptParticleSize::InternalCreate0);

	}

	MonoObject* ScriptParticleSize::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	void ScriptParticleSize::InternalSetOptions(ScriptParticleSize* self, __PARTICLE_SIZE_DESCInterop* options)
	{
		if(!self->IsNativeObjectValid())
			return;

		PARTICLE_SIZE_DESC tmpoptions;
		tmpoptions = ScriptParticleSizeOptions::FromInterop(*options);
		static_cast<ParticleSize*>(self->GetNativeObject())->SetOptions(tmpoptions);
	}

	void ScriptParticleSize::InternalGetOptions(ScriptParticleSize* self, __PARTICLE_SIZE_DESCInterop* __output)
	{
		if(!self->IsNativeObjectValid())
			{
				__output = {};
				return;
			}

		PARTICLE_SIZE_DESC tmp__output;
		tmp__output = static_cast<ParticleSize*>(self->GetNativeObject())->GetOptions();

		__PARTICLE_SIZE_DESCInterop interop__output;
		interop__output = ScriptParticleSizeOptions::ToInterop(tmp__output);
		MonoUtil::ValueCopy(__output, &interop__output, ScriptParticleSizeOptions::GetMetaData()->ScriptClass->GetInternalClass());
	}

	void ScriptParticleSize::InternalCreate(MonoObject* scriptObject, __PARTICLE_SIZE_DESCInterop* desc)
	{
		PARTICLE_SIZE_DESC tmpdesc;
		tmpdesc = ScriptParticleSizeOptions::FromInterop(*desc);
		SPtr<ParticleSize> nativeObject = ParticleSize::Create(tmpdesc);
		ScriptObjectWrapper::Create<ScriptParticleSize>(nativeObject, scriptObject);
	}

	void ScriptParticleSize::InternalCreate0(MonoObject* scriptObject)
	{
		SPtr<ParticleSize> nativeObject = ParticleSize::Create();
		ScriptObjectWrapper::Create<ScriptParticleSize>(nativeObject, scriptObject);
	}
}
