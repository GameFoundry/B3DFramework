//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptObjectWrapper.h"
#include "../../../Foundation/Core/Animation/B3DAnimationUtility.h"

namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptKeyframeRef : public TScriptTypeDefinition<ScriptKeyframeRef>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "KeyframeRef")

		static MonoObject* Box(const KeyframeRef& value);
		static KeyframeRef Unbox(MonoObject* value);

	private:
		ScriptKeyframeRef();

	};
}
