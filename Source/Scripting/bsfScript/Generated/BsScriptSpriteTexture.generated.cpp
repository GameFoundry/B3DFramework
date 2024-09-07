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
#include "../../../Foundation/bsfCore/Image/BsSpriteTexture.h"
#include "BsScriptSpriteTextureCreateInformation.generated.h"

namespace bs
{
	ScriptSpriteTexture::ScriptSpriteTexture(const TResourceHandle<SpriteTexture>& nativeObject)
		:TScriptResourceWrapper(nativeObject)
	{
		RegisterEvents();
	}

	void ScriptSpriteTexture::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetRef", (void*)&ScriptSpriteTexture::InternalGetRef);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetAtlasTexture", (void*)&ScriptSpriteTexture::InternalSetAtlasTexture);
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

	void ScriptSpriteTexture::InternalSetAtlasTexture(ScriptSpriteTexture* self, MonoObject* texture)
	{
		TResourceHandle<Texture> tmptexture;
		ScriptRRefBase* scriptObjectWrappertexture;
		scriptObjectWrappertexture = ScriptRRefBase::GetScriptObjectWrapper(texture);
		if(scriptObjectWrappertexture != nullptr)
			tmptexture = B3DStaticResourceCast<Texture>(scriptObjectWrappertexture->GetNativeObject());
		static_cast<SpriteTexture*>(self->GetNativeObject())->SetAtlasTexture(tmptexture);
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
