//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsEditorPrerequisites.h"
#include "BsEditorCommand.h"
#include "BsUndoRedo.h"

namespace bs
{
	/** @addtogroup UndoRedo
	 *  @{
	 */

	/**
	 * A command used for undo/redo purposes. It records a scene object parent change operations. It allows you to apply
	 * the parent change or revert the object to its original parent as needed.
	 */
	class BS_ED_EXPORT CmdReparentSO : public EditorCommand
	{
	public:
		/**
		 * Creates and executes the command on the provided scene object(s). Automatically registers the command with
		 * undo/redo system.
		 *
		 * @param[in]	sceneObjects	Object(s) to change the parent for.
		 * @param[in]	newParent		New parent for the provided objects.
		 * @param[in]	description		Optional description of what exactly the command does.
		 */
		static void execute(const Vector<HSceneObject>& sceneObjects, const HSceneObject& newParent, 
			const WString& description = StringUtil::WBLANK);

		/**
		 * Creates and executes the command on the provided scene object. Automatically registers the command with
		 * undo/redo system.
		 *
		 * @param[in]	sceneObject		Object to change the parent for.
		 * @param[in]	newParent		New parent for the provided objects.
		 * @param[in]	description		Optional description of what exactly the command does.
		 */
		static void execute(HSceneObject& sceneObject, const HSceneObject& newParent, 
			const WString& description = StringUtil::WBLANK);

		/** @copydoc EditorCommand::commit */
		void commit() override;

		/** @copydoc EditorCommand::revert */
		void revert() override;

	private:
		friend class UndoRedo;

		CmdReparentSO(const WString& description, const Vector<HSceneObject>& sceneObjects, const HSceneObject& newParent);

		Vector<HSceneObject> mSceneObjects;
		Vector<HSceneObject> mOldParents;
		HSceneObject mNewParent;
	};

	/** @} */
}