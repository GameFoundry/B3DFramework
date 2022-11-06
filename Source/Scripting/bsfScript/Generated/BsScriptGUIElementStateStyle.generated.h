//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUIElementStyle.h"
#include "Image/BsColor.h"

namespace bs
{
	struct __GUIElementStateStyleInterop
	{
		MonoObject* Texture;
		Color TextColor;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptGUIElementStateStyle : public ScriptObject<ScriptGUIElementStateStyle>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "GUIElementStateStyle")

		static MonoObject* Box(const __GUIElementStateStyleInterop& value);
		static __GUIElementStateStyleInterop Unbox(MonoObject* value);
		static GUIElementStateStyle FromInterop(const __GUIElementStateStyleInterop& value);
		static __GUIElementStateStyleInterop ToInterop(const GUIElementStateStyle& value);

	private:
		ScriptGUIElementStateStyle(MonoObject* managedInstance);

	};
}
