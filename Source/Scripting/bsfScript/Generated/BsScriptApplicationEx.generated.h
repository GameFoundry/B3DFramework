//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "../../../Foundation/bsfCore/RenderAPI/BsVideoModeInfo.h"
#include "../../../Foundation/bsfCore/BsCoreApplication.h"

namespace bs { struct __START_UP_DESCInterop; }
namespace bs { class ApplicationEx; }
namespace bs { struct __VideoModeInterop; }
namespace bs
{
#if !B3D_IS_ENGINE
	class B3D_SCRIPT_INTEROP_EXPORT ScriptApplicationEx : public ScriptObject<ScriptApplicationEx>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "Application")

		ScriptApplicationEx(MonoObject* managedInstance, const SPtr<ApplicationEx>& value);

		SPtr<ApplicationEx> GetInternal() const { return mInternal; }
		static MonoObject* Create(const SPtr<ApplicationEx>& value);

	private:
		SPtr<ApplicationEx> mInternal;

		static void InternalStartUp(__START_UP_DESCInterop* desc);
		static void InternalStartUp0(__VideoModeInterop* videoMode, MonoString* title, bool fullscreen);
		static void InternalRunMainLoop();
		static void InternalShutDown();
	};
#endif
}
