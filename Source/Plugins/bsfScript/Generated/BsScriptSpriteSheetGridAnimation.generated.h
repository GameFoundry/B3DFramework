//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "../../../Foundation/bsfCore/Image/BsSpriteTexture.h"

namespace bs
{
	class BS_SCR_BE_EXPORT ScriptSpriteSheetGridAnimation : public ScriptObject<ScriptSpriteSheetGridAnimation>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "SpriteSheetGridAnimation")

		static MonoObject* box(const SpriteSheetGridAnimation& value);
		static SpriteSheetGridAnimation unbox(MonoObject* value);

	private:
		ScriptSpriteSheetGridAnimation(MonoObject* managedInstance);

	};
}
