//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptObjectWrapper.h"
#include "../../../Foundation/Core/Image/B3DSpriteImage.h"

namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptSpriteSheetGridAnimation : public TScriptTypeDefinition<ScriptSpriteSheetGridAnimation>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "SpriteSheetGridAnimation")

		static MonoObject* Box(const SpriteSheetGridAnimation& value);
		static SpriteSheetGridAnimation Unbox(MonoObject* value);

	private:
		ScriptSpriteSheetGridAnimation();

	};
}
