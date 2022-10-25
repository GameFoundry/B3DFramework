//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "../../../Foundation/bsfCore/Animation/BsAnimation.h"

namespace bs
{
	struct __BlendClipInfoInterop
	{
		MonoObject* Clip;
		float Position;
	};

	class BS_SCR_BE_EXPORT ScriptBlendClipInfo : public ScriptObject<ScriptBlendClipInfo>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "BlendClipInfo")

		static MonoObject* Box(const __BlendClipInfoInterop& value);
		static __BlendClipInfoInterop Unbox(MonoObject* value);
		static BlendClipInfo FromInterop(const __BlendClipInfoInterop& value);
		static __BlendClipInfoInterop ToInterop(const BlendClipInfo& value);

	private:
		ScriptBlendClipInfo(MonoObject* managedInstance);
	};
} // namespace bs
