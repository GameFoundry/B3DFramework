//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsUndoRedo.h"
#include "BsEditorCommand.h"

namespace BansheeEngine
{
	const UINT32 UndoRedo::MAX_STACK_ELEMENTS = 1000;

	UndoRedo::UndoRedo()
		: mUndoStack(nullptr), mRedoStack(nullptr), mUndoStackPtr(0), mUndoNumElements(0), mRedoStackPtr(0)
		, mRedoNumElements(0), mNextCommandId(0)
	{
		mUndoStack = bs_newN<SPtr<EditorCommand>>(MAX_STACK_ELEMENTS);
		mRedoStack = bs_newN<SPtr<EditorCommand>>(MAX_STACK_ELEMENTS);
	}

	UndoRedo::~UndoRedo()
	{
		clear();

		bs_deleteN(mUndoStack, MAX_STACK_ELEMENTS);
		bs_deleteN(mRedoStack, MAX_STACK_ELEMENTS);
	}

	void UndoRedo::undo()
	{
		if(mUndoNumElements == 0)
			return;

		SPtr<EditorCommand> command = removeLastFromUndoStack();
		
		mRedoStackPtr = (mRedoStackPtr + 1) % MAX_STACK_ELEMENTS;
		mRedoStack[mRedoStackPtr] = command;
		mRedoNumElements = std::min(mRedoNumElements + 1, MAX_STACK_ELEMENTS);

		command->revert();
	}

	void UndoRedo::redo()
	{
		if(mRedoNumElements == 0)
			return;

		SPtr<EditorCommand> command = mRedoStack[mRedoStackPtr];
		mRedoStackPtr = (mRedoStackPtr - 1) % MAX_STACK_ELEMENTS;
		mRedoNumElements--;

		addToUndoStack(command);

		command->commit();
	}

	void UndoRedo::pushGroup(const String& name)
	{
		mGroups.push(GroupData());
		GroupData& newGroup = mGroups.top();

		newGroup.name = name;
		newGroup.numEntries = 0;

		clearRedoStack();
	}

	void UndoRedo::popGroup(const String& name)
	{
		if(mGroups.empty())
			BS_EXCEPT(InvalidStateException, "Attempting to pop an UndoRedo group that doesn't exist: " + name);

		GroupData& topGroup = mGroups.top();
		if(topGroup.name != name)
			BS_EXCEPT(InvalidStateException, "Attempting to pop invalid UndoRedo group. Got: " + name + ". Expected: " + topGroup.name);

		for(UINT32 i = 0; i < topGroup.numEntries; i++)
		{
			mUndoStack[mUndoStackPtr] = SPtr<EditorCommand>();
			mUndoStackPtr = (mUndoStackPtr - 1) % MAX_STACK_ELEMENTS;
			mUndoNumElements--;
		}

		mGroups.pop();
		clearRedoStack();
	}

	void UndoRedo::registerCommand(const SPtr<EditorCommand>& command)
	{
		command->mId = mNextCommandId++;
		addToUndoStack(command);

		clearRedoStack();
	}

	UINT32 UndoRedo::getTopCommandId() const
	{
		if (mUndoNumElements > 0)
			return mUndoStack[mUndoStackPtr]->mId;

		return 0;
	}

	void UndoRedo::popCommand(UINT32 id)
	{
		UINT32 undoPtr = mUndoStackPtr;
		for (UINT32 i = 0; i < mUndoNumElements; i++)
		{
			if (mUndoStack[undoPtr]->mId == id)
			{
				for (UINT32 j = mUndoNumElements - i; j < (mUndoNumElements - 1); j++)
				{
					UINT32 nextUndoPtr = (undoPtr + 1) % MAX_STACK_ELEMENTS;

					std::swap(mUndoStack[undoPtr], mUndoStack[nextUndoPtr]);
					undoPtr = nextUndoPtr;
				}

				mUndoStackPtr = (mUndoStackPtr - 1) % MAX_STACK_ELEMENTS;
				mUndoNumElements--;
				break;
			}

			undoPtr = (undoPtr - 1) % MAX_STACK_ELEMENTS;
		}

		UINT32 redoPtr = mRedoStackPtr;
		for (UINT32 i = 0; i < mRedoNumElements; i++)
		{
			if (mRedoStack[redoPtr]->mId == id)
			{
				for (UINT32 j = mRedoNumElements - i; j < (mRedoNumElements - 1); j++)
				{
					UINT32 nextRedoPtr = (redoPtr + 1) % MAX_STACK_ELEMENTS;

					std::swap(mRedoStack[redoPtr], mRedoStack[nextRedoPtr]);
					redoPtr = nextRedoPtr;
				}

				mRedoStackPtr = (mRedoStackPtr - 1) % MAX_STACK_ELEMENTS;
				mRedoNumElements--;
				break;
			}

			redoPtr = (redoPtr - 1) % MAX_STACK_ELEMENTS;
		}
	}

	void UndoRedo::clear()
	{
		clearUndoStack();
		clearRedoStack();
	}

	SPtr<EditorCommand> UndoRedo::removeLastFromUndoStack()
	{
		SPtr<EditorCommand> command = mUndoStack[mUndoStackPtr];

		mUndoStack[mUndoStackPtr] = SPtr<EditorCommand>();
		mUndoStackPtr = (mUndoStackPtr - 1) % MAX_STACK_ELEMENTS;
		mUndoNumElements--;

		if(!mGroups.empty())
		{
			GroupData& topGroup = mGroups.top();

			if(topGroup.numEntries == 0)
			{
				BS_EXCEPT(InvalidStateException, "Removing an element from UndoRedo stack while in an " \
					"invalid UndoRedo group. Current group: " + topGroup.name);
			}

			topGroup.numEntries--;
		}

		return command;
	}

	void UndoRedo::addToUndoStack(const SPtr<EditorCommand>& command)
	{
		mUndoStackPtr = (mUndoStackPtr + 1) % MAX_STACK_ELEMENTS;
		mUndoStack[mUndoStackPtr] = command;
		mUndoNumElements = std::min(mUndoNumElements + 1, MAX_STACK_ELEMENTS);

		if(!mGroups.empty())
		{
			GroupData& topGroup = mGroups.top();
			topGroup.numEntries = std::min(topGroup.numEntries + 1, MAX_STACK_ELEMENTS);
		}
	}

	void UndoRedo::clearUndoStack()
	{
		while(mUndoNumElements > 0)
		{
			mUndoStack[mUndoStackPtr] = SPtr<EditorCommand>();
			mUndoStackPtr = (mUndoStackPtr - 1) % MAX_STACK_ELEMENTS;
			mUndoNumElements--;
		}

		while(!mGroups.empty())
			mGroups.pop();
	}

	void UndoRedo::clearRedoStack()
	{
		while(mRedoNumElements > 0)
		{
			mRedoStack[mRedoStackPtr] = SPtr<EditorCommand>();
			mRedoStackPtr = (mRedoStackPtr - 1) % MAX_STACK_ELEMENTS;
			mRedoNumElements--;
		}
	}
}