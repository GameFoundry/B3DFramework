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
		B3D_SCRIPT_OBJECT_WRAPPER(kEngineAssembly, kEngineNs, "GUILayout")

		ScriptGUILayout(GUILayout* nativeObject);

		static MonoObject* CreateScriptObject(bool construct);

	protected:
		friend class ScriptGUIPanel;

	private:
		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void InternalCreateInstanceX(MonoObject* instance, MonoArray* guiOptions);
		static void InternalCreateInstanceY(MonoObject* instance, MonoArray* guiOptions);
		static void InternalCreateInstancePanel(MonoObject* instance, i16 depth, u16 depthRangeMin, u32 depthRangeMax, MonoArray* guiOptions);
		static void InternalCreateInstanceYFromScrollArea(MonoObject* instance, MonoObject* parentScrollArea);
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
		B3D_SCRIPT_OBJECT_WRAPPER(kEngineAssembly, kEngineNs, "GUIPanel")

		ScriptGUIPanel(GUIPanel* nativeObject);

		static MonoObject* CreateScriptObject(bool construct);
	};

	/**	Specialized ScriptGUILayout that is used only in GUI scroll areas. */
	class B3D_SCRIPT_INTEROP_EXPORT ScriptGUIScrollAreaLayout : public ScriptGUILayout // TODO
	{
	public:
		/**
		 * Constructor.
		 *
		 * @param[in]	instance	Managed GUILayout instance.
		 * @param[in]	layout  	Native GUILayout instance.
		 */
		ScriptGUIScrollAreaLayout(MonoObject* instance, GUILayout* layout);

		/** @copydoc ScriptGUILayout::destroy */
		void Destroy();

	private:
		friend class ScriptGUIScrollArea;

		ScriptGUIScrollArea* mParentScrollArea;
	};

	/** @} */
} // namespace bs
