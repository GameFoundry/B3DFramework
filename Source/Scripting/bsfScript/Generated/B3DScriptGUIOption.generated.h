//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObjectWrapper.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUIOptions.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUIUnits.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUIOptions.h"

namespace b3d
{
	struct __GUIOptionInterop
	{
		TUnitValue<int32_t, LogicalPixel> mMinimum;
		TUnitValue<int32_t, LogicalPixel> mMaximum;
		GUIOptionType mType;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptGUIOption : public TScriptTypeDefinition<ScriptGUIOption>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "GUIOption")

		static MonoObject* Box(const __GUIOptionInterop& value);
		static __GUIOptionInterop Unbox(MonoObject* value);
		static GUIOption FromInterop(const __GUIOptionInterop& value);
		static __GUIOptionInterop ToInterop(const GUIOption& value);

	private:
		ScriptGUIOption();

	};
}
