//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptResource.h"
#include "Material/BsShaderInclude.h"

namespace bs
{
	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */

	/**	Interop class between C++ & CLR for ShaderInclude. */
	class BS_SCR_BE_EXPORT ScriptShaderInclude : public TScriptResource <ScriptShaderInclude, ShaderInclude>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "ShaderInclude")

		/**	Creates an empty, uninitialized managed instance of the resource interop object. */
		static MonoObject* createInstance();

	private:
		friend class ScriptResourceManager;
		friend class BuiltinResourceTypes;

		ScriptShaderInclude(MonoObject* instance, const HShaderInclude& shaderInclude);
	};

	/** @} */
}