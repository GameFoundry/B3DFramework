//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObjectWrapper.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUIOptions.h"

namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptGUIOption : public TScriptStructWrapper<ScriptGUIOption>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "GUIOption")

		static MonoObject* Box(const GUIOption& value);
		static GUIOption Unbox(MonoObject* value);

	private:
		ScriptGUIOption();

	};
}
