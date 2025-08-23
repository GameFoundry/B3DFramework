//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObjectWrapper.h"
#include "../../../Foundation/bsfCore/Components/BsJoint.h"

namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptSpring : public TScriptTypeDefinition<ScriptSpring>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "Spring")

		static MonoObject* Box(const Spring& value);
		static Spring Unbox(MonoObject* value);

	private:
		ScriptSpring();

	};
}
