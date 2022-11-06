//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptRENDER_WINDOW_DESC.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/RenderAPI/BsVideoModeInfo.h"
#include "BsScriptVideoMode.generated.h"

namespace bs
{
#if !B3D_IS_ENGINE
	ScriptRENDER_WINDOW_DESC::ScriptRENDER_WINDOW_DESC(MonoObject* managedInstance)
		:ScriptObject(managedInstance)
	{ }

	void ScriptRENDER_WINDOW_DESC::InitRuntimeData()
	{ }

	MonoObject*ScriptRENDER_WINDOW_DESC::Box(const __RENDER_WINDOW_DESCInterop& value)
	{
		return MonoUtil::Box(metaData.ScriptClass->GetInternalClassInternal(), (void*)&value);
	}

	__RENDER_WINDOW_DESCInterop ScriptRENDER_WINDOW_DESC::Unbox(MonoObject* value)
	{
		return *(__RENDER_WINDOW_DESCInterop*)MonoUtil::Unbox(value);
	}

	RENDER_WINDOW_DESC ScriptRENDER_WINDOW_DESC::FromInterop(const __RENDER_WINDOW_DESCInterop& value)
	{
		RENDER_WINDOW_DESC output;
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

		return output;
	}

	__RENDER_WINDOW_DESCInterop ScriptRENDER_WINDOW_DESC::ToInterop(const RENDER_WINDOW_DESC& value)
	{
		__RENDER_WINDOW_DESCInterop output;
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

		return output;
	}

#endif
}
