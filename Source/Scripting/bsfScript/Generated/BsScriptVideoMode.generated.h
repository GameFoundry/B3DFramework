//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObjectWrapper.h"
#include "../../../Foundation/bsfCore/RenderAPI/BsVideoModeInfo.h"

namespace bs
{
#if !B3D_IS_ENGINE
	struct __VideoModeInterop
	{
		uint32_t Width;
		uint32_t Height;
		float RefreshRate;
		uint32_t OutputIdx;
		bool IsCustom;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptVideoMode : public TScriptTypeDefinition<ScriptVideoMode>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "VideoMode")

		static MonoObject* Box(const __VideoModeInterop& value);
		static __VideoModeInterop Unbox(MonoObject* value);
		static VideoMode FromInterop(const __VideoModeInterop& value);
		static __VideoModeInterop ToInterop(const VideoMode& value);

	private:
		ScriptVideoMode();

	};
#endif
}
