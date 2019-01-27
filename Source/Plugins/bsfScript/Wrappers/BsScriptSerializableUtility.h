//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"

namespace bs
{
	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */

	/**	Implements external methods for the SerializableUtility managed class. */
	class BS_SCR_BE_EXPORT ScriptSerializableUtility : public ScriptObject<ScriptSerializableUtility>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "SerializableUtility")

	private:
		ScriptSerializableUtility(MonoObject* instance);

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static MonoObject* internal_Clone(MonoObject* original);
		static MonoObject* internal_Create(MonoReflectionType* type);
	};

	/** @} */
}