//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObjectWrapper.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUIElement.h"
#include "BsScriptObjectWrapper.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUIElement.h"

namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptTUnitValue_float__PhysicalPixel_ : public TScriptTypeDefinition<ScriptTUnitValue_float__PhysicalPixel_>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "TUnitValue<float,bs::PhysicalPixel>")

		static MonoObject* Box(const TUnitValue<float, PhysicalPixel>& value);
		static TUnitValue<float, PhysicalPixel> Unbox(MonoObject* value);

	private:
		ScriptTUnitValue_float__PhysicalPixel_();

	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptTUnitValue_int32_t__LogicalPixel_ : public TScriptTypeDefinition<ScriptTUnitValue_int32_t__LogicalPixel_>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "TUnitValue<int,bs::LogicalPixel>")

		static MonoObject* Box(const TUnitValue<int32_t, LogicalPixel>& value);
		static TUnitValue<int32_t, LogicalPixel> Unbox(MonoObject* value);

	private:
		ScriptTUnitValue_int32_t__LogicalPixel_();

	};
}
