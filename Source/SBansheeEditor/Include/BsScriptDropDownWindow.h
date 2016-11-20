//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsScriptEditorPrerequisites.h"
#include "BsScriptObject.h"
#include "BsDropDownWindow.h"
#include "BsVector2I.h"

namespace bs
{
	/** @addtogroup ScriptInteropEditor
	 *  @{
	 */

	class ManagedDropDownWindow;

	/**	Interop class between C++ & CLR for types deriving from DropDownWindow. */
	class BS_SCR_BED_EXPORT ScriptDropDownWindow : public ScriptObject <ScriptDropDownWindow>
	{
	public:
		SCRIPT_OBJ(EDITOR_ASSEMBLY, "BansheeEditor", "DropDownWindow")

		~ScriptDropDownWindow();

	private:
		friend class ManagedDropDownWindow;

		ScriptDropDownWindow(ManagedDropDownWindow* window);

		/**	Triggered when the assembly refresh starts. */
		void onAssemblyRefreshStarted();

		/**	Triggered when the native DropDownWindow wrapped by this object gets closed. */
		void notifyWindowClosed();

		ManagedDropDownWindow* mDropDownWindow;
		HEvent mOnAssemblyRefreshStartedConn;

		static MonoField* guiPanelField;

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static MonoObject* internal_CreateInstance(MonoString* ns, MonoString* typeName, ScriptEditorWindow* parentWindow, Vector2I* position);
		static void internal_Close(ScriptDropDownWindow* nativeInstance);
		static UINT32 internal_GetWidth(ScriptDropDownWindow* nativeInstance);
		static void internal_SetWidth(ScriptDropDownWindow* nativeInstance, UINT32 value);
		static UINT32 internal_GetHeight(ScriptDropDownWindow* nativeInstance);
		static void internal_SetHeight(ScriptDropDownWindow* nativeInstance, UINT32 value);
		static void internal_ScreenToWindowPos(ScriptDropDownWindow* nativeInstance, Vector2I* position, Vector2I* windowPos);
		static void internal_WindowToScreenPos(ScriptDropDownWindow* nativeInstance, Vector2I* position, Vector2I* screenPos);
	};

	/**
	 * Managed implementation of a DropDownWindow. All managed drop down windows are implemented using this class, and the
	 * managed instance contains the specifics of each implementation.
	 */
	class BS_SCR_BED_EXPORT ManagedDropDownWindow : public DropDownWindow
	{
	public:
		ManagedDropDownWindow(const SPtr<RenderWindow>& parent, const SPtr<Camera>& camera,
			const Vector2I& position, MonoObject* managedInstance, UINT32 width, UINT32 height);
		~ManagedDropDownWindow();

		/** Initializes the drop down window with the interop object that owns the managed instance of this window. */
		void initialize(ScriptDropDownWindow* parent);

		/**	Called every frame. Triggers OnEditorUpdate method on the managed object. */
		void update() override;

		/**	Trigger the OnInitialize method on the managed object. */
		void triggerOnInitialize();

		/**	Trigger the OnDestroy method on the managed object. */
		void triggerOnDestroy();

		/**	Returns the managed instance of the drop down window implementation. */
		MonoObject* getManagedInstance() const { return mManagedInstance; }

		/**
		 * Reloads all the managed types and methods. Usually called right after construction or after assembly reload.
		 *
		 * @param[in]	windowClass	Managed class of the drop down window to retrieve the data for.
		 */
		void reloadMonoTypes(MonoClass* windowClass);

	private:
		friend class ScriptModalWindow;

		typedef void(__stdcall *OnInitializeThunkDef) (MonoObject*, MonoException**);
		typedef void(__stdcall *OnDestroyThunkDef) (MonoObject*, MonoException**);
		typedef void(__stdcall *UpdateThunkDef) (MonoObject*, MonoException**);

		String mNamespace;
		String mTypename;

		OnInitializeThunkDef mOnInitializeThunk;
		OnDestroyThunkDef mOnDestroyThunk;
		UpdateThunkDef mUpdateThunk;

		bool mIsInitialized;
		MonoObject* mManagedInstance;
		uint32_t mGCHandle;

		ScriptDropDownWindow* mScriptParent;
		ScriptGUILayout* mContentsPanel;
	};

	/** @} */
}