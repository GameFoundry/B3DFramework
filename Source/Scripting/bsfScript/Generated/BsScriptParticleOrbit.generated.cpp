//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptParticleOrbit.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "BsScriptPARTICLE_ORBIT_DESC.generated.h"
#include "BsScriptParticleOrbit.generated.h"

namespace bs
{
	ScriptParticleOrbit::ScriptParticleOrbit(const SPtr<ParticleOrbit>& nativeObject)
		:TScriptReflectableWrapper(nativeObject)
	{
		RegisterEvents();
	}

	void ScriptParticleOrbit::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetOptions", (void*)&ScriptParticleOrbit::InternalSetOptions);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetOptions", (void*)&ScriptParticleOrbit::InternalGetOptions);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create", (void*)&ScriptParticleOrbit::InternalCreate);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create0", (void*)&ScriptParticleOrbit::InternalCreate0);

	}

	MonoObject* ScriptParticleOrbit::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	void ScriptParticleOrbit::InternalSetOptions(ScriptParticleOrbit* self, __PARTICLE_ORBIT_DESCInterop* options)
	{
		if(!self->IsNativeObjectValid())
			return;

		PARTICLE_ORBIT_DESC tmpoptions;
		tmpoptions = ScriptParticleOrbitOptions::FromInterop(*options);
		static_cast<ParticleOrbit*>(self->GetNativeObject())->SetOptions(tmpoptions);
	}

	void ScriptParticleOrbit::InternalGetOptions(ScriptParticleOrbit* self, __PARTICLE_ORBIT_DESCInterop* __output)
	{
		if(!self->IsNativeObjectValid())
			{
				__output = {};
				return;
			}

		PARTICLE_ORBIT_DESC tmp__output;
		tmp__output = static_cast<ParticleOrbit*>(self->GetNativeObject())->GetOptions();

		__PARTICLE_ORBIT_DESCInterop interop__output;
		interop__output = ScriptParticleOrbitOptions::ToInterop(tmp__output);
		MonoUtil::ValueCopy(__output, &interop__output, ScriptParticleOrbitOptions::GetMetaData()->ScriptClass->GetInternalClass());
	}

	void ScriptParticleOrbit::InternalCreate(MonoObject* scriptObject, __PARTICLE_ORBIT_DESCInterop* desc)
	{
		PARTICLE_ORBIT_DESC tmpdesc;
		tmpdesc = ScriptParticleOrbitOptions::FromInterop(*desc);
		SPtr<ParticleOrbit> nativeObject = ParticleOrbit::Create(tmpdesc);
		ScriptObjectWrapper::Create<ScriptParticleOrbit>(nativeObject, scriptObject);
	}

	void ScriptParticleOrbit::InternalCreate0(MonoObject* scriptObject)
	{
		SPtr<ParticleOrbit> nativeObject = ParticleOrbit::Create();
		ScriptObjectWrapper::Create<ScriptParticleOrbit>(nativeObject, scriptObject);
	}
}
