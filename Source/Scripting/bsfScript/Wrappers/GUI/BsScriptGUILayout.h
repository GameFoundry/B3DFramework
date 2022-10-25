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

	/**	Interop class between C++ & CLR for GUILayout derived classes. */
	class BS_SCR_BE_EXPORT ScriptGUILayout : public TScriptGUIElementBase<ScriptGUILayout>
	{
	protected:
		/**	Contains information about an interop object that represents a child of the layout. */
		struct ChildInfo
		{
			ScriptGUIElementBaseTBase* Element;
			uint32_t GcHandle;
		};

	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "GUILayout")

		virtual ~ScriptGUILayout() {}

		/**	Returns the internal wrapped GUILayout object. */
		GUILayout* GetInternalValue() const { return mLayout; }

		/**	Registers a new managed child GUI element and inserts it at the end of the layout. */
		void AddChild(ScriptGUIElementBaseTBase* element);

		/**	Registers a new managed child GUI element and inserts it at a specific location in the layout. */
		void InsertChild(u32 index, ScriptGUIElementBaseTBase* element);

		/**	Removes a managed GUI element from the layout. */
		void RemoveChild(ScriptGUIElementBaseTBase* element);

		/**
		 * @copydoc	ScriptGUIElementBaseTBase::destroy
		 *
		 * Destroys the layout and all of its managed children.
		 */
		void Destroy();

	protected:
		friend class ScriptGUIPanel;

		/**
		 * Constructor.
		 *
		 * @param[in]	instance	Managed GUILayout instance.
		 * @param[in]	layout  	Native GUILayout instance.
		 * @param[in]	ownsNative	Does this object own the native instance. If it does it will destroy the
		 * 							object when Destroy() is called, otherwise it is up to the caller to destroy it.
		 */
		ScriptGUILayout(MonoObject* instance, GUILayout* layout, bool ownsNative = true);

		GUILayout* mLayout;
		Vector<ChildInfo> mChildren;

		bool mOwnsNative;

	private:
		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void InternalCreateInstanceX(MonoObject* instance, MonoArray* guiOptions);
		static void InternalCreateInstanceY(MonoObject* instance, MonoArray* guiOptions);
		static void InternalCreateInstancePanel(MonoObject* instance, i16 depth, u16 depthRangeMin, u32 depthRangeMax, MonoArray* guiOptions);
		static void InternalAddElement(ScriptGUILayout* instance, ScriptGUIElementBaseTBase* element);
		static void InternalInsertElement(ScriptGUILayout* instance, u32 index, ScriptGUIElementBaseTBase* element);
		static u32 InternalGetChildCount(ScriptGUILayout* instance);
		static MonoObject* InternalGetChild(ScriptGUILayout* instance, u32 index);
		static void InternalClear(ScriptGUILayout* instance);

		static void InternalCreateInstanceYFromScrollArea(MonoObject* instance, MonoObject* parentScrollArea);
	};

	/**	Interop class between C++ & CLR for GUIPanel.  */
	class BS_SCR_BE_EXPORT ScriptGUIPanel : public ScriptObject<ScriptGUIPanel>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "GUIPanel")

		/**	Creates a new managed GUIPanel that wraps the provided native GUIPanel. */
		static MonoObject* CreateFromExisting(GUIPanel* panel);

	private:
		ScriptGUIPanel(MonoObject* instance);
	};

	/**	Specialized ScriptGUILayout that is used only in GUI scroll areas. */
	class BS_SCR_BE_EXPORT ScriptGUIScrollAreaLayout : public ScriptGUILayout
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
