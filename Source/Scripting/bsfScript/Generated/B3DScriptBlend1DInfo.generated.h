//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptObjectWrapper.h"
#include "../../../Foundation/Core/Components/B3DAnimation.h"
#include "../../../Foundation/Core/Components/B3DAnimation.h"
#include "B3DScriptBlendClipInfo.generated.h"

namespace b3d
{
	struct __Blend1DInfoInterop
	{
		MonoArray* Clips;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptBlend1DInfo : public TScriptTypeDefinition<ScriptBlend1DInfo>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "Blend1DInfo")

		static MonoObject* Box(const __Blend1DInfoInterop& value);
		static __Blend1DInfoInterop Unbox(MonoObject* value);
		static Blend1DInfo FromInterop(const __Blend1DInfoInterop& value);
		static __Blend1DInfoInterop ToInterop(const Blend1DInfo& value);

	private:
		ScriptBlend1DInfo();

	};
}
