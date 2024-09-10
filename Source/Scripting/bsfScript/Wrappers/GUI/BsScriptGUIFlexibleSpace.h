//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptGUIElementWrapper.h"
#include "GUI/BsGUISpace.h"

namespace bs
{
	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */

	/**	Interop class between C++ & CLR for GUIFlexibleSpace. */
	class B3D_SCRIPT_INTEROP_EXPORT ScriptGUIFlexibleSpace : public TScriptGUIElementWrapper<GUIFlexibleSpace, ScriptGUIFlexibleSpace>
	{
	public:
		B3D_SCRIPT_OBJECT_WRAPPER(kEngineAssembly, kEngineNs, "GUIFlexibleSpace")

		ScriptGUIFlexibleSpace(GUIFlexibleSpace* nativeObject);

		/** Returns the native object that is being wrapped. */
		GUIFlexibleSpace* GetNativeObject() const { return static_cast<GUIFlexibleSpace*>(mNativeObject); }

		static MonoObject* CreateScriptObject(bool construct);
	private:
		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void InternalCreateInstance(MonoObject* instance);
	};

	/** @} */
} // namespace bs
