//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptResource.h"
#include "Math/BsVector2.h"
#include "../../../Foundation/bsfCore/Image/BsSpriteTexture.h"
#include "../../../Foundation/bsfCore/Image/BsSpriteTexture.h"

namespace bs
{
	class SpriteTexture;
}

namespace bs
{
	class BS_SCR_BE_EXPORT ScriptSpriteTexture : public TScriptResource<ScriptSpriteTexture, SpriteTexture>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "SpriteTexture")

		ScriptSpriteTexture(MonoObject* managedInstance, const ResourceHandle<SpriteTexture>& value);

		static MonoObject* CreateInstance();

	private:
		static MonoObject* InternalGetRef(ScriptSpriteTexture* thisPtr);

		static void InternalSetTexture(ScriptSpriteTexture* thisPtr, MonoObject* texture);
		static MonoObject* InternalGetTexture(ScriptSpriteTexture* thisPtr);
		static uint32_t InternalGetWidth(ScriptSpriteTexture* thisPtr);
		static uint32_t InternalGetHeight(ScriptSpriteTexture* thisPtr);
		static uint32_t InternalGetFrameWidth(ScriptSpriteTexture* thisPtr);
		static uint32_t InternalGetFrameHeight(ScriptSpriteTexture* thisPtr);
		static void InternalSetOffset(ScriptSpriteTexture* thisPtr, Vector2* offset);
		static void InternalGetOffset(ScriptSpriteTexture* thisPtr, Vector2* __output);
		static void InternalSetScale(ScriptSpriteTexture* thisPtr, Vector2* scale);
		static void InternalGetScale(ScriptSpriteTexture* thisPtr, Vector2* __output);
		static void InternalSetAnimation(ScriptSpriteTexture* thisPtr, SpriteSheetGridAnimation* anim);
		static void InternalGetAnimation(ScriptSpriteTexture* thisPtr, SpriteSheetGridAnimation* __output);
		static void InternalSetAnimationPlayback(ScriptSpriteTexture* thisPtr, SpriteAnimationPlayback playback);
		static SpriteAnimationPlayback InternalGetAnimationPlayback(ScriptSpriteTexture* thisPtr);
		static void InternalCreate(MonoObject* managedInstance, MonoObject* texture);
		static void InternalCreate0(MonoObject* managedInstance, Vector2* uvOffset, Vector2* uvScale, MonoObject* texture);
	};
} // namespace bs
