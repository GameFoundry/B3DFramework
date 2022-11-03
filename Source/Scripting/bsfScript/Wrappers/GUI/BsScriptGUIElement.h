//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "GUI/BsGUIOptions.h"
#include "GUI/BsGUIElementBase.h"

namespace bs
{
	/** @addtogroup bsfScript
	 *  @{
	 */

	/** Interop class between C++ & CLR for all elements inheriting from GUIElementBase. */
	class B3D_SCRIPT_INTEROP_EXPORT ScriptGUIElementBaseTBase : public ScriptObjectBase
	{
	public:
		ScriptGUIElementBaseTBase(MonoObject* instance);

		virtual ~ScriptGUIElementBaseTBase() {}

		/** Returns the managed version of this game object. */
		MonoObject* GetManagedInstance() const;

		/**	Returns the underlying GUIElementBase wrapped by this object. */
		GUIElementBase* GetGuiElement() const { return (GUIElementBase*)mElement; }

		/**	Destroys the underlying GUIElementBase. */
		virtual void Destroy() = 0;

		/**	Checks have we destroyed the underlying GUIElementBase. */
		bool IsDestroyed() const { return mIsDestroyed; }

		/**	Returns the parent interop object for a GUI layout or a GUI panel. */
		ScriptGUILayout* GetParent() const { return mParent; }

		/**	Sets an interop object for a GUI layout or a panel as this object's parent. */
		void SetParent(ScriptGUILayout* parent) { mParent = parent; }

	protected:
		/**
		 * Initializes the interop object with a previously initialized GUI element. You must call this before using this
		 * object.
		 */
		void Initialize(GUIElementBase* element);

		void OnManagedInstanceDeletedInternal(bool assemblyRefresh) override;
		void ClearManagedInstanceInternal() override;

		/**	Triggered when the focus changes for the underlying GUIElementBase. */
		static void OnFocusChanged(ScriptGUIElementBaseTBase* thisPtr, bool focus);

		bool mIsDestroyed = false;
		GUIElementBase* mElement = nullptr;
		ScriptGUILayout* mParent = nullptr;
		u32 mGCHandle = 0;
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
			: ScriptObject<Type, ScriptGUIElementBaseTBase>(instance)
		{
			this->Initialize(element);
		}
	};

	/** Interop class between C++ & CLR for all elements inheriting from GUIElement. */
	class B3D_SCRIPT_INTEROP_EXPORT ScriptGUIElementTBase : public ScriptGUIElementBaseTBase
	{
	public:
		ScriptGUIElementTBase(MonoObject* instance);

		virtual ~ScriptGUIElementTBase() {}

		void Destroy() override;
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
			: ScriptObject<Type, ScriptGUIElementTBase>(instance)
		{
			this->Initialize(element);
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
	class B3D_SCRIPT_INTEROP_EXPORT ScriptGUIElement : public ScriptObject<ScriptGUIElement>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "GUIElement")

	private:
		ScriptGUIElement(MonoObject* instance);

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void InternalDestroy(ScriptGUIElementBaseTBase* nativeInstance);
		static void InternalSetVisible(ScriptGUIElementBaseTBase* nativeInstance, bool visible);
		static void InternalSetActive(ScriptGUIElementBaseTBase* nativeInstance, bool active);
		static void InternalSetDisabled(ScriptGUIElementBaseTBase* nativeInstance, bool disabled);
		static void InternalSetFocus(ScriptGUIElementBaseTBase* nativeInstance, bool focus);
		static void InternalSetBlocking(ScriptGUIElementBaseTBase* nativeInstance, bool blocking);
		static void InternalSetAcceptsKeyFocus(ScriptGUIElementBaseTBase* nativeInstance, bool accepts);
		static bool InternalGetVisible(ScriptGUIElementBaseTBase* nativeInstance);
		static bool InternalGetActive(ScriptGUIElementBaseTBase* nativeInstance);
		static bool InternalGetDisabled(ScriptGUIElementBaseTBase* nativeInstance);
		static bool InternalGetBlocking(ScriptGUIElementBaseTBase* nativeInstance);
		static bool InternalGetAcceptsKeyFocus(ScriptGUIElementBaseTBase* nativeInstance);
		static MonoObject* InternalGetParent(ScriptGUIElementBaseTBase* nativeInstance);
		static void InternalGetBounds(ScriptGUIElementBaseTBase* nativeInstance, Rect2I* bounds);
		static void InternalSetBounds(ScriptGUIElementBaseTBase* nativeInstance, Rect2I* bounds);
		static void InternalGetVisibleBounds(ScriptGUIElementBaseTBase* nativeInstance, Rect2I* bounds);
		static void InternalGetScreenBounds(ScriptGUIElementBaseTBase* nativeInstance, Rect2I* bounds);
		static void InternalSetPosition(ScriptGUIElementBaseTBase* nativeInstance, i32 x, i32 y);
		static void InternalSetWidth(ScriptGUIElementBaseTBase* nativeInstance, u32 width);
		static void InternalSetFlexibleWidth(ScriptGUIElementBaseTBase* nativeInstance, u32 minWidth, u32 maxWidth);
		static void InternalSetHeight(ScriptGUIElementBaseTBase* nativeInstance, u32 height);
		static void InternalSetFlexibleHeight(ScriptGUIElementBaseTBase* nativeInstance, u32 minHeight, u32 maxHeight);
		static void InternalSetContextMenu(ScriptGUIElementBaseTBase* nativeInstance, ScriptContextMenu* contextMenu);
		static void InternalResetDimensions(ScriptGUIElementBaseTBase* nativeInstance);
		static MonoString* InternalGetStyle(ScriptGUIElementBaseTBase* nativeInstance);
		static void InternalSetStyle(ScriptGUIElementBaseTBase* nativeInstance, MonoString* style);

		typedef void(B3D_THUNKCALL* OnFocusChangedThunkDef)(MonoObject*, MonoException**);

	public:
		static OnFocusChangedThunkDef onFocusGainedThunk;
		static OnFocusChangedThunkDef onFocusLostThunk;
	};

	/** @} */
} // namespace bs
