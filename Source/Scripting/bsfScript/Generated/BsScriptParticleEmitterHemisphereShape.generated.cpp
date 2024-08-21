//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptParticleEmitterHemisphereShape.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "BsScriptPARTICLE_HEMISPHERE_SHAPE_DESC.generated.h"
#include "BsScriptParticleEmitterHemisphereShape.generated.h"

namespace bs
{
	ScriptParticleEmitterHemisphereShape::ScriptParticleEmitterHemisphereShape(const SPtr<ParticleEmitterHemisphereShape>& nativeObject, MonoObject* scriptObject)
		:TScriptReflectableWrapper(nativeObject, scriptObject)
	{
	}

	void ScriptParticleEmitterHemisphereShape::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetOptions", (void*)&ScriptParticleEmitterHemisphereShape::InternalSetOptions);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetOptions", (void*)&ScriptParticleEmitterHemisphereShape::InternalGetOptions);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create", (void*)&ScriptParticleEmitterHemisphereShape::InternalCreate);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create0", (void*)&ScriptParticleEmitterHemisphereShape::InternalCreate0);

	}

	MonoObject* ScriptParticleEmitterHemisphereShape::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	void ScriptParticleEmitterHemisphereShape::InternalSetOptions(ScriptParticleEmitterHemisphereShape* self, PARTICLE_HEMISPHERE_SHAPE_DESC* options)
	{
		static_cast<ParticleEmitterHemisphereShape*>(self->GetNativeObject())->SetOptions(*options);
	}

	void ScriptParticleEmitterHemisphereShape::InternalGetOptions(ScriptParticleEmitterHemisphereShape* self, PARTICLE_HEMISPHERE_SHAPE_DESC* __output)
	{
		PARTICLE_HEMISPHERE_SHAPE_DESC tmp__output;
		tmp__output = static_cast<ParticleEmitterHemisphereShape*>(self->GetNativeObject())->GetOptions();

		*__output = tmp__output;
	}

	void ScriptParticleEmitterHemisphereShape::InternalCreate(MonoObject* scriptObject, PARTICLE_HEMISPHERE_SHAPE_DESC* desc)
	{
		SPtr<ParticleEmitterHemisphereShape> nativeObject = ParticleEmitterHemisphereShape::Create(*desc);
		B3DNew<ScriptParticleEmitterHemisphereShape>(nativeObject, scriptObject);
	}

	void ScriptParticleEmitterHemisphereShape::InternalCreate0(MonoObject* scriptObject)
	{
		SPtr<ParticleEmitterHemisphereShape> nativeObject = ParticleEmitterHemisphereShape::Create();
		B3DNew<ScriptParticleEmitterHemisphereShape>(nativeObject, scriptObject);
	}
}
