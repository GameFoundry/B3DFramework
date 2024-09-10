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

	/**	Interop class between C++ & CLR for GUIFixedSpace. */
	class B3D_SCRIPT_INTEROP_EXPORT ScriptGUIFixedSpace : public TScriptGUIElementWrapper<GUIFixedSpace, ScriptGUIFixedSpace>
	{
	public:
		B3D_SCRIPT_OBJECT_WRAPPER(kEngineAssembly, kEngineNs, "GUIFixedSpace")

		ScriptGUIFixedSpace(GUIFixedSpace* nativeObject);

		/** Returns the native object that is being wrapped. */
		GUIFixedSpace* GetNativeObject() const { return static_cast<GUIFixedSpace*>(mNativeObject); }

		static MonoObject* CreateScriptObject(bool construct);
	private:
		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void InternalCreateInstance(MonoObject* instance, u32 size);
		static void InternalSetSize(ScriptGUIFixedSpace* self, u32 size);
	};

	/** @} */
} // namespace bs
