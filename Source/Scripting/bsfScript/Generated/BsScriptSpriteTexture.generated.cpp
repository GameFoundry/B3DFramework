//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptSpriteTexture.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Image/BsSpriteTexture.h"
#include "BsScriptResourceManager.h"
#include "Wrappers/BsScriptRRefBase.h"
#include "../../../Foundation/bsfCore/Image/BsTexture.h"
#include "BsScriptTArea2.generated.h"
#include "../../../Foundation/bsfCore/Image/BsSpriteTexture.h"
#include "BsScriptSpriteTextureCreateInformation.generated.h"

namespace bs
{
	ScriptSpriteTexture::ScriptSpriteTexture(const TResourceHandle<SpriteTexture>& nativeObject)
		:TScriptResourceWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptSpriteTexture::~ScriptSpriteTexture()
	{
		UnregisterEvents();
	}

	void ScriptSpriteTexture::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetRef", (void*)&ScriptSpriteTexture::InternalGetRef);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetAtlasTexture", (void*)&ScriptSpriteTexture::InternalGetAtlasTexture);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetUVRange", (void*)&ScriptSpriteTexture::InternalGetUVRange);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create", (void*)&ScriptSpriteTexture::InternalCreate);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create0", (void*)&ScriptSpriteTexture::InternalCreate0);

	}

	MonoObject* ScriptSpriteTexture::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	MonoObject* ScriptSpriteTexture::InternalGetRef(ScriptSpriteTexture* self)
	{
		return self->GetOrCreateResourceReference();
	}

	MonoObject* ScriptSpriteTexture::InternalGetAtlasTexture(ScriptSpriteTexture* self)
	{
		TResourceHandle<Texture> tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<SpriteTexture*>(self->GetNativeObject())->GetAtlasTexture();

		MonoObject* __output;
		ScriptRRefBase* script__output;
		script__output = ScriptResourceManager::Instance().GetScriptRRef(tmp__output);
		if(script__output != nullptr)
			__output = script__output->GetScriptObject();
		else
			__output = nullptr;

		return __output;
	}

	void ScriptSpriteTexture::InternalGetUVRange(ScriptSpriteTexture* self, TArea2<float, float>* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		TArea2<float, float> tmp__output;
		tmp__output = static_cast<SpriteTexture*>(self->GetNativeObject())->GetUVRange();

		*__output = tmp__output;
	}

	void ScriptSpriteTexture::InternalCreate(MonoObject* scriptObject, MonoObject* texture)
	{
		TResourceHandle<Texture> tmptexture;
		ScriptRRefBase* scriptObjectWrappertexture;
		scriptObjectWrappertexture = ScriptRRefBase::GetScriptObjectWrapper(texture);
		if(scriptObjectWrappertexture != nullptr)
			tmptexture = B3DStaticResourceCast<Texture>(scriptObjectWrappertexture->GetNativeObject());
		TResourceHandle<SpriteTexture> nativeObject = SpriteTexture::Create(tmptexture);
		ScriptObjectWrapper::Create<ScriptSpriteTexture>(nativeObject, scriptObject);
	}

	void ScriptSpriteTexture::InternalCreate0(MonoObject* scriptObject, __SpriteTextureCreateInformationInterop* createInformation)
	{
		SpriteTextureCreateInformation tmpcreateInformation;
		tmpcreateInformation = ScriptSpriteTextureCreateInformation::FromInterop(*createInformation);
		TResourceHandle<SpriteTexture> nativeObject = SpriteTexture::Create(tmpcreateInformation);
		ScriptObjectWrapper::Create<ScriptSpriteTexture>(nativeObject, scriptObject);
	}
}
