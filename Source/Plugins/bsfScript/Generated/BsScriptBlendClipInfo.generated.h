//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "../../../../../Foundation/bsfCore/Animation/BsAnimation.h"

namespace bs
{
	struct __BlendClipInfoInterop
	{
		MonoObject* clip;
		float position;
	};

	class BS_SCR_BE_EXPORT ScriptBlendClipInfo : public ScriptObject<ScriptBlendClipInfo>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "bs", "BlendClipInfo")

		static MonoObject* box(const __BlendClipInfoInterop& value);
		static __BlendClipInfoInterop unbox(MonoObject* value);
		static BlendClipInfo fromInterop(const __BlendClipInfoInterop& value);
		static __BlendClipInfoInterop toInterop(const BlendClipInfo& value);

	private:
		ScriptBlendClipInfo(MonoObject* managedInstance);

	};
}
