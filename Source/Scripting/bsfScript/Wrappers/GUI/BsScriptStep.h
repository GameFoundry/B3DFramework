//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptTypeDefinition.h"

namespace bs
{
	/**	Interop class between C++ & CLR for Step attribute. */
	class B3D_SCRIPT_INTEROP_EXPORT ScriptStep : public TScriptTypeDefinition<ScriptStep>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "Step")

		static void SetupScriptBindings();
		static MonoField* GetStepField() { return sStepField; }

	private:
		static MonoField* sStepField;
	};
} // namespace bs
