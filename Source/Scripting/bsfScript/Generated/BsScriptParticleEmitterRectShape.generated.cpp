//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptParticleEmitterRectShape.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "BsScriptPARTICLE_RECT_SHAPE_DESC.generated.h"
#include "BsScriptParticleEmitterRectShape.generated.h"

namespace bs
{
	ScriptParticleEmitterRectShape::ScriptParticleEmitterRectShape(const SPtr<ParticleEmitterRectShape>& nativeObject)
		:TScriptReflectableWrapper(nativeObject)
	{
		RegisterEvents();
	}

	void ScriptParticleEmitterRectShape::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetOptions", (void*)&ScriptParticleEmitterRectShape::InternalSetOptions);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetOptions", (void*)&ScriptParticleEmitterRectShape::InternalGetOptions);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create", (void*)&ScriptParticleEmitterRectShape::InternalCreate);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create0", (void*)&ScriptParticleEmitterRectShape::InternalCreate0);

	}

	MonoObject* ScriptParticleEmitterRectShape::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	void ScriptParticleEmitterRectShape::InternalSetOptions(ScriptParticleEmitterRectShape* self, __PARTICLE_RECT_SHAPE_DESCInterop* options)
	{
		if(!self->IsNativeObjectValid())
			return;

		PARTICLE_RECT_SHAPE_DESC tmpoptions;
		tmpoptions = ScriptParticleRectShapeOptions::FromInterop(*options);
		static_cast<ParticleEmitterRectShape*>(self->GetNativeObject())->SetOptions(tmpoptions);
	}

	void ScriptParticleEmitterRectShape::InternalGetOptions(ScriptParticleEmitterRectShape* self, __PARTICLE_RECT_SHAPE_DESCInterop* __output)
	{
		if(!self->IsNativeObjectValid())
			{
				__output = {};
				return;
			}

		PARTICLE_RECT_SHAPE_DESC tmp__output;
		tmp__output = static_cast<ParticleEmitterRectShape*>(self->GetNativeObject())->GetOptions();

		__PARTICLE_RECT_SHAPE_DESCInterop interop__output;
		interop__output = ScriptParticleRectShapeOptions::ToInterop(tmp__output);
		MonoUtil::ValueCopy(__output, &interop__output, ScriptParticleRectShapeOptions::GetMetaData()->ScriptClass->GetInternalClass());
	}

	void ScriptParticleEmitterRectShape::InternalCreate(MonoObject* scriptObject, __PARTICLE_RECT_SHAPE_DESCInterop* desc)
	{
		PARTICLE_RECT_SHAPE_DESC tmpdesc;
		tmpdesc = ScriptParticleRectShapeOptions::FromInterop(*desc);
		SPtr<ParticleEmitterRectShape> nativeObject = ParticleEmitterRectShape::Create(tmpdesc);
		ScriptObjectWrapper::Create<ScriptParticleEmitterRectShape>(nativeObject, scriptObject);
	}

	void ScriptParticleEmitterRectShape::InternalCreate0(MonoObject* scriptObject)
	{
		SPtr<ParticleEmitterRectShape> nativeObject = ParticleEmitterRectShape::Create();
		ScriptObjectWrapper::Create<ScriptParticleEmitterRectShape>(nativeObject, scriptObject);
	}
}
