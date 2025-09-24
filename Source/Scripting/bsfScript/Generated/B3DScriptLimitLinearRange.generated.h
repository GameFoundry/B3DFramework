//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptObjectWrapper.h"
#include "../../../Foundation/Core/Components/B3DJoint.h"
#include "../../../Foundation/Core/Components/B3DJoint.h"

namespace b3d
{
	struct __LimitLinearRangeInterop
	{
		float Lower;
		float Upper;
		float ContactDist;
		float Restitution;
		Spring Spring;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptLimitLinearRange : public TScriptTypeDefinition<ScriptLimitLinearRange>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "LimitLinearRange")

		static MonoObject* Box(const __LimitLinearRangeInterop& value);
		static __LimitLinearRangeInterop Unbox(MonoObject* value);
		static LimitLinearRange FromInterop(const __LimitLinearRangeInterop& value);
		static __LimitLinearRangeInterop ToInterop(const LimitLinearRange& value);

	private:
		ScriptLimitLinearRange();

	};
}
