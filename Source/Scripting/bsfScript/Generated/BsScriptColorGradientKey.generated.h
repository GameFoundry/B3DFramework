//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "../../../Foundation/bsfUtility/Image/BsColorGradient.h"
#include "Image/BsColor.h"

namespace bs
{
	struct __ColorGradientKeyInterop
	{
		Color Color;
		float Time;
	};

	class BS_SCR_BE_EXPORT ScriptColorGradientKey : public ScriptObject<ScriptColorGradientKey>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "ColorGradientKey")

		static MonoObject* Box(const __ColorGradientKeyInterop& value);
		static __ColorGradientKeyInterop Unbox(MonoObject* value);
		static ColorGradientKey FromInterop(const __ColorGradientKeyInterop& value);
		static __ColorGradientKeyInterop ToInterop(const ColorGradientKey& value);

	private:
		ScriptColorGradientKey(MonoObject* managedInstance);
	};
} // namespace bs
