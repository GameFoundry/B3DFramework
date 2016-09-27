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

	/**
	 * A command used for undo/redo purposes. It records a value of a GUI input field (specified by template type) and
	 * allows you to apply or revert a change to that field as needed.
	 */
	template <class InputFieldType, class ValueType>
	class BS_ED_EXPORT CmdInputFieldValueChange : public EditorCommand
	{
	public:
		/**
		 * Creates and executes the command on the provided object and field. Automatically registers the command with
		 * undo/redo system.
		 *
		 * @param[in]	inputField	Input field to modify the value on.
		 * @param[in]	value		New value for the field.
		 * @param[in]	description	Optional description of what exactly the command does.
		 */
		static void execute(InputFieldType* inputField, const ValueType& value, 
			const WString& description = StringUtil::WBLANK)
		{
			CmdInputFieldValueChange* command = 
				new (bs_alloc<CmdInputFieldValueChange>()) CmdInputFieldValueChange(description, inputField, value);
			SPtr<CmdInputFieldValueChange> commandPtr = bs_shared_ptr(command);

			UndoRedo::instance().registerCommand(commandPtr);
			commandPtr->commit();
		}

		/** @copydoc EditorCommand::commit */
		void commit() override
		{
			mInputField->_setValue(mNewValue, true);
		}

		/** @copydoc EditorCommand::revert */
		void revert() override
		{
			mInputField->_setValue(mOldValue, true);
		}

	private:
		friend class UndoRedo;

		CmdInputFieldValueChange(const WString& description, InputFieldType* inputField, const ValueType& value)
			:EditorCommand(description), mOldValue(inputField->getValue()), mNewValue(value), mInputField(inputField)
		{ }

		ValueType mOldValue;
		ValueType mNewValue;
		InputFieldType* mInputField;
	};

	/** @} */
}