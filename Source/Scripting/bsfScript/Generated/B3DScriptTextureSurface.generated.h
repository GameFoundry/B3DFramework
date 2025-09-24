//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptObjectWrapper.h"
#include "../../../Foundation/Core/Utility/B3DCommonTypes.h"

namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptTextureSurface : public TScriptTypeDefinition<ScriptTextureSurface>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "TextureSurface")

		static MonoObject* Box(const TextureSurface& value);
		static TextureSurface Unbox(MonoObject* value);

	private:
		ScriptTextureSurface();

	};
}
