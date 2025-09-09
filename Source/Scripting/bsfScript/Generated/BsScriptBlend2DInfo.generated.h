//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObjectWrapper.h"
#include "../../../Foundation/bsfCore/Components/BsAnimation.h"

namespace b3d
{
	struct __Blend2DInfoInterop
	{
		MonoObject* TopLeftClip;
		MonoObject* TopRightClip;
		MonoObject* BottomLeftClip;
		MonoObject* BottomRightClip;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptBlend2DInfo : public TScriptTypeDefinition<ScriptBlend2DInfo>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "Blend2DInfo")

		static MonoObject* Box(const __Blend2DInfoInterop& value);
		static __Blend2DInfoInterop Unbox(MonoObject* value);
		static Blend2DInfo FromInterop(const __Blend2DInfoInterop& value);
		static __Blend2DInfoInterop ToInterop(const Blend2DInfo& value);

	private:
		ScriptBlend2DInfo();

	};
}
