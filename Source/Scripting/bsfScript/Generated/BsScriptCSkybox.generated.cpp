//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptCSkybox.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Components/BsCSkybox.h"
#include "BsScriptResourceManager.h"
#include "Wrappers/BsScriptRRefBase.h"
#include "../../../Foundation/bsfCore/Image/BsTexture.h"

namespace bs
{
	ScriptSkybox::ScriptSkybox(MonoObject* managedInstance, const GameObjectHandle<CSkybox>& value)
		:TScriptComponent(managedInstance, value)
	{
	}

	void ScriptSkybox::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_GetTexture", (void*)&ScriptSkybox::InternalGetTexture);
		metaData.ScriptClass->AddInternalCall("Internal_SetTexture", (void*)&ScriptSkybox::InternalSetTexture);
		metaData.ScriptClass->AddInternalCall("Internal_SetBrightness", (void*)&ScriptSkybox::InternalSetBrightness);
		metaData.ScriptClass->AddInternalCall("Internal_GetBrightness", (void*)&ScriptSkybox::InternalGetBrightness);

	}

	MonoObject* ScriptSkybox::InternalGetTexture(ScriptSkybox* thisPtr)
	{
		TResourceHandle<Texture> tmp__output;
		tmp__output = thisPtr->GetHandle()->GetTexture();

		MonoObject* __output;
		ScriptRRefBase* script__output;
		script__output = ScriptResourceManager::Instance().GetScriptRRef(tmp__output);
		if(script__output != nullptr)
			__output = script__output->GetManagedInstance();
		else
			__output = nullptr;

		return __output;
	}

	void ScriptSkybox::InternalSetTexture(ScriptSkybox* thisPtr, MonoObject* texture)
	{
		TResourceHandle<Texture> tmptexture;
		ScriptRRefBase* scriptObjectWrappertexture;
		scriptObjectWrappertexture = ScriptRRefBase::ToNative(texture);
		if(scriptObjectWrappertexture != nullptr)
			tmptexture = B3DStaticResourceCast<Texture>(scriptObjectWrappertexture->GetHandle());
		thisPtr->GetHandle()->SetTexture(tmptexture);
	}

	void ScriptSkybox::InternalSetBrightness(ScriptSkybox* thisPtr, float brightness)
	{
		thisPtr->GetHandle()->SetBrightness(brightness);
	}

	float ScriptSkybox::InternalGetBrightness(ScriptSkybox* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetHandle()->GetBrightness();

		float __output;
		__output = tmp__output;

		return __output;
	}
}
