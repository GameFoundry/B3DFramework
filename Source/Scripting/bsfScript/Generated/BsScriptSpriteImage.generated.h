//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptResource.h"
#include "Utility/BsUtil.h"
#include "../../../Foundation/bsfCore/Image/BsSpriteImage.h"
#include "Math/BsRect2.h"
#include "../../../Foundation/bsfCore/Image/BsSpriteImage.h"

namespace bs { class SpriteImage; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptSpriteImageBase : public ScriptResourceBase
	{
	public:
		ScriptSpriteImageBase(MonoObject* instance);
		virtual ~ScriptSpriteImageBase() {}
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptSpriteImage : public TScriptResource<ScriptSpriteImage, SpriteImage, ScriptSpriteImageBase>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "SpriteImage")

		ScriptSpriteImage(MonoObject* managedInstance, const TResourceHandle<SpriteImage>& value);

		static MonoObject* CreateInstance();

	private:
		static MonoObject* InternalGetRef(ScriptSpriteImageBase* thisPtr);

		static void InternalGetSize(ScriptSpriteImageBase* thisPtr, TSize2<uint32_t>* __output);
		static void InternalGetAnimationFrameSize(ScriptSpriteImageBase* thisPtr, TSize2<uint32_t>* __output);
		static MonoObject* InternalGetAtlasTexture(ScriptSpriteImageBase* thisPtr);
		static void InternalSetUVRange(ScriptSpriteImageBase* thisPtr, Rect2* uvRange);
		static void InternalGetUVRange(ScriptSpriteImageBase* thisPtr, Rect2* __output);
		static void InternalSetAnimation(ScriptSpriteImageBase* thisPtr, SpriteSheetGridAnimation* animation);
		static void InternalGetAnimation(ScriptSpriteImageBase* thisPtr, SpriteSheetGridAnimation* __output);
		static void InternalSetAnimationPlayback(ScriptSpriteImageBase* thisPtr, SpriteAnimationPlayback playback);
		static SpriteAnimationPlayback InternalGetAnimationPlayback(ScriptSpriteImageBase* thisPtr);
	};
}
