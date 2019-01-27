//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptObject.h"
#include "BsScriptObjectManager.h"
#include "BsMonoManager.h"
#include "Error/BsCrashHandler.h"
#include "BsMonoField.h"

namespace bs
{
	ScriptObjectBase::ScriptObjectBase(MonoObject* instance)
	{	
		ScriptObjectManager::instance().registerScriptObject(this);
	}

	ScriptObjectBase::~ScriptObjectBase() 
	{
		ScriptObjectManager::instance().unregisterScriptObject(this);
	}

	ScriptObjectBackup ScriptObjectBase::beginRefresh() 
	{ 
		return ScriptObjectBackup(); 
	}

	void ScriptObjectBase::endRefresh(const ScriptObjectBackup& data) 
	{ 

	}

	void ScriptObjectBase::_onManagedInstanceDeleted(bool assemblyRefresh)
	{
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