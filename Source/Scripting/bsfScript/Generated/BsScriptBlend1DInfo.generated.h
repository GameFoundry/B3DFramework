//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "../../../Foundation/bsfCore/Animation/BsAnimation.h"
#include "../../../Foundation/bsfCore/Animation/BsAnimation.h"
#include "BsScriptBlendClipInfo.generated.h"

namespace bs
{
	struct __Blend1DInfoInterop
	{
		MonoArray* Clips;
	};

	class BS_SCR_BE_EXPORT ScriptBlend1DInfo : public ScriptObject<ScriptBlend1DInfo>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "Blend1DInfo")

		static MonoObject* Box(const __Blend1DInfoInterop& value);
		static __Blend1DInfoInterop Unbox(MonoObject* value);
		static Blend1DInfo FromInterop(const __Blend1DInfoInterop& value);
		static __Blend1DInfoInterop ToInterop(const Blend1DInfo& value);

	private:
		ScriptBlend1DInfo(MonoObject* managedInstance);
	};
} // namespace bs
