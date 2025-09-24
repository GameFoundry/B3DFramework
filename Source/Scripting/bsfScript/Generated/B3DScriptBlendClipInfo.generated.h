//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptObjectWrapper.h"
#include "../../../Foundation/Core/Components/B3DAnimation.h"

namespace b3d
{
	struct __BlendClipInfoInterop
	{
		MonoObject* Clip;
		float Position;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptBlendClipInfo : public TScriptTypeDefinition<ScriptBlendClipInfo>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "BlendClipInfo")

		static MonoObject* Box(const __BlendClipInfoInterop& value);
		static __BlendClipInfoInterop Unbox(MonoObject* value);
		static BlendClipInfo FromInterop(const __BlendClipInfoInterop& value);
		static __BlendClipInfoInterop ToInterop(const BlendClipInfo& value);

	private:
		ScriptBlendClipInfo();

	};
}
