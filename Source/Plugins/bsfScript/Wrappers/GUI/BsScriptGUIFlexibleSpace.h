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

	/**	Interop class between C++ & CLR for GUIFlexibleSpace. */
	class BS_SCR_BE_EXPORT ScriptGUIFlexibleSpace : public TScriptGUIElementBase<ScriptGUIFlexibleSpace>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "GUIFlexibleSpace")

	private:
		ScriptGUIFlexibleSpace(MonoObject* instance, GUIFlexibleSpace* flexibleSpace);

		/** @copydoc ScriptGUIElementBaseTBase::destroy */
		void destroy() override;

		GUIFlexibleSpace* mFlexibleSpace;
		bool mIsDestroyed;

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void internal_createInstance(MonoObject* instance);
	};

	/** @} */
}