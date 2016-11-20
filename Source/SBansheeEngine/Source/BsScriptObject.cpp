//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScriptObject.h"
#include "BsScriptObjectManager.h"
#include "BsMonoManager.h"
#include "BsCrashHandler.h"
#include "BsMonoField.h"

namespace bs
{
	ScriptObjectBase::ScriptObjectBase(MonoObject* instance)
		:mManagedInstance(instance)
	{	
		ScriptObjectManager::instance().registerScriptObject(this);
	}

	ScriptObjectBase::~ScriptObjectBase() 
	{
		if(mManagedInstance != nullptr)
			BS_EXCEPT(InvalidStateException, "Script object is being destroyed without its instance previously being released.");

		ScriptObjectManager::instance().unregisterScriptObject(this);
	}

	ScriptObjectBackup ScriptObjectBase::beginRefresh() 
	{ 
		return ScriptObjectBackup(); 
	}

	void ScriptObjectBase::endRefresh(const ScriptObjectBackup& data) 
	{ 

	}

	void ScriptObjectBase::_onManagedInstanceDeleted()
	{
		mManagedInstance = nullptr;
		bs_delete(this);
	}

	PersistentScriptObjectBase::PersistentScriptObjectBase(MonoObject* instance)
		:ScriptObjectBase(instance)
	{

	}

	PersistentScriptObjectBase::~PersistentScriptObjectBase()
	{

	}
}