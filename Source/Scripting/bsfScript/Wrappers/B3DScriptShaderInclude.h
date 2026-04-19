//********************************* B3D Framework - Copyright 2026 Marko Pintera *****************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptResourceWrapper.h"
#include "Material/B3DShaderInclude.h"

namespace b3d
{
	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */

	/**	Interop class between C++ & CLR for ShaderInclude. */
	class B3D_SCRIPT_INTEROP_EXPORT ScriptShaderInclude : public TScriptResourceWrapper<ShaderInclude, ScriptShaderInclude>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "ShaderInclude")

		ScriptShaderInclude(const HShaderInclude& nativeObject);

		/** Retrieves the underlying native object cast to the correct type. */
		ShaderInclude* GetNativeObject() const;

		static void SetupScriptBindings();
		static MonoObject* CreateScriptObject(bool construct);
	};

	/** @} */
} // namespace b3d
