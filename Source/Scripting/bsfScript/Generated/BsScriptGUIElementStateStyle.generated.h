//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
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

	class BS_SCR_BE_EXPORT ScriptGUIElementStateStyle : public ScriptObject<ScriptGUIElementStateStyle>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "GUIElementStateStyle")

		static MonoObject* Box(const __GUIElementStateStyleInterop& value);
		static __GUIElementStateStyleInterop Unbox(MonoObject* value);
		static GUIElementStateStyle FromInterop(const __GUIElementStateStyleInterop& value);
		static __GUIElementStateStyleInterop ToInterop(const GUIElementStateStyle& value);

	private:
		ScriptGUIElementStateStyle(MonoObject* managedInstance);
	};
} // namespace bs
