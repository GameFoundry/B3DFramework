//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptParticleEmitterBoxShape.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "BsScriptPARTICLE_BOX_SHAPE_DESC.generated.h"
#include "BsScriptParticleEmitterBoxShape.generated.h"

namespace bs
{
	ScriptParticleEmitterBoxShape::ScriptParticleEmitterBoxShape(const SPtr<ParticleEmitterBoxShape>& nativeObject, MonoObject* scriptObject)
		:TScriptReflectableWrapper(nativeObject, scriptObject)
	{
	}

	void ScriptParticleEmitterBoxShape::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetOptions", (void*)&ScriptParticleEmitterBoxShape::InternalSetOptions);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetOptions", (void*)&ScriptParticleEmitterBoxShape::InternalGetOptions);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create", (void*)&ScriptParticleEmitterBoxShape::InternalCreate);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create0", (void*)&ScriptParticleEmitterBoxShape::InternalCreate0);

	}

	MonoObject* ScriptParticleEmitterBoxShape::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	void ScriptParticleEmitterBoxShape::InternalSetOptions(ScriptParticleEmitterBoxShape* self, __PARTICLE_BOX_SHAPE_DESCInterop* options)
	{
		PARTICLE_BOX_SHAPE_DESC tmpoptions;
		tmpoptions = ScriptParticleBoxShapeOptions::FromInterop(*options);
		static_cast<ParticleEmitterBoxShape*>(self->GetNativeObject())->SetOptions(tmpoptions);
	}

	void ScriptParticleEmitterBoxShape::InternalGetOptions(ScriptParticleEmitterBoxShape* self, __PARTICLE_BOX_SHAPE_DESCInterop* __output)
	{
		PARTICLE_BOX_SHAPE_DESC tmp__output;
		tmp__output = static_cast<ParticleEmitterBoxShape*>(self->GetNativeObject())->GetOptions();

		__PARTICLE_BOX_SHAPE_DESCInterop interop__output;
		interop__output = ScriptParticleBoxShapeOptions::ToInterop(tmp__output);
		MonoUtil::ValueCopy(__output, &interop__output, ScriptParticleBoxShapeOptions::GetMetaData()->ScriptClass->GetInternalClass());
	}

	void ScriptParticleEmitterBoxShape::InternalCreate(MonoObject* scriptObject, __PARTICLE_BOX_SHAPE_DESCInterop* desc)
	{
		PARTICLE_BOX_SHAPE_DESC tmpdesc;
		tmpdesc = ScriptParticleBoxShapeOptions::FromInterop(*desc);
		SPtr<ParticleEmitterBoxShape> nativeObject = ParticleEmitterBoxShape::Create(tmpdesc);
		B3DNew<ScriptParticleEmitterBoxShape>(nativeObject, scriptObject);
	}

	void ScriptParticleEmitterBoxShape::InternalCreate0(MonoObject* scriptObject)
	{
		SPtr<ParticleEmitterBoxShape> nativeObject = ParticleEmitterBoxShape::Create();
		B3DNew<ScriptParticleEmitterBoxShape>(nativeObject, scriptObject);
	}
}
