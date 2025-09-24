//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptObjectWrapper.h"
#include "../../../Foundation/Engine/Input/B3DInputConfiguration.h"

namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptVirtualButton : public TScriptTypeDefinition<ScriptVirtualButton>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "VirtualButton")

		static MonoObject* Box(const VirtualButton& value);
		static VirtualButton Unbox(MonoObject* value);

	private:
		ScriptVirtualButton();

	};
}
