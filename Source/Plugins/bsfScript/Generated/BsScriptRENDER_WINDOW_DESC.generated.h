//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "../../../Foundation/bsfCore/RenderAPI/BsRenderWindow.h"
#include "../../../Foundation/bsfCore/RenderAPI/BsVideoModeInfo.h"

namespace bs
{
#if !BS_IS_BANSHEE3D
	struct __RENDER_WINDOW_DESCInterop
	{
		VideoMode videoMode;
		bool fullscreen;
		bool vsync;
		uint32_t vsyncInterval;
		bool hidden;
		bool depthBuffer;
		uint32_t multisampleCount;
		MonoString* multisampleHint;
		bool gamma;
		int32_t left;
		int32_t top;
		MonoString* title;
		bool showTitleBar;
		bool showBorder;
		bool allowResize;
		bool toolWindow;
		bool modal;
		bool hideUntilSwap;
	};

	class BS_SCR_BE_EXPORT ScriptRENDER_WINDOW_DESC : public ScriptObject<ScriptRENDER_WINDOW_DESC>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "RenderWindowDesc")

		static MonoObject* box(const __RENDER_WINDOW_DESCInterop& value);
		static __RENDER_WINDOW_DESCInterop unbox(MonoObject* value);
		static RENDER_WINDOW_DESC fromInterop(const __RENDER_WINDOW_DESCInterop& value);
		static __RENDER_WINDOW_DESCInterop toInterop(const RENDER_WINDOW_DESC& value);

	private:
		ScriptRENDER_WINDOW_DESC(MonoObject* managedInstance);

	};
#endif
}
