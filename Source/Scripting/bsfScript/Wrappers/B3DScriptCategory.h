//********************************* B3D Framework - Copyright 2026 Marko Pintera *****************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptTypeDefinition.h"

namespace b3d
{
	/**	Interop class between C++ & CLR for the Category attribute. */
	class B3D_SCRIPT_INTEROP_EXPORT ScriptCategory : public TScriptTypeDefinition<ScriptCategory>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "Category")

		ScriptCategory();

		static void SetupScriptBindings();

		static MonoField* GetNameField() { return sNameField; }

	private:
		static MonoField* sNameField;
	};
} // namespace b3d
