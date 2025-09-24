//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptResourceWrapper.h"
#include "../../../Foundation/Utility/Math/B3DSize2.h"
#include "../../../Foundation/Core/Image/B3DSpriteImage.h"
#include "../../../Foundation/Core/Image/B3DSpriteImage.h"

namespace b3d { class SpriteImage; }
namespace b3d
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
		~ScriptSpriteImage();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static MonoObject* InternalGetRef(ScriptSpriteImageWrapperBase* self);

		static void InternalGetAnimationFrameSize(ScriptSpriteImageWrapperBase* self, TSize2<uint32_t>* __output);
		static void InternalSetAnimation(ScriptSpriteImageWrapperBase* self, SpriteSheetGridAnimation* animation);
		static void InternalGetAnimation(ScriptSpriteImageWrapperBase* self, SpriteSheetGridAnimation* __output);
		static void InternalSetAnimationPlayback(ScriptSpriteImageWrapperBase* self, SpriteAnimationPlayback playback);
		static SpriteAnimationPlayback InternalGetAnimationPlayback(ScriptSpriteImageWrapperBase* self);
	};
}
