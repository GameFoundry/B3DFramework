//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "../../../Foundation/bsfCore/BsCoreApplication.h"
#include "BsScriptTypeDefinition.h"

namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptCoreApplication : public TScriptTypeDefinition<ScriptCoreApplication>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "CoreApplication")

		ScriptCoreApplication();

		static void SetupScriptBindings();

	private:
	};
}
