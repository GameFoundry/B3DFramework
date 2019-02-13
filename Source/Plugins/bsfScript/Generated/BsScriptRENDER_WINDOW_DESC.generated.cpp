//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptRENDER_WINDOW_DESC.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/RenderAPI/BsVideoModeInfo.h"
#include "BsScriptVideoMode.generated.h"

namespace bs
{
#if !BS_IS_BANSHEE3D
	ScriptRENDER_WINDOW_DESC::ScriptRENDER_WINDOW_DESC(MonoObject* managedInstance)
		:ScriptObject(managedInstance)
	{ }

	void ScriptRENDER_WINDOW_DESC::initRuntimeData()
	{ }

	MonoObject*ScriptRENDER_WINDOW_DESC::box(const __RENDER_WINDOW_DESCInterop& value)
	{
		return MonoUtil::box(metaData.scriptClass->_getInternalClass(), (void*)&value);
	}

	__RENDER_WINDOW_DESCInterop ScriptRENDER_WINDOW_DESC::unbox(MonoObject* value)
	{
		return *(__RENDER_WINDOW_DESCInterop*)MonoUtil::unbox(value);
	}

	RENDER_WINDOW_DESC ScriptRENDER_WINDOW_DESC::fromInterop(const __RENDER_WINDOW_DESCInterop& value)
	{
		RENDER_WINDOW_DESC output;
		output.videoMode = value.videoMode;
		output.fullscreen = value.fullscreen;
		output.vsync = value.vsync;
		output.vsyncInterval = value.vsyncInterval;
		output.hidden = value.hidden;
		output.depthBuffer = value.depthBuffer;
		output.multisampleCount = value.multisampleCount;
		String tmpmultisampleHint;
		tmpmultisampleHint = MonoUtil::monoToString(value.multisampleHint);
		output.multisampleHint = tmpmultisampleHint;
		output.gamma = value.gamma;
		output.left = value.left;
		output.top = value.top;
		String tmptitle;
		tmptitle = MonoUtil::monoToString(value.title);
		output.title = tmptitle;
		output.showTitleBar = value.showTitleBar;
		output.showBorder = value.showBorder;
		output.allowResize = value.allowResize;
		output.toolWindow = value.toolWindow;
		output.modal = value.modal;
		output.hideUntilSwap = value.hideUntilSwap;

		return output;
	}

	__RENDER_WINDOW_DESCInterop ScriptRENDER_WINDOW_DESC::toInterop(const RENDER_WINDOW_DESC& value)
	{
		__RENDER_WINDOW_DESCInterop output;
		output.videoMode = value.videoMode;
		output.fullscreen = value.fullscreen;
		output.vsync = value.vsync;
		output.vsyncInterval = value.vsyncInterval;
		output.hidden = value.hidden;
		output.depthBuffer = value.depthBuffer;
		output.multisampleCount = value.multisampleCount;
		MonoString* tmpmultisampleHint;
		tmpmultisampleHint = MonoUtil::stringToMono(value.multisampleHint);
		output.multisampleHint = tmpmultisampleHint;
		output.gamma = value.gamma;
		output.left = value.left;
		output.top = value.top;
		MonoString* tmptitle;
		tmptitle = MonoUtil::stringToMono(value.title);
		output.title = tmptitle;
		output.showTitleBar = value.showTitleBar;
		output.showBorder = value.showBorder;
		output.allowResize = value.allowResize;
		output.toolWindow = value.toolWindow;
		output.modal = value.modal;
		output.hideUntilSwap = value.hideUntilSwap;

		return output;
	}

#endif
}
