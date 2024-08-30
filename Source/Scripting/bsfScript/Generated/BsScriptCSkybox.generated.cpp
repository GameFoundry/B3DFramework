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
	ScriptSkybox::ScriptSkybox(const GameObjectHandle<CSkybox>& nativeObject, MonoObject* scriptObject)
		:TScriptGameObjectWrapper(nativeObject, scriptObject)
	{
		RegisterEvents();
	}

	void ScriptSkybox::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetTexture", (void*)&ScriptSkybox::InternalGetTexture);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetTexture", (void*)&ScriptSkybox::InternalSetTexture);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetBrightness", (void*)&ScriptSkybox::InternalSetBrightness);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetBrightness", (void*)&ScriptSkybox::InternalGetBrightness);

	}

	MonoObject* ScriptSkybox::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	MonoObject* ScriptSkybox::InternalGetTexture(ScriptSkybox* self)
	{
		TResourceHandle<Texture> tmp__output;
		tmp__output = static_cast<CSkybox*>(self->GetNativeObject())->GetTexture();

		MonoObject* __output;
		ScriptRRefBase* script__output;
		script__output = ScriptResourceManager::Instance().GetScriptRRef(tmp__output);
		if(script__output != nullptr)
			__output = script__output->GetManagedInstance();
		else
			__output = nullptr;

		return __output;
	}

	void ScriptSkybox::InternalSetTexture(ScriptSkybox* self, MonoObject* texture)
	{
		TResourceHandle<Texture> tmptexture;
		ScriptRRefBase* scriptObjectWrappertexture;
		scriptObjectWrappertexture = ScriptRRefBase::ToNative(texture);
		if(scriptObjectWrappertexture != nullptr)
			tmptexture = B3DStaticResourceCast<Texture>(scriptObjectWrappertexture->GetHandle());
		static_cast<CSkybox*>(self->GetNativeObject())->SetTexture(tmptexture);
	}

	void ScriptSkybox::InternalSetBrightness(ScriptSkybox* self, float brightness)
	{
		static_cast<CSkybox*>(self->GetNativeObject())->SetBrightness(brightness);
	}

	float ScriptSkybox::InternalGetBrightness(ScriptSkybox* self)
	{
		float tmp__output;
		tmp__output = static_cast<CSkybox*>(self->GetNativeObject())->GetBrightness();

		float __output;
		__output = tmp__output;

		return __output;
	}
}
