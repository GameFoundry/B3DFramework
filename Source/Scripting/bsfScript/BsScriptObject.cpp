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

	ScriptObjectImpl::ScriptObjectImpl(MonoObject* instance)
		:ScriptObject(instance)
	{ }

	void ScriptObjectImpl::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_ManagedInstanceDeleted", (void*)&ScriptObjectImpl::internal_managedInstanceDeleted);
	}

	void ScriptObjectImpl::internal_managedInstanceDeleted(ScriptObjectBase* instance)
	{
		// This method gets called on the finalizer thread, but so that we don't need to deal
		// with multi-threading issues we just delay it and execute it on the sim thread.
		ScriptObjectManager::instance().notifyObjectFinalized(instance);
	}
}