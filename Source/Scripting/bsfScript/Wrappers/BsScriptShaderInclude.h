//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptResourceWrapper.h"
#include "Material/BsShaderInclude.h"

namespace bs
{
	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */

	/**	Interop class between C++ & CLR for ShaderInclude. */
	class B3D_SCRIPT_INTEROP_EXPORT ScriptShaderInclude : public TScriptResourceWrapper<ShaderInclude, ScriptShaderInclude>
	{
	public:
		B3D_SCRIPT_OBJECT_WRAPPER(kEngineAssembly, kEngineNs, "ShaderInclude")

		ScriptShaderInclude(const HShaderInclude& nativeObject, MonoObject* scriptObject);

		/** Retrieves the underlying native object cast to the correct type. */
		ShaderInclude* GetNativeObject() const;

		static MonoObject* CreateScriptObject(bool construct);
	};

	/** @} */
} // namespace bs
