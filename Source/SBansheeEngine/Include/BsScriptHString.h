//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "BsHString.h"

namespace bs
{
	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */

	/**	Interop class between C++ & CLR for HString. */
	class BS_SCR_BE_EXPORT ScriptHString : public ScriptObject<ScriptHString>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "LocString")

		/**	Returns the wrapped native HString. */
		const HString& getInternalValue() const { return mString; }

	private:
		ScriptHString(MonoObject* instance, const HString& string);

		HString mString;

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void internal_createInstance(MonoObject* instance, MonoString* identifier, UINT32 tableId);
		static void internal_setParameter(ScriptHString* nativeInstance, UINT32 idx, MonoString* value);
		static void internal_getValue(ScriptHString* nativeInstance, MonoString** value);
	};

	/** @} */
}