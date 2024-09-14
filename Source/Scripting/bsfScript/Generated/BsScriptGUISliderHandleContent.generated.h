//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObjectWrapper.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUISliderHandle.h"

namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptGUISliderHandleContent : public TScriptStructWrapper<ScriptGUISliderHandleContent>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "GUISliderHandleContent")

		static MonoObject* Box(const GUISliderHandleContent& value);
		static GUISliderHandleContent Unbox(MonoObject* value);

	private:
		ScriptGUISliderHandleContent();

	};
}
