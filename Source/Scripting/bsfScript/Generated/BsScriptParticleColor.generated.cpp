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
	ScriptParticleColor::ScriptParticleColor(const SPtr<ParticleColor>& nativeObject, MonoObject* scriptObject)
		:TScriptReflectableWrapper(nativeObject, scriptObject)
	{
	}

	void ScriptParticleColor::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetOptions", (void*)&ScriptParticleColor::InternalSetOptions);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetOptions", (void*)&ScriptParticleColor::InternalGetOptions);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create", (void*)&ScriptParticleColor::InternalCreate);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create0", (void*)&ScriptParticleColor::InternalCreate0);

	}

	MonoObject* ScriptParticleColor::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	void ScriptParticleColor::InternalSetOptions(ScriptParticleColor* self, __PARTICLE_COLOR_DESCInterop* options)
	{
		PARTICLE_COLOR_DESC tmpoptions;
		tmpoptions = ScriptParticleColorOptions::FromInterop(*options);
		static_cast<ParticleColor*>(self->GetNativeObject())->SetOptions(tmpoptions);
	}

	void ScriptParticleColor::InternalGetOptions(ScriptParticleColor* self, __PARTICLE_COLOR_DESCInterop* __output)
	{
		PARTICLE_COLOR_DESC tmp__output;
		tmp__output = static_cast<ParticleColor*>(self->GetNativeObject())->GetOptions();

		__PARTICLE_COLOR_DESCInterop interop__output;
		interop__output = ScriptParticleColorOptions::ToInterop(tmp__output);
		MonoUtil::ValueCopy(__output, &interop__output, ScriptParticleColorOptions::GetMetaData()->ScriptClass->GetInternalClass());
	}

	void ScriptParticleColor::InternalCreate(MonoObject* scriptObject, __PARTICLE_COLOR_DESCInterop* desc)
	{
		PARTICLE_COLOR_DESC tmpdesc;
		tmpdesc = ScriptParticleColorOptions::FromInterop(*desc);
		SPtr<ParticleColor> nativeObject = ParticleColor::Create(tmpdesc);
		B3DNew<ScriptParticleColor>(nativeObject, scriptObject);
	}

	void ScriptParticleColor::InternalCreate0(MonoObject* scriptObject)
	{
		SPtr<ParticleColor> nativeObject = ParticleColor::Create();
		B3DNew<ScriptParticleColor>(nativeObject, scriptObject);
	}
}
