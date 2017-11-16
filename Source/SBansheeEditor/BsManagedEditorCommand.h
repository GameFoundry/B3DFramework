//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsScriptEditorPrerequisites.h"
#include "BsScriptObject.h"
#include "UndoRedo/BsEditorCommand.h"

namespace bs
{
	class CmdManaged;

	/** @addtogroup ScriptInteropEditor
	 *  @{
	 */

	/**	Interop class between C++ & CLR for CmdManaged. */
	class BS_SCR_BED_EXPORT ScriptCmdManaged : public ScriptObject <ScriptCmdManaged>
	{
	public:
		SCRIPT_OBJ(EDITOR_ASSEMBLY, "BansheeEditor", "UndoableCommand")

		~ScriptCmdManaged();

		/** Returns the internal command wrapped by this object. */
		SPtr<CmdManaged> getInternal() const { return mManagedCommand; }

	private:
		friend class CmdManaged;

		ScriptCmdManaged(MonoObject* instance);

		/** Triggers the Commit() method on the managed object instance. */
		void triggerCommit();

		/** Triggers the Revert() method on the managed object instance. */
		void triggerRevert();

		/**
		 * Allocates a GC handle that ensures the object doesn't get GC collected. Must eventually be followed by
		 * freeGCHandle().
		 */
		void allocGCHandle();

		/** Frees a GC handle previously allocated from allocGCHandle(). */
		void freeGCHandle();

		/** Notifies the script instance that the underlying managed command was destroyed. */
		void notifyCommandDestroyed();

		SPtr<CmdManaged> mManagedCommand;
		UINT32 mGCHandle = 0;
		bool mWeakHandle = true;

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void internal_CreateInstance(MonoObject* instance);

		static MonoMethod* sCommitMethod;
		static MonoMethod* sRevertMethod;
	};

	/** @} */

	/** @addtogroup SBansheeEditor
	 *  @{
	 */

	/** 
	 * A command used for undo/redo purposes. This particular implementation provides a generic overridable base to be
	 * implemented by script classes. 
	 */
	class BS_SCR_BED_EXPORT CmdManaged : public EditorCommand
	{
	public:
		~CmdManaged();

		/** @copydoc EditorCommand::commit */
		void commit() override;

		/** @copydoc EditorCommand::revert */
		void revert() override;

	private:
		friend class UndoRedo;
		friend class ScriptCmdManaged;

		CmdManaged(ScriptCmdManaged* scriptObj);

		/** @copydoc EditorCommand::commit */
		void onCommandAdded() override;

		/** @copydoc EditorCommand::commit */
		void onCommandRemoved() override;

		/** 
		 * Notifies the command the managed script object instance it is referencing has been destroyed. Normally when this
		 * happens the command should already be outside of the undo/redo stack, but we clear the instance just in case.
		 */
		void notifyScriptInstanceDestroyed();

		ScriptCmdManaged* mScriptObj;
		UINT32 mRefCount;

	};

	/** @} */
}