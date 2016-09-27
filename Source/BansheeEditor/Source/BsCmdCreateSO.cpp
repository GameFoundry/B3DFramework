//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsCmdCreateSO.h"
#include "BsSceneObject.h"

namespace BansheeEngine
{
	CmdCreateSO::CmdCreateSO(const WString& description, const String& name, UINT32 flags)
		:EditorCommand(description), mName(name), mFlags(flags)
	{

	}

	CmdCreateSO::~CmdCreateSO()
	{

	}

	HSceneObject CmdCreateSO::execute(const String& name, UINT32 flags, const WString& description)
	{
		// Register command and commit it
		CmdCreateSO* command = new (bs_alloc<CmdCreateSO>()) CmdCreateSO(description, name, flags);
		SPtr<CmdCreateSO> commandPtr = bs_shared_ptr(command);

		UndoRedo::instance().registerCommand(commandPtr);
		commandPtr->commit();

		return commandPtr->mSceneObject;
	}

	void CmdCreateSO::commit()
	{
		mSceneObject = SceneObject::create(mName, mFlags);
	}

	void CmdCreateSO::revert()
	{
		if (mSceneObject == nullptr)
			return;

		if (!mSceneObject.isDestroyed())
			mSceneObject->destroy(true);
		mSceneObject = nullptr;
	}
}