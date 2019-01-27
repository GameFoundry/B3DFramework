//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "../../../../../Foundation/bsfCore/Animation/BsAnimationUtility.h"

namespace bs
{
	class BS_SCR_BE_EXPORT ScriptKeyframeRef : public ScriptObject<ScriptKeyframeRef>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "bs", "KeyframeRef")

		static MonoObject* box(const KeyframeRef& value);
		static KeyframeRef unbox(MonoObject* value);

	private:
		ScriptKeyframeRef(MonoObject* managedInstance);

	};
}
