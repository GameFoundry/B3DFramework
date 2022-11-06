//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
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

	class B3D_SCRIPT_INTEROP_EXPORT ScriptBlendClipInfo : public ScriptObject<ScriptBlendClipInfo>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "BlendClipInfo")

		static MonoObject* Box(const __BlendClipInfoInterop& value);
		static __BlendClipInfoInterop Unbox(MonoObject* value);
		static BlendClipInfo FromInterop(const __BlendClipInfoInterop& value);
		static __BlendClipInfoInterop ToInterop(const BlendClipInfo& value);

	private:
		ScriptBlendClipInfo(MonoObject* managedInstance);

	};
}
