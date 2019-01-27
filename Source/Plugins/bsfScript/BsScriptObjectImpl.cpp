//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptObjectImpl.h"
#include "BsMonoManager.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "BsScriptObjectManager.h"

namespace bs
{
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