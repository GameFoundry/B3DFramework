//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptSpriteImage.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Image/BsSpriteImage.h"
#include "BsScriptResourceManager.h"
#include "Wrappers/BsScriptRRefBase.h"
#include "Wrappers/BsScriptSize.h"
#include "../../../Foundation/bsfCore/Image/BsTexture.h"
#include "BsScriptSpriteSheetGridAnimation.generated.h"

namespace bs
{
	ScriptSpriteImageBase::ScriptSpriteImageBase(MonoObject* managedInstance)
		:ScriptResourceBase(managedInstance)
	 { }

	ScriptSpriteImage::ScriptSpriteImage(MonoObject* managedInstance, const ResourceHandle<SpriteImage>& value)
		:TScriptResource(managedInstance, value)
	{
	}

	void ScriptSpriteImage::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_GetRef", (void*)&ScriptSpriteImage::InternalGetRef);
		metaData.ScriptClass->AddInternalCall("Internal_GetSize", (void*)&ScriptSpriteImage::InternalGetSize);
		metaData.ScriptClass->AddInternalCall("Internal_GetAnimationFrameSize", (void*)&ScriptSpriteImage::InternalGetAnimationFrameSize);
		metaData.ScriptClass->AddInternalCall("Internal_GetAtlasTexture", (void*)&ScriptSpriteImage::InternalGetAtlasTexture);
		metaData.ScriptClass->AddInternalCall("Internal_SetUVRange", (void*)&ScriptSpriteImage::InternalSetUVRange);
		metaData.ScriptClass->AddInternalCall("Internal_GetUVRange", (void*)&ScriptSpriteImage::InternalGetUVRange);
		metaData.ScriptClass->AddInternalCall("Internal_SetAnimation", (void*)&ScriptSpriteImage::InternalSetAnimation);
		metaData.ScriptClass->AddInternalCall("Internal_GetAnimation", (void*)&ScriptSpriteImage::InternalGetAnimation);
		metaData.ScriptClass->AddInternalCall("Internal_SetAnimationPlayback", (void*)&ScriptSpriteImage::InternalSetAnimationPlayback);
		metaData.ScriptClass->AddInternalCall("Internal_GetAnimationPlayback", (void*)&ScriptSpriteImage::InternalGetAnimationPlayback);

	}

	 MonoObject*ScriptSpriteImage::CreateInstance()
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		return metaData.ScriptClass->CreateInstance("bool", ctorParams);
	}
	MonoObject* ScriptSpriteImage::InternalGetRef(ScriptSpriteImageBase* thisPtr)
	{
		return thisPtr->GetRRef(thisPtr->GetGenericHandle(), SpriteImage::GetRttiStatic()->GetRttiId());
	}

	void ScriptSpriteImage::InternalGetSize(ScriptSpriteImageBase* thisPtr, TSize2<uint32_t>* __output)
	{
		TSize2<uint32_t> tmp__output;
		tmp__output = B3DStaticResourceCast<SpriteImage>(thisPtr->GetGenericHandle())->GetSize();

		*__output = tmp__output;
	}

	void ScriptSpriteImage::InternalGetAnimationFrameSize(ScriptSpriteImageBase* thisPtr, TSize2<uint32_t>* __output)
	{
		TSize2<uint32_t> tmp__output;
		tmp__output = B3DStaticResourceCast<SpriteImage>(thisPtr->GetGenericHandle())->GetAnimationFrameSize();

		*__output = tmp__output;
	}

	MonoObject* ScriptSpriteImage::InternalGetAtlasTexture(ScriptSpriteImageBase* thisPtr)
	{
		ResourceHandle<Texture> tmp__output;
		tmp__output = B3DStaticResourceCast<SpriteImage>(thisPtr->GetGenericHandle())->GetAtlasTexture();

		MonoObject* __output;
		ScriptRRefBase* script__output;
		script__output = ScriptResourceManager::Instance().GetScriptRRef(tmp__output);
		if(script__output != nullptr)
			__output = script__output->GetManagedInstance();
		else
			__output = nullptr;

		return __output;
	}

	void ScriptSpriteImage::InternalSetUVRange(ScriptSpriteImageBase* thisPtr, Rect2* uvRange)
	{
		B3DStaticResourceCast<SpriteImage>(thisPtr->GetGenericHandle())->SetUVRange(*uvRange);
	}

	void ScriptSpriteImage::InternalGetUVRange(ScriptSpriteImageBase* thisPtr, Rect2* __output)
	{
		Rect2 tmp__output;
		tmp__output = B3DStaticResourceCast<SpriteImage>(thisPtr->GetGenericHandle())->GetUVRange();

		*__output = tmp__output;
	}

	void ScriptSpriteImage::InternalSetAnimation(ScriptSpriteImageBase* thisPtr, SpriteSheetGridAnimation* animation)
	{
		B3DStaticResourceCast<SpriteImage>(thisPtr->GetGenericHandle())->SetAnimation(*animation);
	}

	void ScriptSpriteImage::InternalGetAnimation(ScriptSpriteImageBase* thisPtr, SpriteSheetGridAnimation* __output)
	{
		SpriteSheetGridAnimation tmp__output;
		tmp__output = B3DStaticResourceCast<SpriteImage>(thisPtr->GetGenericHandle())->GetAnimation();

		*__output = tmp__output;
	}

	void ScriptSpriteImage::InternalSetAnimationPlayback(ScriptSpriteImageBase* thisPtr, SpriteAnimationPlayback playback)
	{
		B3DStaticResourceCast<SpriteImage>(thisPtr->GetGenericHandle())->SetAnimationPlayback(playback);
	}

	SpriteAnimationPlayback ScriptSpriteImage::InternalGetAnimationPlayback(ScriptSpriteImageBase* thisPtr)
	{
		SpriteAnimationPlayback tmp__output;
		tmp__output = B3DStaticResourceCast<SpriteImage>(thisPtr->GetGenericHandle())->GetAnimationPlayback();

		SpriteAnimationPlayback __output;
		__output = tmp__output;

		return __output;
	}
}
