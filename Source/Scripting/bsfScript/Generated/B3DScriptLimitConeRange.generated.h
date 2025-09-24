//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptObjectWrapper.h"
#include "../../../Foundation/Core/Components/B3DJoint.h"
#include "Math/B3DRadian.h"
#include "../../../Foundation/Core/Components/B3DJoint.h"

namespace b3d
{
	struct __LimitConeRangeInterop
	{
		TRadian<float> YLimitAngle;
		TRadian<float> ZLimitAngle;
		float ContactDist;
		float Restitution;
		Spring Spring;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptLimitConeRange : public TScriptTypeDefinition<ScriptLimitConeRange>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "LimitConeRange")

		static MonoObject* Box(const __LimitConeRangeInterop& value);
		static __LimitConeRangeInterop Unbox(MonoObject* value);
		static LimitConeRange FromInterop(const __LimitConeRangeInterop& value);
		static __LimitConeRangeInterop ToInterop(const LimitConeRange& value);

	private:
		ScriptLimitConeRange();

	};
}
