//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptObjectWrapper.h"
#include "../../../Foundation/Core/Image/B3DSpriteImage.h"
#include "../../../Foundation/Core/Image/B3DSpriteImage.h"
#include "../../../Foundation/Core/Image/B3DSpriteImage.h"

namespace b3d
{
	struct __SpriteImageInformationInterop
	{
		SpriteAnimationPlayback AnimationPlayback;
		SpriteSheetGridAnimation Animation;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptSpriteImageInformation : public TScriptTypeDefinition<ScriptSpriteImageInformation>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "SpriteImageInformation")

		static MonoObject* Box(const __SpriteImageInformationInterop& value);
		static __SpriteImageInformationInterop Unbox(MonoObject* value);
		static SpriteImageInformation FromInterop(const __SpriteImageInformationInterop& value);
		static __SpriteImageInformationInterop ToInterop(const SpriteImageInformation& value);

	private:
		ScriptSpriteImageInformation();

	};
}
