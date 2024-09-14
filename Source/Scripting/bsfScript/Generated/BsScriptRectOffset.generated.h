//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObjectWrapper.h"
#include "../../../Foundation/bsfEngine/Utility/BsRectOffset.h"

namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptRectOffset : public TScriptTypeDefinition<ScriptRectOffset>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "RectOffset")

		static MonoObject* Box(const RectOffset& value);
		static RectOffset Unbox(MonoObject* value);

	private:
		ScriptRectOffset();

	};
}
