//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptSpriteTexture.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Image/BsSpriteTexture.h"
#include "BsScriptResourceManager.h"
#include "Wrappers/BsScriptRRefBase.h"
#include "../../../Foundation/bsfCore/Image/BsTexture.h"
#include "Wrappers/BsScriptVector.h"
#include "BsScriptSpriteSheetGridAnimation.generated.h"
#include "../../../Foundation/bsfCore/Image/BsSpriteTexture.h"

namespace bs
{
	ScriptSpriteTexture::ScriptSpriteTexture(MonoObject* managedInstance, const ResourceHandle<SpriteTexture>& value)
		:TScriptResource(managedInstance, value)
	{
	}

	void ScriptSpriteTexture::InitRuntimeData()
	{
		metaData.scriptClass->AddInternalCall("Internal_GetRef", (void*)&ScriptSpriteTexture::Internal_getRef);
		metaData.scriptClass->AddInternalCall("Internal_setTexture", (void*)&ScriptSpriteTexture::Internal_setTexture);
		metaData.scriptClass->AddInternalCall("Internal_getTexture", (void*)&ScriptSpriteTexture::Internal_getTexture);
		metaData.scriptClass->AddInternalCall("Internal_getWidth", (void*)&ScriptSpriteTexture::Internal_getWidth);
		metaData.scriptClass->AddInternalCall("Internal_getHeight", (void*)&ScriptSpriteTexture::Internal_getHeight);
		metaData.scriptClass->AddInternalCall("Internal_getFrameWidth", (void*)&ScriptSpriteTexture::Internal_getFrameWidth);
		metaData.scriptClass->AddInternalCall("Internal_getFrameHeight", (void*)&ScriptSpriteTexture::Internal_getFrameHeight);
		metaData.scriptClass->AddInternalCall("Internal_setOffset", (void*)&ScriptSpriteTexture::Internal_setOffset);
		metaData.scriptClass->AddInternalCall("Internal_getOffset", (void*)&ScriptSpriteTexture::Internal_getOffset);
		metaData.scriptClass->AddInternalCall("Internal_setScale", (void*)&ScriptSpriteTexture::Internal_setScale);
		metaData.scriptClass->AddInternalCall("Internal_getScale", (void*)&ScriptSpriteTexture::Internal_getScale);
		metaData.scriptClass->AddInternalCall("Internal_setAnimation", (void*)&ScriptSpriteTexture::Internal_setAnimation);
		metaData.scriptClass->AddInternalCall("Internal_getAnimation", (void*)&ScriptSpriteTexture::Internal_getAnimation);
		metaData.scriptClass->AddInternalCall("Internal_setAnimationPlayback", (void*)&ScriptSpriteTexture::Internal_setAnimationPlayback);
		metaData.scriptClass->AddInternalCall("Internal_getAnimationPlayback", (void*)&ScriptSpriteTexture::Internal_getAnimationPlayback);
		metaData.scriptClass->AddInternalCall("Internal_create", (void*)&ScriptSpriteTexture::Internal_create);
		metaData.scriptClass->AddInternalCall("Internal_create0", (void*)&ScriptSpriteTexture::Internal_create0);

	}

	 MonoObject*ScriptSpriteTexture::createInstance()
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		return metaData.scriptClass->CreateInstance("bool", ctorParams);
	}
	MonoObject* ScriptSpriteTexture::Internal_getRef(ScriptSpriteTexture* thisPtr)
	{
		return thisPtr->GetRRef();
	}

	void ScriptSpriteTexture::Internal_setTexture(ScriptSpriteTexture* thisPtr, MonoObject* texture)
	{
		ResourceHandle<Texture> tmptexture;
		ScriptRRefBase* scripttexture;
		scripttexture = ScriptRRefBase::toNative(texture);
		if(scripttexture != nullptr)
			tmptexture = static_resource_cast<Texture>(scripttexture->GetHandle());
		thisPtr->GetHandle()->setTexture(tmptexture);
	}

	MonoObject* ScriptSpriteTexture::Internal_getTexture(ScriptSpriteTexture* thisPtr)
	{
		ResourceHandle<Texture> tmp__output;
		tmp__output = thisPtr->GetHandle()->getTexture();

		MonoObject* __output;
		ScriptRRefBase* script__output;
		script__output = ScriptResourceManager::instance().GetScriptRRef(tmp__output);
		if(script__output != nullptr)
			__output = script__output->GetManagedInstance();
		else
			__output = nullptr;

		return __output;
	}

	uint32_t ScriptSpriteTexture::Internal_getWidth(ScriptSpriteTexture* thisPtr)
	{
		uint32_t tmp__output;
		tmp__output = thisPtr->GetHandle()->getWidth();

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	uint32_t ScriptSpriteTexture::Internal_getHeight(ScriptSpriteTexture* thisPtr)
	{
		uint32_t tmp__output;
		tmp__output = thisPtr->GetHandle()->getHeight();

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	uint32_t ScriptSpriteTexture::Internal_getFrameWidth(ScriptSpriteTexture* thisPtr)
	{
		uint32_t tmp__output;
		tmp__output = thisPtr->GetHandle()->getFrameWidth();

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	uint32_t ScriptSpriteTexture::Internal_getFrameHeight(ScriptSpriteTexture* thisPtr)
	{
		uint32_t tmp__output;
		tmp__output = thisPtr->GetHandle()->getFrameHeight();

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptSpriteTexture::Internal_setOffset(ScriptSpriteTexture* thisPtr, Vector2* offset)
	{
		thisPtr->GetHandle()->setOffset(*offset);
	}

	void ScriptSpriteTexture::Internal_getOffset(ScriptSpriteTexture* thisPtr, Vector2* __output)
	{
		Vector2 tmp__output;
		tmp__output = thisPtr->GetHandle()->getOffset();

		*__output = tmp__output;
	}

	void ScriptSpriteTexture::Internal_setScale(ScriptSpriteTexture* thisPtr, Vector2* scale)
	{
		thisPtr->GetHandle()->setScale(*scale);
	}

	void ScriptSpriteTexture::Internal_getScale(ScriptSpriteTexture* thisPtr, Vector2* __output)
	{
		Vector2 tmp__output;
		tmp__output = thisPtr->GetHandle()->getScale();

		*__output = tmp__output;
	}

	void ScriptSpriteTexture::Internal_setAnimation(ScriptSpriteTexture* thisPtr, SpriteSheetGridAnimation* anim)
	{
		thisPtr->GetHandle()->setAnimation(*anim);
	}

	void ScriptSpriteTexture::Internal_getAnimation(ScriptSpriteTexture* thisPtr, SpriteSheetGridAnimation* __output)
	{
		SpriteSheetGridAnimation tmp__output;
		tmp__output = thisPtr->GetHandle()->getAnimation();

		*__output = tmp__output;
	}

	void ScriptSpriteTexture::Internal_setAnimationPlayback(ScriptSpriteTexture* thisPtr, SpriteAnimationPlayback playback)
	{
		thisPtr->GetHandle()->setAnimationPlayback(playback);
	}

	SpriteAnimationPlayback ScriptSpriteTexture::Internal_getAnimationPlayback(ScriptSpriteTexture* thisPtr)
	{
		SpriteAnimationPlayback tmp__output;
		tmp__output = thisPtr->GetHandle()->getAnimationPlayback();

		SpriteAnimationPlayback __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptSpriteTexture::Internal_create(MonoObject* managedInstance, MonoObject* texture)
	{
		ResourceHandle<Texture> tmptexture;
		ScriptRRefBase* scripttexture;
		scripttexture = ScriptRRefBase::toNative(texture);
		if(scripttexture != nullptr)
			tmptexture = static_resource_cast<Texture>(scripttexture->GetHandle());
		ResourceHandle<SpriteTexture> instance = SpriteTexture::create(tmptexture);
		ScriptResourceManager::instance().CreateBuiltinScriptResource(instance, managedInstance);
	}

	void ScriptSpriteTexture::Internal_create0(MonoObject* managedInstance, Vector2* uvOffset, Vector2* uvScale, MonoObject* texture)
	{
		ResourceHandle<Texture> tmptexture;
		ScriptRRefBase* scripttexture;
		scripttexture = ScriptRRefBase::toNative(texture);
		if(scripttexture != nullptr)
			tmptexture = static_resource_cast<Texture>(scripttexture->GetHandle());
		ResourceHandle<SpriteTexture> instance = SpriteTexture::create(*uvOffset, *uvScale, tmptexture);
		ScriptResourceManager::instance().CreateBuiltinScriptResource(instance, managedInstance);
	}
}
