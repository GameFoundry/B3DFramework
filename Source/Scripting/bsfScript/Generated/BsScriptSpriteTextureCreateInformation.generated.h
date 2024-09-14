//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObjectWrapper.h"
#include "../../../Foundation/bsfCore/Image/BsSpriteTexture.h"
#include "../../../Foundation/bsfCore/Image/BsSpriteImage.h"
#include "Math/BsRect2.h"
#include "../../../Foundation/bsfCore/Image/BsSpriteImage.h"

namespace bs
{
	struct __SpriteTextureCreateInformationInterop
	{
		MonoObject* AtlasTexture;
		Rect2 UVRange;
		SpriteAnimationPlayback AnimationPlayback;
		SpriteSheetGridAnimation Animation;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptSpriteTextureCreateInformation : public TScriptStructWrapper<ScriptSpriteTextureCreateInformation>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "SpriteTextureCreateInformation")

		static MonoObject* Box(const __SpriteTextureCreateInformationInterop& value);
		static __SpriteTextureCreateInformationInterop Unbox(MonoObject* value);
		static SpriteTextureCreateInformation FromInterop(const __SpriteTextureCreateInformationInterop& value);
		static __SpriteTextureCreateInformationInterop ToInterop(const SpriteTextureCreateInformation& value);

	private:
		ScriptSpriteTextureCreateInformation();

	};
}
