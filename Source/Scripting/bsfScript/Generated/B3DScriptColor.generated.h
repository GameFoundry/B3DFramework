//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptObjectWrapper.h"
#include "../../../Foundation/Utility/Image/B3DColor.h"

namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptColor : public TScriptTypeDefinition<ScriptColor>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "Color")

		static MonoObject* Box(const Color& value);
		static Color Unbox(MonoObject* value);

	private:
		ScriptColor();

	};
}
