//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "../../../Foundation/bsfCore/Image/BsSpriteTexture.h"

namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptSpriteSheetGridAnimation : public ScriptObject<ScriptSpriteSheetGridAnimation>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "SpriteSheetGridAnimation")

		static MonoObject* Box(const SpriteSheetGridAnimation& value);
		static SpriteSheetGridAnimation Unbox(MonoObject* value);

	private:
		ScriptSpriteSheetGridAnimation(MonoObject* managedInstance);

	};
}
