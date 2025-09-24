//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "../../../Foundation/bsfEngine/BsApplication.h"
#include "BsScriptTypeDefinition.h"
#include "BsScriptCoreApplication.generated.h"

namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptApplication : public TScriptTypeDefinition<ScriptApplication>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "Application")

		ScriptApplication();

		static void SetupScriptBindings();

	private:
	};
}
