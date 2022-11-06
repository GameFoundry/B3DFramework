//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
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
#if !B3D_IS_ENGINE
	ScriptApplicationEx::ScriptApplicationEx(MonoObject* managedInstance, const SPtr<ApplicationEx>& value)
		:ScriptObject(managedInstance), mInternal(value)
	{
	}

	void ScriptApplicationEx::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_StartUp", (void*)&ScriptApplicationEx::InternalStartUp);
		metaData.ScriptClass->AddInternalCall("Internal_StartUp0", (void*)&ScriptApplicationEx::InternalStartUp0);
		metaData.ScriptClass->AddInternalCall("Internal_RunMainLoop", (void*)&ScriptApplicationEx::InternalRunMainLoop);
		metaData.ScriptClass->AddInternalCall("Internal_ShutDown", (void*)&ScriptApplicationEx::InternalShutDown);

	}

	MonoObject* ScriptApplicationEx::Create(const SPtr<ApplicationEx>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.ScriptClass->CreateInstance("bool", ctorParams);
		new (B3DAllocate<ScriptApplicationEx>()) ScriptApplicationEx(managedInstance, value);
		return managedInstance;
	}
	void ScriptApplicationEx::InternalStartUp(__START_UP_DESCInterop* desc)
	{
		START_UP_DESC tmpdesc;
		tmpdesc = ScriptSTART_UP_DESC::FromInterop(*desc);
		ApplicationEx::StartUp(tmpdesc);
	}

	void ScriptApplicationEx::InternalStartUp0(__VideoModeInterop* videoMode, MonoString* title, bool fullscreen)
	{
		VideoMode tmpvideoMode;
		tmpvideoMode = ScriptVideoMode::FromInterop(*videoMode);
		String tmptitle;
		tmptitle = MonoUtil::MonoToString(title);
		ApplicationEx::StartUp(tmpvideoMode, tmptitle, fullscreen);
	}

	void ScriptApplicationEx::InternalRunMainLoop()
	{
		ApplicationEx::RunMainLoop();
	}

	void ScriptApplicationEx::InternalShutDown()
	{
		ApplicationEx::ShutDown();
	}
#endif
}
