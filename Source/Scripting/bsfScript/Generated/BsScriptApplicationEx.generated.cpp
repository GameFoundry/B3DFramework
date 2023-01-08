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
	ScriptApplication::ScriptApplication(MonoObject* managedInstance, const SPtr<ApplicationEx>& value)
		:ScriptObject(managedInstance), mInternal(value)
	{
	}

	void ScriptApplication::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_StartUp", (void*)&ScriptApplication::InternalStartUp);
		metaData.ScriptClass->AddInternalCall("Internal_StartUp0", (void*)&ScriptApplication::InternalStartUp0);
		metaData.ScriptClass->AddInternalCall("Internal_RunMainLoop", (void*)&ScriptApplication::InternalRunMainLoop);
		metaData.ScriptClass->AddInternalCall("Internal_ShutDown", (void*)&ScriptApplication::InternalShutDown);

	}

	MonoObject* ScriptApplication::Create(const SPtr<ApplicationEx>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.ScriptClass->CreateInstance("bool", ctorParams);
		new (B3DAllocate<ScriptApplication>()) ScriptApplication(managedInstance, value);
		return managedInstance;
	}
	void ScriptApplication::InternalStartUp(__START_UP_DESCInterop* desc)
	{
		START_UP_DESC tmpdesc;
		tmpdesc = ScriptStartUpDesc::FromInterop(*desc);
		ApplicationEx::StartUp(tmpdesc);
	}

	void ScriptApplication::InternalStartUp0(__VideoModeInterop* videoMode, MonoString* title, bool fullscreen)
	{
		VideoMode tmpvideoMode;
		tmpvideoMode = ScriptVideoMode::FromInterop(*videoMode);
		String tmptitle;
		tmptitle = MonoUtil::MonoToString(title);
		ApplicationEx::StartUp(tmpvideoMode, tmptitle, fullscreen);
	}

	void ScriptApplication::InternalRunMainLoop()
	{
		ApplicationEx::RunMainLoop();
	}

	void ScriptApplication::InternalShutDown()
	{
		ApplicationEx::ShutDown();
	}
#endif
}
