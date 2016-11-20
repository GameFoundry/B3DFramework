//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
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