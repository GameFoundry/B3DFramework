//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObjectWrapper.h"
#include "../../../Foundation/bsfCore/Animation/BsAnimation.h"
#include "../../../Foundation/bsfCore/Animation/BsAnimation.h"
#include "BsScriptBlendClipInfo.generated.h"

namespace bs
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
