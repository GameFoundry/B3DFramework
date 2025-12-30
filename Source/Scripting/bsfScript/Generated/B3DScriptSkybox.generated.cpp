//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptSkybox.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "../../../Foundation/Core/Components/B3DSkybox.h"
#include "B3DScriptResourceManager.h"
#include "Wrappers/B3DScriptRRefBase.h"
#include "../../../Foundation/Core/Image/B3DTexture.h"

namespace b3d
{
	ScriptSkybox::ScriptSkybox(const TGameObjectHandle<Skybox>& nativeObject)
		:TScriptGameObjectWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptSkybox::~ScriptSkybox()
	{
		UnregisterEvents();
	}

	void ScriptSkybox::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetTexture", (void*)&ScriptSkybox::InternalSetTexture);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetBrightness", (void*)&ScriptSkybox::InternalSetBrightness);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetBrightness", (void*)&ScriptSkybox::InternalGetBrightness);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetTexture", (void*)&ScriptSkybox::InternalGetTexture);

	}

	MonoObject* ScriptSkybox::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	void ScriptSkybox::InternalSetTexture(ScriptSkybox* self, MonoObject* texture)
	{
		if(!self->IsNativeObjectValid())
			return;

		TResourceHandle<Texture> tmptexture;
		ScriptRRefBase* scriptObjectWrappertexture;
		scriptObjectWrappertexture = ScriptRRefBase::GetScriptObjectWrapper(texture);
		if(scriptObjectWrappertexture != nullptr)
			tmptexture = B3DStaticResourceCast<Texture>(scriptObjectWrappertexture->GetNativeObject());
		static_cast<Skybox*>(self->GetNativeObject())->SetTexture(tmptexture);
	}

	void ScriptSkybox::InternalSetBrightness(ScriptSkybox* self, float brightness)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<Skybox*>(self->GetNativeObject())->SetBrightness(brightness);
	}

	float ScriptSkybox::InternalGetBrightness(ScriptSkybox* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<Skybox*>(self->GetNativeObject())->GetBrightness();

		float __output;
		__output = tmp__output;

		return __output;
	}

	MonoObject* ScriptSkybox::InternalGetTexture(ScriptSkybox* self)
	{
		TResourceHandle<Texture> tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<Skybox*>(self->GetNativeObject())->GetTexture();

		MonoObject* __output;
		ScriptRRefBase* script__output;
		script__output = ScriptResourceManager::Instance().GetScriptRRef(tmp__output);
		if(script__output != nullptr)
			__output = script__output->GetScriptObject();
		else
			__output = nullptr;

		return __output;
	}
}
