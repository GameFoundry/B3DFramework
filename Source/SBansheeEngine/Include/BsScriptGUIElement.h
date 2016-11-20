//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "BsGUIOptions.h"
#include "BsGUIElementBase.h"

namespace bs
{
	/** @addtogroup SBansheeEngine
	 *  @{
	 */

	/** Interop class between C++ & CLR for all elements inheriting from GUIElementBase. */
	class BS_SCR_BE_EXPORT ScriptGUIElementBaseTBase : public ScriptObjectBase
	{
	public:
		ScriptGUIElementBaseTBase(MonoObject* instance);
		virtual ~ScriptGUIElementBaseTBase() {}

		/**	Returns the underlying GUIElementBase wrapped by this object. */
		GUIElementBase* getGUIElement() const { return (GUIElementBase*)mElement; }

		/**	Destroys the underlying GUIElementBase. */
		virtual void destroy() = 0;

		/**	Checks have we destroyed the underlying GUIElementBase. */
		bool isDestroyed() const { return mIsDestroyed; }

		/**	Returns the parent interop object for a GUI layout or a GUI panel. */
		ScriptGUILayout* getParent() const { return mParent; }

		/**	Sets an interop object for a GUI layout or a panel as this object's parent. */
		void setParent(ScriptGUILayout* parent) { mParent = parent; }

	protected:
		/**
		 * Initializes the interop object with a previously initialized GUI element. You must call this before using this
		 * object.
		 */
		void initialize(GUIElementBase* element);

		/** @copydoc ScriptObjectBase::_onManagedInstanceDeleted */
		void _onManagedInstanceDeleted() override;

		/**	Triggered when the focus changes for the underlying GUIElementBase. */
		static void onFocusChanged(MonoObject* instance, bool focus);

		bool mIsDestroyed;
		GUIElementBase* mElement;
		ScriptGUILayout* mParent;
	};

	/**
	 * A more specialized implementation of ScriptGUIElementBaseTBase that references a specific GUI element type instead
	 * of the generic GUIElementBase.
	 */
	template <class Type>
	class TScriptGUIElementBase : public ScriptObject<Type, ScriptGUIElementBaseTBase>
	{
	public:
		virtual ~TScriptGUIElementBase() {}

	protected:
		TScriptGUIElementBase(MonoObject* instance, GUIElementBase* element)
			:ScriptObject<Type, ScriptGUIElementBaseTBase>(instance)
		{ 
			this->initialize(element);
		}
	};

	/** Interop class between C++ & CLR for all elements inheriting from GUIElement. */
	class BS_SCR_BE_EXPORT ScriptGUIElementTBase : public ScriptGUIElementBaseTBase
	{
	public:
		ScriptGUIElementTBase(MonoObject* instance);
		virtual ~ScriptGUIElementTBase() {}

		/** @copydoc ScriptGUIElementBaseTBase::destroy */
		void destroy() override;
	};

	/**
	 * A more specialized implementation of ScriptGUIElementTBase that references a specific GUI element type instead of
	 * the generic GUIElement.
	 */
	template <class Type>
	class TScriptGUIElement : public ScriptObject<Type, ScriptGUIElementTBase>
	{
	public:
		virtual ~TScriptGUIElement() {}

	protected:
		TScriptGUIElement(MonoObject* instance, GUIElementBase* element)
			:ScriptObject<Type, ScriptGUIElementTBase>(instance)
		{
			this->initialize(element);
		}
	};

	/** @} */

	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */

	/**
	 * Interop class between C++ & CLR for GUIElement. This includes only base methods belonging directly to GUIElement
	 * while specific GUI element implementations have their own interop classes.
	 */
	class BS_SCR_BE_EXPORT ScriptGUIElement : public ScriptObject<ScriptGUIElement>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "GUIElement")

	private:
		ScriptGUIElement(MonoObject* instance);

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void internal_destroy(ScriptGUIElementBaseTBase* nativeInstance);
		static void internal_setVisible(ScriptGUIElementBaseTBase* nativeInstance, bool visible);
		static void internal_setActive(ScriptGUIElementBaseTBase* nativeInstance, bool active);
		static void internal_setDisabled(ScriptGUIElementBaseTBase* nativeInstance, bool disabled);
		static void internal_setFocus(ScriptGUIElementBaseTBase* nativeInstance, bool focus);
		static void internal_setBlocking(ScriptGUIElementBaseTBase* nativeInstance, bool blocking);
		static bool internal_getVisible(ScriptGUIElementBaseTBase* nativeInstance);
		static bool internal_getActive(ScriptGUIElementBaseTBase* nativeInstance);
		static bool internal_getDisabled(ScriptGUIElementBaseTBase* nativeInstance);
		static bool internal_getBlocking(ScriptGUIElementBaseTBase* nativeInstance);
		static MonoObject* internal_getParent(ScriptGUIElementBaseTBase* nativeInstance);
		static void internal_getBounds(ScriptGUIElementBaseTBase* nativeInstance, Rect2I* bounds);
		static void internal_setBounds(ScriptGUIElementBaseTBase* nativeInstance, Rect2I* bounds);
		static void internal_getVisibleBounds(ScriptGUIElementBaseTBase* nativeInstance, Rect2I* bounds);
		static void internal_SetPosition(ScriptGUIElementBaseTBase* nativeInstance, INT32 x, INT32 y);
		static void internal_SetWidth(ScriptGUIElementBaseTBase* nativeInstance, UINT32 width);
		static void internal_SetFlexibleWidth(ScriptGUIElementBaseTBase* nativeInstance, UINT32 minWidth, UINT32 maxWidth);
		static void internal_SetHeight(ScriptGUIElementBaseTBase* nativeInstance, UINT32 height);
		static void internal_SetFlexibleHeight(ScriptGUIElementBaseTBase* nativeInstance, UINT32 minHeight, UINT32 maxHeight);
		static void internal_SetContextMenu(ScriptGUIElementBaseTBase* nativeInstance, ScriptContextMenu* contextMenu);
		static void internal_ResetDimensions(ScriptGUIElementBaseTBase* nativeInstance);
		static MonoString* internal_GetStyle(ScriptGUIElementBaseTBase* nativeInstance);
		static void internal_SetStyle(ScriptGUIElementBaseTBase* nativeInstance, MonoString* style);

		typedef void(__stdcall *OnFocusChangedThunkDef) (MonoObject*, MonoException**);

	public:
		static OnFocusChangedThunkDef onFocusGainedThunk;
		static OnFocusChangedThunkDef onFocusLostThunk;
	};

	/** @} */
}