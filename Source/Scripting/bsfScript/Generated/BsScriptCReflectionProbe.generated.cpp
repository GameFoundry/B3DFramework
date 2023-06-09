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

	ReflectionProbeType ScriptReflectionProbe::InternalGetType(ScriptReflectionProbe* thisPtr)
	{
		ReflectionProbeType tmp__output;
		tmp__output = thisPtr->GetHandle()->GetType();

		ReflectionProbeType __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptReflectionProbe::InternalSetType(ScriptReflectionProbe* thisPtr, ReflectionProbeType type)
	{
		thisPtr->GetHandle()->SetType(type);
	}

	float ScriptReflectionProbe::InternalGetRadius(ScriptReflectionProbe* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetHandle()->GetRadius();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptReflectionProbe::InternalSetRadius(ScriptReflectionProbe* thisPtr, float radius)
	{
		thisPtr->GetHandle()->SetRadius(radius);
	}

	void ScriptReflectionProbe::InternalGetExtents(ScriptReflectionProbe* thisPtr, TVector3<float>* __output)
	{
		TVector3<float> tmp__output;
		tmp__output = thisPtr->GetHandle()->GetExtents();

		*__output = tmp__output;
	}

	void ScriptReflectionProbe::InternalSetExtents(ScriptReflectionProbe* thisPtr, TVector3<float>* extents)
	{
		thisPtr->GetHandle()->SetExtents(*extents);
	}

	MonoObject* ScriptReflectionProbe::InternalGetCustomTexture(ScriptReflectionProbe* thisPtr)
	{
		ResourceHandle<Texture> tmp__output;
		tmp__output = thisPtr->GetHandle()->GetCustomTexture();

		MonoObject* __output;
		ScriptRRefBase* script__output;
		script__output = ScriptResourceManager::Instance().GetScriptRRef(tmp__output);
		if(script__output != nullptr)
			__output = script__output->GetManagedInstance();
		else
			__output = nullptr;

		return __output;
	}

	void ScriptReflectionProbe::InternalSetCustomTexture(ScriptReflectionProbe* thisPtr, MonoObject* texture)
	{
		ResourceHandle<Texture> tmptexture;
		ScriptRRefBase* scripttexture;
		scripttexture = ScriptRRefBase::ToNative(texture);
		if(scripttexture != nullptr)
			tmptexture = B3DStaticResourceCast<Texture>(scripttexture->GetHandle());
		thisPtr->GetHandle()->SetCustomTexture(tmptexture);
	}

	void ScriptReflectionProbe::InternalCapture(ScriptReflectionProbe* thisPtr)
	{
		thisPtr->GetHandle()->Capture();
	}
}
