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

	/**	Interop class between C++ & CLR for GUILayout derived classes. */
	class BS_SCR_BE_EXPORT ScriptGUILayout : public TScriptGUIElementBase<ScriptGUILayout>
	{
	protected:
		/**	Contains information about an interop object that represents a child of the layout. */
		struct ChildInfo
		{
			ScriptGUIElementBaseTBase* element;
			uint32_t gcHandle;
		};

	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "GUILayout")

		virtual ~ScriptGUILayout() { }

		/**	Returns the internal wrapped GUILayout object. */
		GUILayout* getInternalValue() const { return mLayout; }

		/**	Registers a new managed child GUI element and inserts it at the end of the layout. */
		void addChild(ScriptGUIElementBaseTBase* element);

		/**	Registers a new managed child GUI element and inserts it at a specific location in the layout. */
		void insertChild(UINT32 index, ScriptGUIElementBaseTBase* element);

		/**	Removes a managed GUI element from the layout. */
		void removeChild(ScriptGUIElementBaseTBase* element);

		/**
		 * @copydoc	ScriptGUIElementBaseTBase::destroy
		 *
		 * Destroys the layout and all of its managed children.
		 */
		void destroy() override;
	protected:
		friend class ScriptGUIPanel;

		/**
		 * Constructor.
		 *
		 * @param[in]	instance	Managed GUILayout instance.
		 * @param[in]	layout  	Native GUILayout instance.
		 * @param[in]	ownsNative	Does this object own the native instance. If it does it will destroy the
		 * 							object when destroy() is called, otherwise it is up to the caller to destroy it.
		 */
		ScriptGUILayout(MonoObject* instance, GUILayout* layout, bool ownsNative = true);

		GUILayout* mLayout;
		Vector<ChildInfo> mChildren;

		bool mOwnsNative;

	private:
		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void internal_createInstanceX(MonoObject* instance, MonoArray* guiOptions);
		static void internal_createInstanceY(MonoObject* instance, MonoArray* guiOptions);
		static void internal_createInstancePanel(MonoObject* instance, INT16 depth, UINT16 depthRangeMin, UINT32 depthRangeMax, MonoArray* guiOptions);
		static void internal_addElement(ScriptGUILayout* instance, ScriptGUIElementBaseTBase* element);
		static void internal_insertElement(ScriptGUILayout* instance, UINT32 index, ScriptGUIElementBaseTBase* element);
		static UINT32 internal_getChildCount(ScriptGUILayout* instance);
		static MonoObject* internal_getChild(ScriptGUILayout* instance, UINT32 index);
		static void internal_clear(ScriptGUILayout* instance);

		static void internal_createInstanceYFromScrollArea(MonoObject* instance, MonoObject* parentScrollArea);
	};

	/**	Interop class between C++ & CLR for GUIPanel.  */
	class BS_SCR_BE_EXPORT ScriptGUIPanel : public ScriptObject<ScriptGUIPanel>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "GUIPanel")

		/**	Creates a new managed GUIPanel that wraps the provided native GUIPanel. */
		static MonoObject* createFromExisting(GUIPanel* panel);
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
		void destroy() override;

	private:
		friend class ScriptGUIScrollArea;

		ScriptGUIScrollArea* mParentScrollArea;
	};

	/** @} */
}