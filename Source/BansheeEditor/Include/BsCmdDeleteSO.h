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

	/** A command used for undo/redo purposes. Deletes a scene object and restores it as an undo operation. */
	class BS_ED_EXPORT CmdDeleteSO : public EditorCommand
	{
	public:
		~CmdDeleteSO();

		/**
		 * Creates and executes the command on the provided scene object. Automatically registers the command with 
		 * undo/redo system.
		 *
		 * @param[in]	sceneObject	Scene object to delete.
		 * @param[in]	description	Optional description of what exactly the command does.
		 */
		static void execute(const HSceneObject& sceneObject, const WString& description = StringUtil::WBLANK);

		/** @copydoc EditorCommand::commit */
		void commit() override;

		/** @copydoc EditorCommand::revert */
		void revert() override;

	private:
		friend class UndoRedo;

		CmdDeleteSO(const WString& description, const HSceneObject& sceneObject);

		/**
		 * Saves the state of the specified object, all of its children and components. Make sure to call clear() when you
		 * no longer need the data, or wish to call this method again.
		 */
		void recordSO(const HSceneObject& sceneObject);

		/**	Clears all the stored data and frees memory. */
		void clear();

		HSceneObject mSceneObject;
		EditorUtility::SceneObjProxy mSceneObjectProxy;

		UINT8* mSerializedObject;
		UINT32 mSerializedObjectSize;
		UINT64 mSerializedObjectParentId;
	};

	/** @} */
}