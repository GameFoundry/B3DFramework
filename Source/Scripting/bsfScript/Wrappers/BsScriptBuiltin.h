//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "Resources/BsBuiltinResources.h"

namespace bs
{
	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */

	/**	Interop class between C++ & CLR for BuiltinResources. */
	class BS_SCR_BE_EXPORT ScriptBuiltin : public ScriptObject<ScriptBuiltin>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "Builtin")

	private:
		ScriptBuiltin(MonoObject* instance);

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static MonoObject* InternalGetWhiteTexture();
		static MonoObject* InternalGetBuiltinShader(BuiltinShader type);
		static MonoObject* InternalGetMesh(BuiltinMesh meshType);
		static MonoObject* InternalGetDefaultFont();
	};

	/** @} */
} // namespace bs
