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

	/**	Interop class between C++ & CLR for GUILayout derived classes. */
	class B3D_SCRIPT_INTEROP_EXPORT ScriptGUILayout : public TScriptGUIElementWrapper<GUILayout, ScriptGUILayout, ScriptGUILayoutWrapperBase>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "GUILayout")

		ScriptGUILayout(GUILayout* nativeObject);

		static void SetupScriptBindings();
		static MonoObject* CreateScriptObject(bool construct) { return nullptr; }

	protected:
		friend class ScriptGUIPanel;

	private:
		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void InternalAddElement(ScriptGUILayoutWrapperBase* self, ScriptGUIElementWrapper* element);
		static void InternalInsertElement(ScriptGUILayoutWrapperBase* self, u32 index, ScriptGUIElementWrapper* element);
		static u32 InternalGetChildCount(ScriptGUILayoutWrapperBase* self);
		static MonoObject* InternalGetChild(ScriptGUILayoutWrapperBase* self, u32 index);
		static void InternalClear(ScriptGUILayoutWrapperBase* self);
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

	/**	Interop class between C++ & CLR for GUILayoutX.  */
	class B3D_SCRIPT_INTEROP_EXPORT ScriptGUILayoutX : public TScriptGUIElementWrapper<GUILayoutX, ScriptGUILayoutX, ScriptGUILayoutWrapperBase>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "GUILayoutX")

		ScriptGUILayoutX(GUILayoutX* nativeObject);

		static void SetupScriptBindings();
		static MonoObject* CreateScriptObject(bool construct);

	private:
		static void InternalCreate(MonoObject* instance, MonoArray* guiOptions);
	};

	/**	Interop class between C++ & CLR for GUILayoutY.  */
	class B3D_SCRIPT_INTEROP_EXPORT ScriptGUILayoutY : public TScriptGUIElementWrapper<GUILayoutY, ScriptGUILayoutY, ScriptGUILayoutWrapperBase>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "GUILayoutY")

		ScriptGUILayoutY(GUILayoutY* nativeObject);

		static void SetupScriptBindings();
		static MonoObject* CreateScriptObject(bool construct);

	private:
		static void InternalCreate(MonoObject* instance, MonoArray* guiOptions);
	};

	/** @} */
} // namespace bs
