//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "../../../../../Foundation/bsfCore/Animation/BsAnimation.h"
#include "../../../../../Foundation/bsfCore/Animation/BsAnimation.h"
#include "BsScriptBlendClipInfo.generated.h"

namespace bs
{
	struct __Blend1DInfoInterop
	{
		MonoArray* clips;
	};

	class BS_SCR_BE_EXPORT ScriptBlend1DInfo : public ScriptObject<ScriptBlend1DInfo>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "bs", "Blend1DInfo")

		static MonoObject* box(const __Blend1DInfoInterop& value);
		static __Blend1DInfoInterop unbox(MonoObject* value);
		static Blend1DInfo fromInterop(const __Blend1DInfoInterop& value);
		static __Blend1DInfoInterop toInterop(const Blend1DInfo& value);

	private:
		ScriptBlend1DInfo(MonoObject* managedInstance);

	};
}
