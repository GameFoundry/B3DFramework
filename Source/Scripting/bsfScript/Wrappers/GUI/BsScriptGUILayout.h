//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptGUIElementWrapper.h"
#include "GUI/BsGUILayout.h"
#include "GUI/BsGUIPanel.h"

namespace bs
{
	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */

	/** Interop class between C++ & CLR for all elements inheriting from GUILayout. */
	class B3D_SCRIPT_INTEROP_EXPORT ScriptGUILayoutWrapperBase : public ScriptGUIElementWrapper
	{
	public:
		using ScriptGUIElementWrapper::ScriptGUIElementWrapper;

		/** Returns the native object that is being wrapped. */
		GUILayout* GetNativeObject() const { return static_cast<GUILayout*>(mNativeObject); }
	};

	/**	Interop class between C++ & CLR for GUIPanel.  */
	class B3D_SCRIPT_INTEROP_EXPORT ScriptGUIPanel : public TScriptGUIElementWrapper<GUIPanel, ScriptGUIPanel, ScriptGUILayoutWrapperBase>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "GUIPanel")

		ScriptGUIPanel(GUIPanel* nativeObject);

		static void SetupScriptBindings();
		static MonoObject* CreateScriptObject(bool construct);

	private:
		static void InternalCreate(MonoObject* instance, i16 depth, u16 depthRangeMin, u32 depthRangeMax, MonoArray* guiOptions);
	};

	/** @} */
} // namespace bs
