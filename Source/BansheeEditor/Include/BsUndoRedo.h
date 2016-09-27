//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsEditorPrerequisites.h"
#include "BsModule.h"

namespace BansheeEngine
{
	/** @addtogroup UndoRedo
	 *  @{
	 */

	/**	Provides functionality to undo or redo recently performed operations in the editor. */
	class BS_ED_EXPORT UndoRedo : public Module<UndoRedo>
	{
		/**
		 * Contains data about a single undo/redo group. Groups allow you to create context sensitive undo/redo operations.
		 */
		struct GroupData
		{
			String name;
			UINT32 numEntries;
		};

	public:
		UndoRedo();
		~UndoRedo();

		/**	Executes the last command on the undo stack, undoing its operations. */
		void undo();

		/** Executes the last command on the redo stack (last command we called undo on), re-applying its operation. */
		void redo();

		/**
		 * Creates a new undo/redo group. All new commands will be registered to this group. You may remove the group and 
		 * all of its commands by calling popGroup().
		 *
		 * For example you might require global editor-wide undo/redo operations, and also more specific ones like input in
		 * an input box. When the user is done with the input box you no longer require its undo operations and you may use
		 * groups to easily remove them.
		 *
		 * @param[in]	name	Unique name for the group.
		 */
		void pushGroup(const String& name);

		/**
		 * Removes all the command registered to the current undo/redo group.
		 *
		 * @param[in]	name	Unique name for the group.
		 */
		void popGroup(const String& name);

		/**	Registers a new undo command. */
		void registerCommand(const SPtr<EditorCommand>& command);

		/**	Returns the unique identifier for the command on top of the undo stack. */
		UINT32 getTopCommandId() const;

		/**
		 * Removes a command from the undo/redo list, without executing it.
		 *
		 * @param[in]	id	Identifier of the command returned by getTopCommandIdx().
		 */
		void popCommand(UINT32 id);

		/**	Resets the undo/redo stacks. */
		void clear();

	private:
		/**	Removes the last undo command from the undo stack, and returns it. */
		SPtr<EditorCommand> removeLastFromUndoStack();

		/**	Adds a new command to the undo stack. */
		void addToUndoStack(const SPtr<EditorCommand>& command);

		/**	Removes all entries from the undo stack. */
		void clearUndoStack();

		/**	Removes all entries from the redo stack. */
		void clearRedoStack();

		static const UINT32 MAX_STACK_ELEMENTS;

		SPtr<EditorCommand>* mUndoStack;
		SPtr<EditorCommand>* mRedoStack;

		UINT32 mUndoStackPtr;
		UINT32 mUndoNumElements;

		UINT32 mRedoStackPtr;
		UINT32 mRedoNumElements;

		UINT32 mNextCommandId;

		Stack<GroupData> mGroups;
	};

	/** @} */
}
