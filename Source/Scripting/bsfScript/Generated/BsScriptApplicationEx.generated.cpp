//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptApplicationEx.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../Extensions/BsApplicationEx.h"
#include "BsScriptSTART_UP_DESC.generated.h"

namespace bs
{
#if !BS_IS_BANSHEE3D
	ScriptApplicationEx::ScriptApplicationEx(MonoObject* managedInstance, const SPtr<ApplicationEx>& value)
		:ScriptObject(managedInstance), mInternal(value)
	{
	}

	void ScriptApplicationEx::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_startUp", (void*)&ScriptApplicationEx::Internal_startUp);
		metaData.scriptClass->addInternalCall("Internal_runMainLoop", (void*)&ScriptApplicationEx::Internal_runMainLoop);
		metaData.scriptClass->addInternalCall("Internal_shutDown", (void*)&ScriptApplicationEx::Internal_shutDown);

	}

	MonoObject* ScriptApplicationEx::create(const SPtr<ApplicationEx>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.scriptClass->createInstance("bool", ctorParams);
		new (bs_alloc<ScriptApplicationEx>()) ScriptApplicationEx(managedInstance, value);
		return managedInstance;
	}
	void ScriptApplicationEx::Internal_startUp(__START_UP_DESCInterop* desc)
	{
		START_UP_DESC tmpdesc;
		tmpdesc = ScriptSTART_UP_DESC::fromInterop(*desc);
		ApplicationEx::startUp(tmpdesc);
	}

	void ScriptApplicationEx::Internal_runMainLoop()
	{
		ApplicationEx::runMainLoop();
	}

	void ScriptApplicationEx::Internal_shutDown()
	{
		ApplicationEx::shutDown();
	}
#endif
}
