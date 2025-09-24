//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObjectWrapper.h"
#include "../../../Foundation/bsfEngine/Input/BsInputConfiguration.h"

namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptVirtualAxis : public TScriptTypeDefinition<ScriptVirtualAxis>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "VirtualAxis")

		static MonoObject* Box(const VirtualAxis& value);
		static VirtualAxis Unbox(MonoObject* value);

	private:
		ScriptVirtualAxis();

	};
}
