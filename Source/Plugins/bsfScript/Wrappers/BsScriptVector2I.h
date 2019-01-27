//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "Math/BsVector2I.h"

namespace bs
{
	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */

	/**	Interop class between C++ & CLR for Vector2I. */
	class BS_SCR_BE_EXPORT ScriptVector2I : public ScriptObject<ScriptVector2I>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "Vector2I")

		/**	Unboxes a boxed managed Vector2I struct and returns the native version of the structure. */
		static Vector2I unbox(MonoObject* obj);

		/**	Boxes a native Vector2I struct and returns a managed object containing it. */
		static MonoObject* box(const Vector2I& value);

	private:
		ScriptVector2I(MonoObject* instance);
	};

	/** @} */
}