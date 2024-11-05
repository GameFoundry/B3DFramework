//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "../Extensions/BsApplicationEx.h"
#include "BsScriptNonReflectableWrapper.h"
#include "../../../Foundation/bsfCore/RenderAPI/BsVideoModeInfo.h"
#include "../../../Foundation/bsfCore/BsCoreApplication.h"

namespace bs { struct __VideoModeInterop; }
namespace bs { struct __START_UP_DESCInterop; }
namespace bs
{
#if !B3D_IS_ENGINE
	class B3D_SCRIPT_INTEROP_EXPORT ScriptApplication : public TScriptNonReflectableWrapper<ApplicationEx, ScriptApplication>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "Application")

		ScriptApplication(const SPtr<ApplicationEx>& nativeObject);

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static void InternalStartUp(__START_UP_DESCInterop* desc);
		static void InternalStartUp0(__VideoModeInterop* videoMode, MonoString* title, bool fullscreen);
		static void InternalRunMainLoop();
		static void InternalShutDown();
	};
#endif
}
