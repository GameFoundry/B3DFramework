//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
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
		metaData.scriptClass->addInternalCall("Internal_ManagedInstanceDeleted", &ScriptObjectImpl::internal_managedInstanceDeleted);
	}

	void ScriptObjectImpl::internal_managedInstanceDeleted(ScriptObjectBase* instance)
	{
		// This method gets called on the finalizer thread, but so that we don't need to deal
		// with multi-threading issues we just delay it and execute it on the sim thread.
		ScriptObjectManager::instance().notifyObjectFinalized(instance);
	}
}