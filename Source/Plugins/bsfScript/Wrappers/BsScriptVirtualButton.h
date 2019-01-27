//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "Input/BsInputConfiguration.h"

namespace bs
{
	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */

	/**	Interop class between C++ & CLR for VirtualButton. */
	class BS_SCR_BE_EXPORT ScriptVirtualButton : public ScriptObject <ScriptVirtualButton>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "VirtualButton")

		/**	Unboxes a boxed managed VirtualButton struct and returns the native version of the structure. */
		static VirtualButton unbox(MonoObject* obj);

		/**	Boxes a native VirtualButton struct and returns a managed object containing it. */
		static MonoObject* box(const VirtualButton& value);

	private:
		ScriptVirtualButton(MonoObject* instance);

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static UINT32 internal_InitVirtualButton(MonoString* name);
	};

	/** @} */
}