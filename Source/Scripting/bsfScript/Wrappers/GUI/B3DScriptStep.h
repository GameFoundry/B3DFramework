//********************************* B3D Framework - Copyright 2026 Marko Pintera *****************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptTypeDefinition.h"

namespace b3d
{
	/**	Interop class between C++ & CLR for Step attribute. */
	class B3D_SCRIPT_INTEROP_EXPORT ScriptStep : public TScriptTypeDefinition<ScriptStep>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "Step")

		ScriptStep();

		static void SetupScriptBindings();
		static MonoField* GetStepField() { return sStepField; }

	private:
		static MonoField* sStepField;
	};
} // namespace b3d
