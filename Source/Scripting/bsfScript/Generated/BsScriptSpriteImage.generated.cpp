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
	ScriptSpriteImage::ScriptSpriteImage(const TResourceHandle<SpriteImage>& nativeObject, MonoObject* scriptObject)
		:TScriptResourceWrapper(nativeObject, scriptObject)
	{
		RegisterEvents();
	}

	void ScriptSpriteImage::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetRef", (void*)&ScriptSpriteImage::InternalGetRef);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetSize", (void*)&ScriptSpriteImage::InternalGetSize);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetAnimationFrameSize", (void*)&ScriptSpriteImage::InternalGetAnimationFrameSize);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetAtlasTexture", (void*)&ScriptSpriteImage::InternalGetAtlasTexture);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetUVRange", (void*)&ScriptSpriteImage::InternalSetUVRange);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetUVRange", (void*)&ScriptSpriteImage::InternalGetUVRange);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetAnimation", (void*)&ScriptSpriteImage::InternalSetAnimation);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetAnimation", (void*)&ScriptSpriteImage::InternalGetAnimation);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetAnimationPlayback", (void*)&ScriptSpriteImage::InternalSetAnimationPlayback);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetAnimationPlayback", (void*)&ScriptSpriteImage::InternalGetAnimationPlayback);

	}

	MonoObject* ScriptSpriteImage::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	MonoObject* ScriptSpriteImage::InternalGetRef(ScriptSpriteImageWrapperBase* self)
	{
		return self->GetOrCreateResourceReference(self->GetBaseNativeObjectAsHandle(), SpriteImage::GetRttiStatic()->GetRttiId());
	}

	void ScriptSpriteImage::InternalGetSize(ScriptSpriteImageWrapperBase* self, TSize2<uint32_t>* __output)
	{
		TSize2<uint32_t> tmp__output;
		tmp__output = static_cast<SpriteImage*>(self->GetNativeObject())->GetSize();

		*__output = tmp__output;
	}

	void ScriptSpriteImage::InternalGetAnimationFrameSize(ScriptSpriteImageWrapperBase* self, TSize2<uint32_t>* __output)
	{
		TSize2<uint32_t> tmp__output;
		tmp__output = static_cast<SpriteImage*>(self->GetNativeObject())->GetAnimationFrameSize();

		*__output = tmp__output;
	}

	MonoObject* ScriptSpriteImage::InternalGetAtlasTexture(ScriptSpriteImageWrapperBase* self)
	{
		TResourceHandle<Texture> tmp__output;
		tmp__output = static_cast<SpriteImage*>(self->GetNativeObject())->GetAtlasTexture();

		MonoObject* __output;
		ScriptRRefBase* script__output;
		script__output = ScriptResourceManager::Instance().GetScriptRRef(tmp__output);
		if(script__output != nullptr)
			__output = script__output->GetScriptObject();
		else
			__output = nullptr;

		return __output;
	}

	void ScriptSpriteImage::InternalSetUVRange(ScriptSpriteImageWrapperBase* self, Rect2* uvRange)
	{
		static_cast<SpriteImage*>(self->GetNativeObject())->SetUVRange(*uvRange);
	}

	void ScriptSpriteImage::InternalGetUVRange(ScriptSpriteImageWrapperBase* self, Rect2* __output)
	{
		Rect2 tmp__output;
		tmp__output = static_cast<SpriteImage*>(self->GetNativeObject())->GetUVRange();

		*__output = tmp__output;
	}

	void ScriptSpriteImage::InternalSetAnimation(ScriptSpriteImageWrapperBase* self, SpriteSheetGridAnimation* animation)
	{
		static_cast<SpriteImage*>(self->GetNativeObject())->SetAnimation(*animation);
	}

	void ScriptSpriteImage::InternalGetAnimation(ScriptSpriteImageWrapperBase* self, SpriteSheetGridAnimation* __output)
	{
		SpriteSheetGridAnimation tmp__output;
		tmp__output = static_cast<SpriteImage*>(self->GetNativeObject())->GetAnimation();

		*__output = tmp__output;
	}

	void ScriptSpriteImage::InternalSetAnimationPlayback(ScriptSpriteImageWrapperBase* self, SpriteAnimationPlayback playback)
	{
		static_cast<SpriteImage*>(self->GetNativeObject())->SetAnimationPlayback(playback);
	}

	SpriteAnimationPlayback ScriptSpriteImage::InternalGetAnimationPlayback(ScriptSpriteImageWrapperBase* self)
	{
		SpriteAnimationPlayback tmp__output;
		tmp__output = static_cast<SpriteImage*>(self->GetNativeObject())->GetAnimationPlayback();

		SpriteAnimationPlayback __output;
		__output = tmp__output;

		return __output;
	}
}
