//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "../../../Foundation/bsfCore/Image/BsSpriteImage.h"
#include "../../../Foundation/bsfCore/Image/BsSpriteImage.h"
#include "Math/BsRect2.h"
#include "../../../Foundation/bsfCore/Image/BsSpriteImage.h"

namespace bs
{
	struct __SpriteImageInformationInterop
	{
		Rect2 UVRange;
		SpriteAnimationPlayback AnimationPlayback;
		SpriteSheetGridAnimation Animation;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptSpriteImageInformation : public ScriptObject<ScriptSpriteImageInformation>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "SpriteImageInformation")

		static MonoObject* Box(const __SpriteImageInformationInterop& value);
		static __SpriteImageInformationInterop Unbox(MonoObject* value);
		static SpriteImageInformation FromInterop(const __SpriteImageInformationInterop& value);
		static __SpriteImageInformationInterop ToInterop(const SpriteImageInformation& value);

	private:
		ScriptSpriteImageInformation(MonoObject* managedInstance);

	};
}
