//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsScriptEditorPrerequisites.h"
#include "BsScriptObject.h"
#include "BsEditorWidget.h"
#include "BsVector2I.h"

namespace BansheeEngine
{
	/** @addtogroup ScriptInteropEditor
	 *  @{
	 */

	class ScriptEditorWidget;

	/**	Interop class between C++ & CLR for ScriptEditorWidget. */
	class BS_SCR_BED_EXPORT ScriptEditorWindow : public ScriptObject<ScriptEditorWindow, PersistentScriptObjectBase>
	{
		/**	Contains data about the managed handle to an editor window. */
		struct EditorWindowHandle
		{
			uint32_t gcHandle;
			ScriptEditorWindow* nativeObj;
		};
	public:
		SCRIPT_OBJ(EDITOR_ASSEMBLY, "BansheeEditor", "EditorWindow")

		~ScriptEditorWindow();

		/**	Returns the internal wrapped editor widget. */
		EditorWidgetBase* getEditorWidget() const;

		/**	Checks has the native widget been destroyed. */
		bool isDestroyed() const { return mIsDestroyed; }

		/**
		 * Finds all editor window implementations in managed assemblies and registers them with the editor widget system.
		 */
		static void registerManagedEditorWindows();

		/**
		 * Removes all editor widgets registered previously with registerManagedEditorWindows(). Useful during assembly
		 * refresh when editor window implementations might be added/removed.
		 */
		static void clearRegisteredEditorWindow();

	private:
		friend class ScriptEditorWidget;

		ScriptEditorWindow(ScriptEditorWidget* editorWidget);

		/**	Triggered when the native editor widget is resized. */
		void onWidgetResized(UINT32 width, UINT32 height);

		/**	Triggered when the native editor widget gains or loses focus. */
		void onFocusChanged(bool inFocus);

		/**	Triggered when assembly refresh has started. */
		void onAssemblyRefreshStarted();

		/** @copydoc ScriptObjectBase::_onManagedInstanceDeleted */
		void _onManagedInstanceDeleted() override;

		/** @copydoc ScriptObjectBase::beginRefresh */
		ScriptObjectBackup beginRefresh() override;

		/** @copydoc ScriptObjectBase::endRefresh */
		void endRefresh(const ScriptObjectBackup& backupData) override;

		/** @copydoc ScriptObjectBase::_createManagedInstance */
		MonoObject* _createManagedInstance(bool construct) override;

		String mName;
		ScriptEditorWidget* mEditorWidget;
		HEvent mOnWidgetResizedConn;
		HEvent mOnFocusChangedConn;
		HEvent mOnAssemblyRefreshStartedConn;
		bool mRefreshInProgress;
		bool mIsDestroyed;

		static MonoMethod* onResizedMethod;
		static MonoMethod* onFocusChangedMethod;
		static MonoField* guiPanelField;
		static MonoField* undoRedoField;

		// Global editor window management methods

		/**
		 * Registers a newly created editor window interop object and adds it to a list of currently active editor windows.
		 */
		static void registerScriptEditorWindow(ScriptEditorWindow* editorWindow);

		/**
		 * Removes a window from the active editor window list.
		 *
		 * @param[in]	windowTypeName	Name of the window type. Provided by EditorWidget::getName.
		 */
		static void unregisterScriptEditorWindow(const String& windowTypeName);

		/**	Callback that is triggered when user requests a widget to be opened. */
		static EditorWidgetBase* openEditorWidgetCallback(String ns, String type, UINT32 width, UINT32 height, 
			bool localUndoRedo, EditorWidgetContainer& parentContainer);

		static UnorderedMap<String, EditorWindowHandle> OpenScriptEditorWindows;
		static Vector<String> AvailableWindowTypes;

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static MonoObject* internal_createOrGetInstance(MonoString* ns, MonoString* typeName);
		static MonoObject* internal_getInstance(MonoString* ns, MonoString* typeName);
		static MonoArray* internal_getAllWindows();

		static bool internal_hasFocus(ScriptEditorWindow* thisPtr);
		static void internal_setFocus(ScriptEditorWindow* thisPtr, bool focus);
		static bool internal_setActive(ScriptEditorWindow* thisPtr);
		static bool internal_isActive(ScriptEditorWindow* thisPtr);
		static bool internal_isPointerHovering(ScriptEditorWindow* thisPtr);
		static void internal_screenToWindowPos(ScriptEditorWindow* thisPtr, Vector2I* screenPos, Vector2I* windowPos);
		static void internal_windowToScreenPos(ScriptEditorWindow* thisPtr, Vector2I* windowPos, Vector2I* screenPos);
		static UINT32 internal_getWidth(ScriptEditorWindow* thisPtr);
		static UINT32 internal_getHeight(ScriptEditorWindow* thisPtr);
		static void internal_getBounds(ScriptEditorWindow* thisPtr, Rect2I* bounds);
	};

	/** @} */
	/** @addtogroup SBansheeEditor
	 *  @{
	 */

	/**
	 * Editor widget implementation that handles managed editor window implementations. Each implementation is wrapped in
	 * this object and then managed by its parent interop object of ScriptEditorWindow type.
	 */
	class BS_SCR_BED_EXPORT ScriptEditorWidget : public EditorWidgetBase
	{
	public:
		/**
		 * Constructs a new managed widget.
		 *
		 * @param[in]	ns				Namespace of the widget type.
		 * @param[in]	type			Name of the widget type.
		 * @param[in]	defaultWidth	Default width of the widget when initially created.
		 * @param[in]	defaultHeight	Default height of the widget when initially created.
		 * @param[in]	localUndoRedo	Determines should the window use a local undo/redo stack instead of the global one.
		 * @param[in]	parentContainer	Container to initially dock the widget in.
		 */
		ScriptEditorWidget(const String& ns, const String& type, UINT32 defaultWidth, 
			UINT32 defaultHeight, bool localUndoRedo, EditorWidgetContainer& parentContainer);
		~ScriptEditorWidget();

		/**
		 * Attempts to create a managed instance for the editor window described by the type provided upon construction.
		 *
		 * @return	True if the managed instance was created.
		 */
		bool createManagedInstance();

		/** Checks has the OnInitialize method been called yet. */
		bool isInitialized() const { return mIsInitialized; }

		/** @copydoc EditorWidgetBase::update  */
		void update() override;

		/**
		 * Loads all required mono methods, fields and types required for operation of this object. Must be called after
		 * construction and after assembly refresh.
		 *
		 * @param[in]	windowClass		Mono class to load the types from.
		 */
		void reloadMonoTypes(MonoClass* windowClass);

		/**	Triggers OnInitialize callbacks on the managed instance. */
		void triggerOnInitialize();

		/**	Triggers OnDestroy callbacks on the managed instance. */
		void triggerOnDestroy();

		/**
		 * Sets the parent interop object that handles part of the communication between this object and the managed
		 * instance.
		 */
		void setScriptOwner(ScriptEditorWindow* owner) { mScriptOwner = owner; }

		/**	Returns the managed instance for the editor window represented by this object. */
		MonoObject* getManagedInstance() const { return mManagedInstance; }

	private:
		typedef void(__stdcall *OnInitializeThunkDef) (MonoObject*, MonoException**);
		typedef void(__stdcall *OnDestroyThunkDef) (MonoObject*, MonoException**);
		typedef void(__stdcall *UpdateThunkDef) (MonoObject*, MonoException**);

		String mNamespace;
		String mTypename;

		OnInitializeThunkDef mOnInitializeThunk;
		OnDestroyThunkDef mOnDestroyThunk;
		UpdateThunkDef mUpdateThunk;
		MonoObject* mManagedInstance;
		MonoMethod* mGetDisplayName;

		ScriptEditorWindow* mScriptOwner;
		ScriptGUILayout* mContentsPanel;
		bool mIsInitialized;
		bool mHasLocalUndoRedo;
	};

	/** @} */
}