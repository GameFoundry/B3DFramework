//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObjectWrapper.h"
#include "../../../Foundation/bsfCore/Image/BsSpriteGlyph.h"
#include "../../../Foundation/bsfCore/Image/BsSpriteImage.h"
#include "Math/BsArea2.h"
#include "../../../Foundation/bsfCore/Image/BsSpriteImage.h"

namespace bs
{
	struct __SpriteGlyphCreateInformationInterop
	{
		MonoObject* Font;
		uint32_t Glyph;
		float Size;
		Area2 UVRange;
		SpriteAnimationPlayback AnimationPlayback;
		SpriteSheetGridAnimation Animation;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptSpriteGlyphCreateInformation : public TScriptTypeDefinition<ScriptSpriteGlyphCreateInformation>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "SpriteGlyphCreateInformation")

		static MonoObject* Box(const __SpriteGlyphCreateInformationInterop& value);
		static __SpriteGlyphCreateInformationInterop Unbox(MonoObject* value);
		static SpriteGlyphCreateInformation FromInterop(const __SpriteGlyphCreateInformationInterop& value);
		static __SpriteGlyphCreateInformationInterop ToInterop(const SpriteGlyphCreateInformation& value);

	private:
		ScriptSpriteGlyphCreateInformation();

	};
}
