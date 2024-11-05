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
	ScriptApplication::ScriptApplication(const SPtr<ApplicationEx>& nativeObject)
		:TScriptNonReflectableWrapper(nativeObject)
	{
		RegisterEvents();
	}

	void ScriptApplication::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_StartUp", (void*)&ScriptApplication::InternalStartUp);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_StartUp0", (void*)&ScriptApplication::InternalStartUp0);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_RunMainLoop", (void*)&ScriptApplication::InternalRunMainLoop);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_ShutDown", (void*)&ScriptApplication::InternalShutDown);

	}

	MonoObject* ScriptApplication::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
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
