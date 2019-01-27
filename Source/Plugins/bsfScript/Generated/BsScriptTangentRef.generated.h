//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "../../../../../Foundation/bsfCore/Animation/BsAnimationUtility.h"
#include "../../../../../Foundation/bsfCore/Animation/BsAnimationUtility.h"
#include "../../../../../Foundation/bsfCore/Animation/BsAnimationUtility.h"

namespace bs
{
	struct __TangentRefInterop
	{
		KeyframeRef keyframeRef;
		TangentType type;
	};

	class BS_SCR_BE_EXPORT ScriptTangentRef : public ScriptObject<ScriptTangentRef>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "bs", "TangentRef")

		static MonoObject* box(const __TangentRefInterop& value);
		static __TangentRefInterop unbox(MonoObject* value);
		static TangentRef fromInterop(const __TangentRefInterop& value);
		static __TangentRefInterop toInterop(const TangentRef& value);

	private:
		ScriptTangentRef(MonoObject* managedInstance);

	};
}
