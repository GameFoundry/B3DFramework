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

	/**	Interop class between C++ & CLR for GUIToggleGroup. */
	class BS_SCR_BE_EXPORT ScriptGUIToggleGroup : public ScriptObject<ScriptGUIToggleGroup>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "GUIToggleGroup")

		/**	Returns the native toggle group that this object wraps. */
		SPtr<GUIToggleGroup> GetInternalValue() const { return mToggleGroup; }

	private:
		ScriptGUIToggleGroup(MonoObject* instance, const SPtr<GUIToggleGroup>& toggleGroup);

		SPtr<GUIToggleGroup> mToggleGroup;

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void InternalCreateInstance(MonoObject* instance, bool allowAllOff);
	};

	/** @} */
} // namespace bs
