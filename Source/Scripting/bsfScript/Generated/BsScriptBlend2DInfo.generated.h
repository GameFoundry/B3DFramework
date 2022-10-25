//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "../../../Foundation/bsfCore/Animation/BsAnimation.h"

namespace bs
{
	struct __Blend2DInfoInterop
	{
		MonoObject* TopLeftClip;
		MonoObject* TopRightClip;
		MonoObject* BotLeftClip;
		MonoObject* BotRightClip;
	};

	class BS_SCR_BE_EXPORT ScriptBlend2DInfo : public ScriptObject<ScriptBlend2DInfo>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "Blend2DInfo")

		static MonoObject* Box(const __Blend2DInfoInterop& value);
		static __Blend2DInfoInterop Unbox(MonoObject* value);
		static Blend2DInfo FromInterop(const __Blend2DInfoInterop& value);
		static __Blend2DInfoInterop ToInterop(const Blend2DInfo& value);

	private:
		ScriptBlend2DInfo(MonoObject* managedInstance);
	};
} // namespace bs
