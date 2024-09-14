//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObjectWrapper.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUIInputBox.h"

namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptGUIInputBoxContent : public TScriptStructWrapper<ScriptGUIInputBoxContent>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "GUIInputBoxContent")

		static MonoObject* Box(const GUIInputBoxContent& value);
		static GUIInputBoxContent Unbox(MonoObject* value);

	private:
		ScriptGUIInputBoxContent();

	};
}
