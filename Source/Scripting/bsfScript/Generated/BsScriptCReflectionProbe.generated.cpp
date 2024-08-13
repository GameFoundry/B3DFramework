//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptCReflectionProbe.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Components/BsCReflectionProbe.h"
#include "BsScriptResourceManager.h"
#include "Wrappers/BsScriptRRefBase.h"
#include "Wrappers/BsScriptVector.h"
#include "../../../Foundation/bsfCore/Image/BsTexture.h"

namespace bs
{
	ScriptReflectionProbe::ScriptReflectionProbe(MonoObject* managedInstance, const GameObjectHandle<CReflectionProbe>& value)
		:TScriptComponent(managedInstance, value)
	{
	}

	void ScriptReflectionProbe::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_GetType", (void*)&ScriptReflectionProbe::InternalGetType);
		metaData.ScriptClass->AddInternalCall("Internal_SetType", (void*)&ScriptReflectionProbe::InternalSetType);
		metaData.ScriptClass->AddInternalCall("Internal_GetRadius", (void*)&ScriptReflectionProbe::InternalGetRadius);
		metaData.ScriptClass->AddInternalCall("Internal_SetRadius", (void*)&ScriptReflectionProbe::InternalSetRadius);
		metaData.ScriptClass->AddInternalCall("Internal_GetExtents", (void*)&ScriptReflectionProbe::InternalGetExtents);
		metaData.ScriptClass->AddInternalCall("Internal_SetExtents", (void*)&ScriptReflectionProbe::InternalSetExtents);
		metaData.ScriptClass->AddInternalCall("Internal_GetCustomTexture", (void*)&ScriptReflectionProbe::InternalGetCustomTexture);
		metaData.ScriptClass->AddInternalCall("Internal_SetCustomTexture", (void*)&ScriptReflectionProbe::InternalSetCustomTexture);
		metaData.ScriptClass->AddInternalCall("Internal_Capture", (void*)&ScriptReflectionProbe::InternalCapture);

	}

	ReflectionProbeType ScriptReflectionProbe::InternalGetType(ScriptReflectionProbe* self)
	{
		ReflectionProbeType tmp__output;
		tmp__output = self->GetHandle()->GetType();

		ReflectionProbeType __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptReflectionProbe::InternalSetType(ScriptReflectionProbe* self, ReflectionProbeType type)
	{
		self->GetHandle()->SetType(type);
	}

	float ScriptReflectionProbe::InternalGetRadius(ScriptReflectionProbe* self)
	{
		float tmp__output;
		tmp__output = self->GetHandle()->GetRadius();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptReflectionProbe::InternalSetRadius(ScriptReflectionProbe* self, float radius)
	{
		self->GetHandle()->SetRadius(radius);
	}

	void ScriptReflectionProbe::InternalGetExtents(ScriptReflectionProbe* self, TVector3<float>* __output)
	{
		TVector3<float> tmp__output;
		tmp__output = self->GetHandle()->GetExtents();

		*__output = tmp__output;
	}

	void ScriptReflectionProbe::InternalSetExtents(ScriptReflectionProbe* self, TVector3<float>* extents)
	{
		self->GetHandle()->SetExtents(*extents);
	}

	MonoObject* ScriptReflectionProbe::InternalGetCustomTexture(ScriptReflectionProbe* self)
	{
		TResourceHandle<Texture> tmp__output;
		tmp__output = self->GetHandle()->GetCustomTexture();

		MonoObject* __output;
		ScriptRRefBase* script__output;
		script__output = ScriptResourceManager::Instance().GetScriptRRef(tmp__output);
		if(script__output != nullptr)
			__output = script__output->GetManagedInstance();
		else
			__output = nullptr;

		return __output;
	}

	void ScriptReflectionProbe::InternalSetCustomTexture(ScriptReflectionProbe* self, MonoObject* texture)
	{
		TResourceHandle<Texture> tmptexture;
		ScriptRRefBase* scriptObjectWrappertexture;
		scriptObjectWrappertexture = ScriptRRefBase::ToNative(texture);
		if(scriptObjectWrappertexture != nullptr)
			tmptexture = B3DStaticResourceCast<Texture>(scriptObjectWrappertexture->GetHandle());
		self->GetHandle()->SetCustomTexture(tmptexture);
	}

	void ScriptReflectionProbe::InternalCapture(ScriptReflectionProbe* self)
	{
		self->GetHandle()->Capture();
	}
}
