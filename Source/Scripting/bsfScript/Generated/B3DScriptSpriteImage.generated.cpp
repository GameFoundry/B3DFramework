//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptSpriteImage.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "../../../Foundation/Core/Image/B3DSpriteImage.h"
#include "B3DScriptTSize2.generated.h"
#include "B3DScriptSpriteSheetGridAnimation.generated.h"

namespace b3d
{
	ScriptSpriteImage::ScriptSpriteImage(const TResourceHandle<SpriteImage>& nativeObject)
		:TScriptResourceWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptSpriteImage::~ScriptSpriteImage()
	{
		UnregisterEvents();
	}

	void ScriptSpriteImage::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetRef", (void*)&ScriptSpriteImage::InternalGetRef);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetAnimationFrameSize", (void*)&ScriptSpriteImage::InternalGetAnimationFrameSize);
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

	void ScriptSpriteImage::InternalGetAnimationFrameSize(ScriptSpriteImageWrapperBase* self, TSize2<uint32_t>* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		TSize2<uint32_t> tmp__output;
		tmp__output = static_cast<SpriteImage*>(self->GetNativeObject())->GetAnimationFrameSize();

		*__output = tmp__output;
	}

	void ScriptSpriteImage::InternalSetAnimation(ScriptSpriteImageWrapperBase* self, SpriteSheetGridAnimation* animation)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<SpriteImage*>(self->GetNativeObject())->SetAnimation(*animation);
	}

	void ScriptSpriteImage::InternalGetAnimation(ScriptSpriteImageWrapperBase* self, SpriteSheetGridAnimation* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		SpriteSheetGridAnimation tmp__output;
		tmp__output = static_cast<SpriteImage*>(self->GetNativeObject())->GetAnimation();

		*__output = tmp__output;
	}

	void ScriptSpriteImage::InternalSetAnimationPlayback(ScriptSpriteImageWrapperBase* self, SpriteAnimationPlayback playback)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<SpriteImage*>(self->GetNativeObject())->SetAnimationPlayback(playback);
	}

	SpriteAnimationPlayback ScriptSpriteImage::InternalGetAnimationPlayback(ScriptSpriteImageWrapperBase* self)
	{
		SpriteAnimationPlayback tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<SpriteImage*>(self->GetNativeObject())->GetAnimationPlayback();

		SpriteAnimationPlayback __output;
		__output = tmp__output;

		return __output;
	}
}
