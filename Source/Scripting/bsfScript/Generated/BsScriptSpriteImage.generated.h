//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptResourceWrapper.h"
#include "Utility/BsUtil.h"
#include "../../../Foundation/bsfCore/Image/BsSpriteImage.h"
#include "Math/BsRect2.h"
#include "../../../Foundation/bsfCore/Image/BsSpriteImage.h"

namespace bs { class SpriteImage; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptSpriteImageWrapperBase : public ScriptResourceWrapper
	{
	public:
		using ScriptResourceWrapper::ScriptResourceWrapper;

	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptSpriteImage : public TScriptResourceWrapper<SpriteImage, ScriptSpriteImage, ScriptSpriteImageWrapperBase>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "SpriteImage")

		ScriptSpriteImage(const TResourceHandle<SpriteImage>& nativeObject);

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static MonoObject* InternalGetRef(ScriptSpriteImageWrapperBase* self);

		static void InternalGetSize(ScriptSpriteImageWrapperBase* self, TSize2<uint32_t>* __output);
		static void InternalGetAnimationFrameSize(ScriptSpriteImageWrapperBase* self, TSize2<uint32_t>* __output);
		static MonoObject* InternalGetAtlasTexture(ScriptSpriteImageWrapperBase* self);
		static void InternalSetUVRange(ScriptSpriteImageWrapperBase* self, Rect2* uvRange);
		static void InternalGetUVRange(ScriptSpriteImageWrapperBase* self, Rect2* __output);
		static void InternalSetAnimation(ScriptSpriteImageWrapperBase* self, SpriteSheetGridAnimation* animation);
		static void InternalGetAnimation(ScriptSpriteImageWrapperBase* self, SpriteSheetGridAnimation* __output);
		static void InternalSetAnimationPlayback(ScriptSpriteImageWrapperBase* self, SpriteAnimationPlayback playback);
		static SpriteAnimationPlayback InternalGetAnimationPlayback(ScriptSpriteImageWrapperBase* self);
	};
}
