//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptGUIElement.h"

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