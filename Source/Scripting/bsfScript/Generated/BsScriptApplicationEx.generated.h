//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "../../../Foundation/bsfCore/RenderAPI/BsVideoModeInfo.h"

namespace bs { struct __START_UP_DESCInterop; }
namespace bs { class ApplicationEx; }
namespace bs { struct __VideoModeInterop; }
namespace bs
{
#if !BS_IS_BANSHEE3D
	class BS_SCR_BE_EXPORT ScriptApplicationEx : public ScriptObject<ScriptApplicationEx>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "Application")

		ScriptApplicationEx(MonoObject* managedInstance, const SPtr<ApplicationEx>& value);

		SPtr<ApplicationEx> getInternal() const { return mInternal; }
		static MonoObject* create(const SPtr<ApplicationEx>& value);

	private:
		SPtr<ApplicationEx> mInternal;

		static void Internal_startUp(__START_UP_DESCInterop* desc);
		static void Internal_startUp0(__VideoModeInterop* videoMode, MonoString* title, bool fullscreen);
		static void Internal_runMainLoop();
		static void Internal_shutDown();
	};
#endif
}
