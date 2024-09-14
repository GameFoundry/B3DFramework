//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObjectWrapper.h"
#include "../../../Foundation/bsfCore/Animation/BsAnimationUtility.h"

namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptKeyframeRef : public TScriptStructWrapper<ScriptKeyframeRef>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "KeyframeRef")

		static MonoObject* Box(const KeyframeRef& value);
		static KeyframeRef Unbox(MonoObject* value);

	private:
		ScriptKeyframeRef();

	};
}
