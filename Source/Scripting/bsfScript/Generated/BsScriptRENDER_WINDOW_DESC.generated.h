//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "../../../Foundation/bsfCore/RenderAPI/BsRenderWindow.h"
#include "../../../Foundation/bsfCore/RenderAPI/BsVideoModeInfo.h"
#include "BsScriptVideoMode.generated.h"

namespace bs
{
#if !BS_IS_BANSHEE3D
	struct __RENDER_WINDOW_DESCInterop
	{
		__VideoModeInterop VideoMode;
		bool Fullscreen;
		bool Vsync;
		uint32_t VsyncInterval;
		bool Hidden;
		bool DepthBuffer;
		uint32_t MultisampleCount;
		MonoString* MultisampleHint;
		bool Gamma;
		int32_t Left;
		int32_t Top;
		MonoString* Title;
		bool ShowTitleBar;
		bool ShowBorder;
		bool AllowResize;
		bool ToolWindow;
		bool Modal;
		bool HideUntilSwap;
	};

	class BS_SCR_BE_EXPORT ScriptRENDER_WINDOW_DESC : public ScriptObject<ScriptRENDER_WINDOW_DESC>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "RenderWindowDesc")

		static MonoObject* Box(const __RENDER_WINDOW_DESCInterop& value);
		static __RENDER_WINDOW_DESCInterop Unbox(MonoObject* value);
		static RENDER_WINDOW_DESC FromInterop(const __RENDER_WINDOW_DESCInterop& value);
		static __RENDER_WINDOW_DESCInterop ToInterop(const RENDER_WINDOW_DESC& value);

	private:
		ScriptRENDER_WINDOW_DESC(MonoObject* managedInstance);
	};
#endif
} // namespace bs
