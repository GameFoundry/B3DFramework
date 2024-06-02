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
	ScriptSpriteTexture::ScriptSpriteTexture(MonoObject* managedInstance, const TResourceHandle<SpriteTexture>& value)
		:TScriptResource(managedInstance, value)
	{
	}

	void ScriptSpriteTexture::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_GetRef", (void*)&ScriptSpriteTexture::InternalGetRef);
		metaData.ScriptClass->AddInternalCall("Internal_SetAtlasTexture", (void*)&ScriptSpriteTexture::InternalSetAtlasTexture);
		metaData.ScriptClass->AddInternalCall("Internal_Create", (void*)&ScriptSpriteTexture::InternalCreate);
		metaData.ScriptClass->AddInternalCall("Internal_Create0", (void*)&ScriptSpriteTexture::InternalCreate0);

	}

	 MonoObject*ScriptSpriteTexture::CreateInstance()
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		return metaData.ScriptClass->CreateInstance("bool", ctorParams);
	}
	MonoObject* ScriptSpriteTexture::InternalGetRef(ScriptSpriteTexture* thisPtr)
	{
		return thisPtr->GetRRef();
	}

	void ScriptSpriteTexture::InternalSetAtlasTexture(ScriptSpriteTexture* thisPtr, MonoObject* texture)
	{
		TResourceHandle<Texture> tmptexture;
		ScriptRRefBase* scripttexture;
		scripttexture = ScriptRRefBase::ToNative(texture);
		if(scripttexture != nullptr)
			tmptexture = B3DStaticResourceCast<Texture>(scripttexture->GetHandle());
		thisPtr->GetHandle()->SetAtlasTexture(tmptexture);
	}

	void ScriptSpriteTexture::InternalCreate(MonoObject* managedInstance, MonoObject* texture)
	{
		TResourceHandle<Texture> tmptexture;
		ScriptRRefBase* scripttexture;
		scripttexture = ScriptRRefBase::ToNative(texture);
		if(scripttexture != nullptr)
			tmptexture = B3DStaticResourceCast<Texture>(scripttexture->GetHandle());
		TResourceHandle<SpriteTexture> instance = SpriteTexture::Create(tmptexture);
		ScriptResourceManager::Instance().CreateBuiltinScriptResource(instance, managedInstance);
	}

	void ScriptSpriteTexture::InternalCreate0(MonoObject* managedInstance, __SpriteTextureCreateInformationInterop* createInformation)
	{
		SpriteTextureCreateInformation tmpcreateInformation;
		tmpcreateInformation = ScriptSpriteTextureCreateInformation::FromInterop(*createInformation);
		TResourceHandle<SpriteTexture> instance = SpriteTexture::Create(tmpcreateInformation);
		ScriptResourceManager::Instance().CreateBuiltinScriptResource(instance, managedInstance);
	}
}
