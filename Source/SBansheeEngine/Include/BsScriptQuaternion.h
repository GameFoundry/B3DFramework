//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "BsQuaternion.h"

namespace bs
{
	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */

	/**	Interop class between C++ & CLR for Quaternion. */
	class BS_SCR_BE_EXPORT ScriptQuaternion : public ScriptObject <ScriptQuaternion>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "Quaternion")

		/** Unboxes a boxed managed Quaternion struct and returns the native version of the structure. */
		static Quaternion unbox(MonoObject* obj);

		/**	Boxes a native Quaternion struct and returns a managed object containing it. */
		static MonoObject* box(const Quaternion& value);

	private:
		ScriptQuaternion(MonoObject* instance);
	};

	/** @} */
}
