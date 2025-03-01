//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObjectWrapper.h"
#include "../../../Foundation/bsfCore/Image/BsSpriteImage.h"
#include "../../../Foundation/bsfCore/Image/BsSpriteImage.h"
#include "Math/BsArea2.h"
#include "../../../Foundation/bsfCore/Image/BsSpriteImage.h"

namespace bs
{
	struct __SpriteImageCreateInformationInterop
	{
		Area2 UVRange;
		SpriteAnimationPlayback AnimationPlayback;
		SpriteSheetGridAnimation Animation;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptSpriteImageCreateInformation : public TScriptTypeDefinition<ScriptSpriteImageCreateInformation>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "SpriteImageCreateInformation")

		static MonoObject* Box(const __SpriteImageCreateInformationInterop& value);
		static __SpriteImageCreateInformationInterop Unbox(MonoObject* value);
		static SpriteImageCreateInformation FromInterop(const __SpriteImageCreateInformationInterop& value);
		static __SpriteImageCreateInformationInterop ToInterop(const SpriteImageCreateInformation& value);

	private:
		ScriptSpriteImageCreateInformation();

	};
}
