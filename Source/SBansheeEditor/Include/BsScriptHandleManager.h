//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsScriptEditorPrerequisites.h"
#include "BsHandleManager.h"

namespace bs
{
	/** @addtogroup SBansheeEditor
	 *  @{
	 */

	/**
	 * Renders, updates and manipulates handles declared in managed code. Managed code handles have a [CustomHandle]
	 * attribute and must implement BansheeEditor.Handle.
	 */
	class BS_SCR_BED_EXPORT ScriptHandleManager : public HandleManager
	{
		/** Contains data about a manage type that draws and handles interaction with a custom handle. */
		struct CustomHandleData
		{
			MonoClass* handleType;
			MonoClass* componentType;
			MonoMethod* ctor;
		};

		/**
		 * Data about an active instance of a managed custom handle object. Active handle means its scene object is
		 * currently selected and the handle is displayed and can be interacted with.
		 */
		struct ActiveCustomHandleData
		{
			MonoObject* object;
			uint32_t gcHandle;
		};

		/**
		 * Data about all active managed custom handle objects for a specific scene object. Active handle means its 
		 * scene object is currently selected and the handle is displayed and can be interacted with.
		 */
		struct ActiveCustomHandles
		{
			HSceneObject selectedObject;
			Vector<ActiveCustomHandleData> handles;
		};

	public:
		ScriptHandleManager(ScriptAssemblyManager& scriptObjectManager);
		~ScriptHandleManager();

	protected:
		/** @copydoc HandleManager::triggerPreInput */
		void triggerPreInput() override;

		/** @copydoc HandleManager::triggerPostInput */
		void triggerPostInput() override;

		/** @copydoc HandleManager::queueDrawCommands */
		void queueDrawCommands() override;

		/**   
		 * Clears references to all managed types and objects. Must be called before loadAssemblyData() if 
		 * loadAssemblyData() was called previously.
		 */
		void clearAssemblyData();

		/**
		 * Loads internal managed assembly types and finds all custom handle classes. Must be called after construction and
		 * after assembly reload.
		 */
		void loadAssemblyData();

		/**
		 * Checks is the provided type a valid custom handle class. Custom handles must have a [CustomHandle] attribute and
		 * must implement BansheeEditor.Handle.
		 *
		 * @param[in]	type			Type to check.
		 * @param[in]	componentType	Component type for which the handle should be displayed for. Handle will not
		 *								be displayed unless a component of this type is selected. Only valid if method
		 *								returns true.
		 * @param[in]	ctor			Constructor method for the handle type. Only valid if method returns true.
		 * @return						True if the type is a valid custom handle type.
		 */
		bool isValidHandleType(MonoClass* type, MonoClass*& componentType, MonoMethod*& ctor) const;

		/**
		 * Triggers the PreInput method on the provided Handle object. Pre input happens before any handles are selected
		 * or moved and allows you to position the handles or prepare them in some other way.
		 */
		void callPreInput(MonoObject* instance);

		/**
		 * Triggers the PostInput method on the provided Handle object. Post input happens after we know in what way has the
		 * user interacted with the handles this frame.
		 */
		void callPostInput(MonoObject* instance);

		/**
		 * Triggers the Draw method on the provided Handle object. Draw allows you to draw the visual representation of the
		 * handles. Called after PostInput.
		 */
		void callDraw(MonoObject* instance);

		/**
		 * Triggers the Destroy method on the provided Handle object. Destroy is called when the handle is no longer being
		 * displayed.
		 */
		void callDestroy(MonoObject* instance);

		ScriptAssemblyManager& mScriptObjectManager;
		HEvent mDomainUnloadConn;
		HEvent mDomainLoadConn;

		Map<String, CustomHandleData> mHandles;

		ActiveCustomHandles mActiveHandleData;
		Vector<MonoClass*> mGlobalHandlesToCreate;
		Vector<ActiveCustomHandleData> mActiveGlobalHandles;

		MonoObject* mDefaultHandleManager = nullptr;
		uint32_t mDefaultHandleManagerGCHandle = 0;

		MonoClass* mCustomHandleAttribute = nullptr;
		MonoField* mTypeField = nullptr;
		MonoClass* mHandleBaseClass = nullptr;
		MonoClass* mDefaultHandleManagerClass = nullptr;

		typedef void(__stdcall *DestroyThunkDef) (MonoObject*, MonoException**);

		MonoMethod* mPreInputMethod;
		MonoMethod* mPostInputMethod;
		MonoMethod* mDrawMethod;
		DestroyThunkDef mDestroyThunk;
	};

	/** @} */
}