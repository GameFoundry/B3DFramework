//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptApplicationEx.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../Extensions/BsApplicationEx.h"
#include "BsScriptVideoMode.generated.h"
#include "BsScriptSTART_UP_DESC.generated.h"

namespace bs
{
#if !BS_IS_BANSHEE3D
	ScriptApplicationEx::ScriptApplicationEx(MonoObject* managedInstance, const SPtr<ApplicationEx>& value)
		:ScriptObject(managedInstance), mInternal(value)
	{
	}

	void ScriptApplicationEx::InitRuntimeData()
	{
		metaData.scriptClass->AddInternalCall("Internal_startUp", (void*)&ScriptApplicationEx::Internal_startUp);
		metaData.scriptClass->AddInternalCall("Internal_startUp0", (void*)&ScriptApplicationEx::Internal_startUp0);
		metaData.scriptClass->AddInternalCall("Internal_runMainLoop", (void*)&ScriptApplicationEx::Internal_runMainLoop);
		metaData.scriptClass->AddInternalCall("Internal_shutDown", (void*)&ScriptApplicationEx::Internal_shutDown);

	}

	MonoObject* ScriptApplicationEx::create(const SPtr<ApplicationEx>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.scriptClass->CreateInstance("bool", ctorParams);
		new (bs_alloc<ScriptApplicationEx>()) ScriptApplicationEx(managedInstance, value);
		return managedInstance;
	}
	void ScriptApplicationEx::Internal_startUp(__START_UP_DESCInterop* desc)
	{
		START_UP_DESC tmpdesc;
		tmpdesc = ScriptSTART_UP_DESC::fromInterop(*desc);
		ApplicationEx::startUp(tmpdesc);
	}

	void ScriptApplicationEx::Internal_startUp0(__VideoModeInterop* videoMode, MonoString* title, bool fullscreen)
	{
		VideoMode tmpvideoMode;
		tmpvideoMode = ScriptVideoMode::fromInterop(*videoMode);
		String tmptitle;
		tmptitle = MonoUtil::monoToString(title);
		ApplicationEx::startUp(tmpvideoMode, tmptitle, fullscreen);
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
