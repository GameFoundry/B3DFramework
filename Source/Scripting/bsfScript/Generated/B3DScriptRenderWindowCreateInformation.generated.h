//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObjectWrapper.h"
#include "../../../Foundation/bsfCore/RenderAPI/BsRenderWindow.h"
#include "../../../Foundation/bsfCore/RenderAPI/BsVideoModeInfo.h"
#include "BsScriptVideoMode.generated.h"

namespace b3d
{
#if !B3D_IS_ENGINE
	struct __RenderWindowCreateInformationInterop
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
		bool CreateRenderSurface;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptRenderWindowCreateInformation : public TScriptTypeDefinition<ScriptRenderWindowCreateInformation>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "RenderWindowCreateInformation")

		static MonoObject* Box(const __RenderWindowCreateInformationInterop& value);
		static __RenderWindowCreateInformationInterop Unbox(MonoObject* value);
		static RenderWindowCreateInformation FromInterop(const __RenderWindowCreateInformationInterop& value);
		static __RenderWindowCreateInformationInterop ToInterop(const RenderWindowCreateInformation& value);

	private:
		ScriptRenderWindowCreateInformation();

	};
#endif
}
