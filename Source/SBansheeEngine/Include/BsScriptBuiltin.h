//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "BsBuiltinResources.h"

namespace BansheeEngine
{
	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */

	/**	Interop class between C++ & CLR for BuiltinResources. */
	class BS_SCR_BE_EXPORT ScriptBuiltin : public ScriptObject <ScriptBuiltin>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "Builtin")

	private:
		ScriptBuiltin(MonoObject* instance);

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static MonoObject* internal_GetWhiteTexture();
		static MonoObject* internal_GetDiffuseShader();
		static MonoObject* internal_GetMesh(BuiltinMesh meshType);
		static MonoObject* internal_GetDefaultFont();
	};

	/** @} */
}