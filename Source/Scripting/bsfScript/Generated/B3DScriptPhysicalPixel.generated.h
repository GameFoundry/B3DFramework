//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptObjectWrapper.h"
#include "../../../Foundation/Engine/GUI/B3DGUIUnits.h"

namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptPhysicalPixel : public TScriptTypeDefinition<ScriptPhysicalPixel>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "PhysicalPixel")

		static MonoObject* Box(const PhysicalPixel& value);
		static PhysicalPixel Unbox(MonoObject* value);

	private:
		ScriptPhysicalPixel();

	};
}
