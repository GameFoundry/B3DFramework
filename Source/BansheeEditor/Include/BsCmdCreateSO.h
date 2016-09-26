//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsEditorPrerequisites.h"
#include "BsEditorCommand.h"
#include "BsUndoRedo.h"

namespace BansheeEngine
{
	/** @addtogroup UndoRedo
	 *  @{
	 */

	/** A command used for undo/redo purposes. Creates a scene object and removes it as an undo operation. */
	class BS_ED_EXPORT CmdCreateSO : public EditorCommand
	{
	public:
		~CmdCreateSO();

		/**
		 * Creates a new scene object. Automatically registers the command with undo/redo system.
		 *
		 * @param[in]	name		Name of the scene object.
		 * @param[in]	flags		Optional creation flags for the scene object.
		 * @param[in]	description	Optional description of what exactly the command does.
		 * @return					Newly created scene object.
		 */
		static HSceneObject execute(const String& name, UINT32 flags, const WString& description = StringUtil::WBLANK);

		/** @copydoc EditorCommand::commit */
		void commit() override;

		/** @copydoc EditorCommand::revert */
		void revert() override;

	private:
		friend class UndoRedo;

		CmdCreateSO(const WString& description, const String& name, UINT32 flags);

		String mName;
		UINT32 mFlags;

		HSceneObject mSceneObject;
	};

	/** @} */
}