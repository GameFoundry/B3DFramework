//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsEditorPrerequisites.h"
#include "BsEditorCommand.h"
#include "BsUndoRedo.h"
#include "BsEditorUtility.h"

namespace BansheeEngine
{
	/** @addtogroup UndoRedo
	 *  @{
	 */

	/**
	 * A command used for undo/redo purposes. It records a state of the entire scene object at a specific point and allows
	 * you to restore it to its original values as needed.
	 */
	class BS_ED_EXPORT CmdRecordSO : public EditorCommand
	{
	public:
		~CmdRecordSO();

		/**
		 * Creates and executes the command on the provided scene object. Automatically registers the command with undo/redo
		 * system.
		 *
		 * @param[in]	sceneObject		Scene object to record.
		 * @param[in]	recordHierarchy	If true, all children of the provided scene object will be recorded as well.
		 * @param[in]	description		Optional description of what exactly the command does.
		 */
		static void execute(const HSceneObject& sceneObject, bool recordHierarchy = false, 
			const WString& description = StringUtil::WBLANK);

		/** @copydoc EditorCommand::commit */
		void commit() override;

		/** @copydoc EditorCommand::revert */
		void revert() override;

	private:
		friend class UndoRedo;

		CmdRecordSO(const WString& description, const HSceneObject& sceneObject, bool recordHierarchy);

		/**
		 * Saves the state of the specified object, all of its children and components. Make sure to call clear() when you
		 * no longer need the data, or wish to call this method again.
		 */
		void recordSO(const HSceneObject& sceneObject);

		/**	Clears all the stored data and frees memory. */
		void clear();

		HSceneObject mSceneObject;
		EditorUtility::SceneObjProxy mSceneObjectProxy;
		bool mRecordHierarchy;

		UINT8* mSerializedObject;
		UINT32 mSerializedObjectSize;
	};

	/** @} */
}