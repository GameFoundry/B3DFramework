//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/GUI/BsScriptGUIElement.h"

namespace bs
{
	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */

	/**	Interop class between C++ & CLR for GUIFixedSpace. */
	class BS_SCR_BE_EXPORT ScriptGUIFixedSpace : public TScriptGUIElementBase<ScriptGUIFixedSpace>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "GUIFixedSpace")

	private:
		ScriptGUIFixedSpace(MonoObject* instance, GUIFixedSpace* fixedSpace);

		/** @copydoc ScriptGUIElementBaseTBase::destroy */
		void Destroy();

		GUIFixedSpace* mFixedSpace;
		bool mIsDestroyed;

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void InternalCreateInstance(MonoObject* instance, u32 size);
		static void InternalSetSize(ScriptGUIFixedSpace* nativeInstance, u32 size);
	};

	/** @} */
} // namespace bs
