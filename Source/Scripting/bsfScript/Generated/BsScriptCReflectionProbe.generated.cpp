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
	ScriptCReflectionProbe::ScriptCReflectionProbe(MonoObject* managedInstance, const GameObjectHandle<CReflectionProbe>& value)
		:TScriptComponent(managedInstance, value)
	{
	}

	void ScriptCReflectionProbe::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_GetType", (void*)&ScriptCReflectionProbe::InternalGetType);
		metaData.ScriptClass->AddInternalCall("Internal_SetType", (void*)&ScriptCReflectionProbe::InternalSetType);
		metaData.ScriptClass->AddInternalCall("Internal_GetRadius", (void*)&ScriptCReflectionProbe::InternalGetRadius);
		metaData.ScriptClass->AddInternalCall("Internal_SetRadius", (void*)&ScriptCReflectionProbe::InternalSetRadius);
		metaData.ScriptClass->AddInternalCall("Internal_GetExtents", (void*)&ScriptCReflectionProbe::InternalGetExtents);
		metaData.ScriptClass->AddInternalCall("Internal_SetExtents", (void*)&ScriptCReflectionProbe::InternalSetExtents);
		metaData.ScriptClass->AddInternalCall("Internal_GetCustomTexture", (void*)&ScriptCReflectionProbe::InternalGetCustomTexture);
		metaData.ScriptClass->AddInternalCall("Internal_SetCustomTexture", (void*)&ScriptCReflectionProbe::InternalSetCustomTexture);
		metaData.ScriptClass->AddInternalCall("Internal_Capture", (void*)&ScriptCReflectionProbe::InternalCapture);

	}

	ReflectionProbeType ScriptCReflectionProbe::InternalGetType(ScriptCReflectionProbe* thisPtr)
	{
		ReflectionProbeType tmp__output;
		tmp__output = thisPtr->GetHandle()->GetType();

		ReflectionProbeType __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCReflectionProbe::InternalSetType(ScriptCReflectionProbe* thisPtr, ReflectionProbeType type)
	{
		thisPtr->GetHandle()->SetType(type);
	}

	float ScriptCReflectionProbe::InternalGetRadius(ScriptCReflectionProbe* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetHandle()->GetRadius();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCReflectionProbe::InternalSetRadius(ScriptCReflectionProbe* thisPtr, float radius)
	{
		thisPtr->GetHandle()->SetRadius(radius);
	}

	void ScriptCReflectionProbe::InternalGetExtents(ScriptCReflectionProbe* thisPtr, Vector3* __output)
	{
		Vector3 tmp__output;
		tmp__output = thisPtr->GetHandle()->GetExtents();

		*__output = tmp__output;
	}

	void ScriptCReflectionProbe::InternalSetExtents(ScriptCReflectionProbe* thisPtr, Vector3* extents)
	{
		thisPtr->GetHandle()->SetExtents(*extents);
	}

	MonoObject* ScriptCReflectionProbe::InternalGetCustomTexture(ScriptCReflectionProbe* thisPtr)
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

	void ScriptCReflectionProbe::InternalSetCustomTexture(ScriptCReflectionProbe* thisPtr, MonoObject* texture)
	{
		ResourceHandle<Texture> tmptexture;
		ScriptRRefBase* scripttexture;
		scripttexture = ScriptRRefBase::ToNative(texture);
		if(scripttexture != nullptr)
			tmptexture = B3DStaticResourceCast<Texture>(scripttexture->GetHandle());
		thisPtr->GetHandle()->SetCustomTexture(tmptexture);
	}

	void ScriptCReflectionProbe::InternalCapture(ScriptCReflectionProbe* thisPtr)
	{
		thisPtr->GetHandle()->Capture();
	}
}
