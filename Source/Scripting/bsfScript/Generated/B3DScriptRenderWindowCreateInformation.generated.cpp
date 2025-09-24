//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptRenderWindowCreateInformation.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/RenderAPI/BsVideoModeInfo.h"
#include "BsScriptVideoMode.generated.h"

namespace b3d
{
#if !B3D_IS_ENGINE
	ScriptRenderWindowCreateInformation::ScriptRenderWindowCreateInformation()
	{ }

	MonoObject* ScriptRenderWindowCreateInformation::Box(const __RenderWindowCreateInformationInterop& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	__RenderWindowCreateInformationInterop ScriptRenderWindowCreateInformation::Unbox(MonoObject* value)
	{
		return *(__RenderWindowCreateInformationInterop*)MonoUtil::Unbox(value);
	}

	RenderWindowCreateInformation ScriptRenderWindowCreateInformation::FromInterop(const __RenderWindowCreateInformationInterop& value)
	{
		RenderWindowCreateInformation output;
		VideoMode tmpVideoMode;
		tmpVideoMode = ScriptVideoMode::FromInterop(value.VideoMode);
		output.VideoMode = tmpVideoMode;
		output.Fullscreen = value.Fullscreen;
		output.Vsync = value.Vsync;
		output.VsyncInterval = value.VsyncInterval;
		output.Hidden = value.Hidden;
		output.DepthBuffer = value.DepthBuffer;
		output.MultisampleCount = value.MultisampleCount;
		String tmpMultisampleHint;
		tmpMultisampleHint = MonoUtil::MonoToString(value.MultisampleHint);
		output.MultisampleHint = tmpMultisampleHint;
		output.Gamma = value.Gamma;
		output.Left = value.Left;
		output.Top = value.Top;
		String tmpTitle;
		tmpTitle = MonoUtil::MonoToString(value.Title);
		output.Title = tmpTitle;
		output.ShowTitleBar = value.ShowTitleBar;
		output.ShowBorder = value.ShowBorder;
		output.AllowResize = value.AllowResize;
		output.ToolWindow = value.ToolWindow;
		output.Modal = value.Modal;
		output.HideUntilSwap = value.HideUntilSwap;
		output.CreateRenderSurface = value.CreateRenderSurface;

		return output;
	}

	__RenderWindowCreateInformationInterop ScriptRenderWindowCreateInformation::ToInterop(const RenderWindowCreateInformation& value)
	{
		__RenderWindowCreateInformationInterop output;
		__VideoModeInterop tmpVideoMode;
		tmpVideoMode = ScriptVideoMode::ToInterop(value.VideoMode);
		output.VideoMode = tmpVideoMode;
		output.Fullscreen = value.Fullscreen;
		output.Vsync = value.Vsync;
		output.VsyncInterval = value.VsyncInterval;
		output.Hidden = value.Hidden;
		output.DepthBuffer = value.DepthBuffer;
		output.MultisampleCount = value.MultisampleCount;
		MonoString* tmpMultisampleHint;
		tmpMultisampleHint = MonoUtil::StringToMono(value.MultisampleHint);
		output.MultisampleHint = tmpMultisampleHint;
		output.Gamma = value.Gamma;
		output.Left = value.Left;
		output.Top = value.Top;
		MonoString* tmpTitle;
		tmpTitle = MonoUtil::StringToMono(value.Title);
		output.Title = tmpTitle;
		output.ShowTitleBar = value.ShowTitleBar;
		output.ShowBorder = value.ShowBorder;
		output.AllowResize = value.AllowResize;
		output.ToolWindow = value.ToolWindow;
		output.Modal = value.Modal;
		output.HideUntilSwap = value.HideUntilSwap;
		output.CreateRenderSurface = value.CreateRenderSurface;

		return output;
	}

#endif
}
