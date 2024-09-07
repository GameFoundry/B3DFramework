//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "GUI/BsGUIOptions.h"
#include "GUI/BsGUIElement.h"
#include "GUI/BsGUIInteractable.h"

namespace bs
{
	/** @addtogroup bsfScript
	 *  @{
	 */

	/** Interop class between C++ & CLR for all elements inheriting from GUIElementBase. */
	class B3D_SCRIPT_INTEROP_EXPORT ScriptGUIElementBase : public ScriptObjectBase
	{
	public:
		ScriptGUIElementBase(MonoObject* instance);

		virtual ~ScriptGUIElementBase() {}

		/** Returns the managed version of this game object. */
		MonoObject* GetManagedInstance() const;

		/**	Returns the underlying GUIElementBase wrapped by this object. */
		GUIElement* GetGuiElement() const { return (GUIElement*)mElement; }

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
		void Initialize(GUIElement* element);

		/** Registers event callback to the provided @p element. */
		virtual void RegisterEvents(GUIElement* element) {} // TODO - This should be moved to ScriptObjectBase, but currently we don't have a common base class for all script exportable objects

		void OnManagedInstanceDeletedInternal(bool assemblyRefresh) override;
		void ClearManagedInstanceInternal() override;

		bool mIsDestroyed = false;
		GUIElement* mElement = nullptr;
		ScriptGUILayout* mParent = nullptr;
		u32 mGCHandle = 0;
	};

	/**
	 * A more specialized implementation of ScriptGUIElementBaseTBase that references a specific GUI element type instead
	 * of the generic GUIElementBase.
	 */
	template <class Type>
	class TScriptGUIElementBase : public ScriptObject<Type, ScriptGUIElementBase>
	{
	public:
		virtual ~TScriptGUIElementBase() {}

	protected:
		TScriptGUIElementBase(MonoObject* instance, GUIElement* element)
			: ScriptObject<Type, ScriptGUIElementBase>(instance)
		{
			this->Initialize(element);
		}
	};

	/** Interop class between C++ & CLR for all elements inheriting from GUIElement. */
	class B3D_SCRIPT_INTEROP_EXPORT ScriptGUIInteractableBase : public ScriptGUIElementBase
	{
	public:
		ScriptGUIInteractableBase(MonoObject* instance);

		virtual ~ScriptGUIInteractableBase() {}

		/**	Returns the underlying GUIElementBase wrapped by this object. */
		GUIInteractable* GetGuiInteractable() const { return static_cast<GUIInteractable*>(mElement); }

		void RegisterEvents(GUIElement* element) override;
		void Destroy() override;

		/**	Triggered when the focus changes for the underlying GUIElementBase. */
		static void OnFocusChanged(ScriptGUIElementBase* thisPtr, bool focus);

		typedef void(B3D_THUNKCALL* OnFocusChangedThunkDef)(MonoObject*, MonoException**);

		static OnFocusChangedThunkDef onFocusGainedThunk;
		static OnFocusChangedThunkDef onFocusLostThunk;
	};

	/**
	 * A more specialized implementation of ScriptGUIElementTBase that references a specific GUI element type instead of
	 * the generic GUIElement.
	 */
	template <class Type, class BaseType = ScriptGUIInteractableBase>
	class TScriptGUIInteractable : public ScriptObject<Type, BaseType>
	{
	public:
		virtual ~TScriptGUIInteractable() {}

	protected:
		TScriptGUIInteractable(MonoObject* instance, GUIElement* element)
			: ScriptObject<Type, BaseType>(instance)
		{
			this->Initialize(element);
		}
	};

	/** @} */

	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */

	/** Interop class between C++ & CLR for GUIElement. */
	class B3D_SCRIPT_INTEROP_EXPORT ScriptGUIElement : public TScriptGUIElementBase<ScriptGUIElement>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "GUIElement")

	private:
		ScriptGUIElement(MonoObject* instance);

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void InternalDestroy(ScriptGUIElementBase* nativeInstance);
		static void InternalSetVisible(ScriptGUIElementBase* nativeInstance, bool visible);
		static bool InternalGetVisible(ScriptGUIElementBase* nativeInstance);
		static void InternalSetActive(ScriptGUIElementBase* nativeInstance, bool active);
		static bool InternalGetActive(ScriptGUIElementBase* nativeInstance);
		static void InternalSetDisabled(ScriptGUIElementBase* nativeInstance, bool disabled);
		static bool InternalGetDisabled(ScriptGUIElementBase* nativeInstance);
		static MonoObject* InternalGetParent(ScriptGUIElementBase* nativeInstance);
		static void InternalGetBounds(ScriptGUIElementBase* nativeInstance, Rect2I* bounds);
		static void InternalSetBounds(ScriptGUIElementBase* nativeInstance, Rect2I* bounds);
		static void InternalGetVisibleBounds(ScriptGUIElementBase* nativeInstance, Rect2I* bounds);
		static void InternalGetScreenBounds(ScriptGUIElementBase* nativeInstance, Rect2I* bounds);
		static void InternalSetPosition(ScriptGUIElementBase* nativeInstance, i32 x, i32 y);
		static void InternalSetWidth(ScriptGUIElementBase* nativeInstance, u32 width);
		static void InternalSetHeight(ScriptGUIElementBase* nativeInstance, u32 height);
		static void InternalSetFlexibleWidth(ScriptGUIElementBase* nativeInstance, u32 minWidth, u32 maxWidth);
		static void InternalSetFlexibleHeight(ScriptGUIElementBase* nativeInstance, u32 minHeight, u32 maxHeight);
		static void InternalResetDimensions(ScriptGUIElementBase* nativeInstance);
	};


	/** Interop class between C++ & CLR for GUIInteractable. */
	class B3D_SCRIPT_INTEROP_EXPORT ScriptGUIInteractable : public TScriptGUIInteractable<ScriptGUIInteractable>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "GUIInteractable")

	private:
		ScriptGUIInteractable(MonoObject* instance);

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void InternalSetFocus(ScriptGUIInteractableBase* nativeInstance, bool focus);
		static void InternalSetBlocking(ScriptGUIInteractableBase* nativeInstance, bool blocking);
		static bool InternalGetBlocking(ScriptGUIInteractableBase* nativeInstance);
		static void InternalSetAcceptsKeyFocus(ScriptGUIInteractableBase* nativeInstance, bool accepts);
		static bool InternalGetAcceptsKeyFocus(ScriptGUIInteractableBase* nativeInstance);
		static void InternalSetTint(ScriptGUIInteractableBase* nativeInstance, Color* tint);
		static void InternalSetContextMenu(ScriptGUIInteractableBase* nativeInstance, ScriptContextMenu* contextMenu);
		static MonoString* InternalGetStyle(ScriptGUIInteractableBase* nativeInstance);
		static void InternalSetStyle(ScriptGUIInteractableBase* nativeInstance, MonoString* style);
	};

	/** @} */
} // namespace bs
