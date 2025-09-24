//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObjectWrapper.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUIPanel.h"

namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptGUIPanelContent : public TScriptTypeDefinition<ScriptGUIPanelContent>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "GUIPanelContent")

		static MonoObject* Box(const GUIPanelContent& value);
		static GUIPanelContent Unbox(MonoObject* value);

	private:
		ScriptGUIPanelContent();

	};
}
