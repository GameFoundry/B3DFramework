//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsCmdCloneSO.h"
#include "BsSceneObject.h"

namespace BansheeEngine
{
	CmdCloneSO::CmdCloneSO(const WString& description, const Vector<HSceneObject>& originals)
		:EditorCommand(description), mOriginals(originals)
	{

	}

	CmdCloneSO::~CmdCloneSO()
	{

	}

	HSceneObject CmdCloneSO::execute(const HSceneObject& sceneObject, const WString& description)
	{
		// Register command and commit it
		CmdCloneSO* command = new (bs_alloc<CmdCloneSO>()) CmdCloneSO(description, { sceneObject });
		SPtr<CmdCloneSO> commandPtr = bs_shared_ptr(command);

		UndoRedo::instance().registerCommand(commandPtr);
		commandPtr->commit();

		if (commandPtr->mClones.size() > 0)
			return commandPtr->mClones[0];

		return HSceneObject();
	}

	Vector<HSceneObject> CmdCloneSO::execute(const Vector<HSceneObject>& sceneObjects, const WString& description)
	{
		// Register command and commit it
		CmdCloneSO* command = new (bs_alloc<CmdCloneSO>()) CmdCloneSO(description, sceneObjects);
		SPtr<CmdCloneSO> commandPtr = bs_shared_ptr(command);

		UndoRedo::instance().registerCommand(commandPtr);
		commandPtr->commit();

		return commandPtr->mClones;
	}

	void CmdCloneSO::commit()
	{
		mClones.clear();

		for (auto& original : mOriginals)
		{
			if (!original.isDestroyed())
				mClones.push_back(original->clone());
		}
	}

	void CmdCloneSO::revert()
	{
		for (auto& clone : mClones)
		{
			if (!clone.isDestroyed())
				clone->destroy(true);
		}

		mClones.clear();
	}
}