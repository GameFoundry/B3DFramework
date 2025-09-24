//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptObjectWrapper.h"
#include "../../../Foundation/Utility/Image/B3DColorGradient.h"
#include "../../../Foundation/Utility/Image/B3DColor.h"

namespace b3d
{
	struct __ColorGradientKeyInterop
	{
		Color Color;
		float Time;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptColorGradientKey : public TScriptTypeDefinition<ScriptColorGradientKey>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "ColorGradientKey")

		static MonoObject* Box(const __ColorGradientKeyInterop& value);
		static __ColorGradientKeyInterop Unbox(MonoObject* value);
		static ColorGradientKey FromInterop(const __ColorGradientKeyInterop& value);
		static __ColorGradientKeyInterop ToInterop(const ColorGradientKey& value);

	private:
		ScriptColorGradientKey();

	};
}
