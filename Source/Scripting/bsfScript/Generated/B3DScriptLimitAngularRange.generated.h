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
	struct __LimitAngularRangeInterop
	{
		TRadian<float> Lower;
		TRadian<float> Upper;
		float ContactDist;
		float Restitution;
		Spring Spring;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptLimitAngularRange : public TScriptTypeDefinition<ScriptLimitAngularRange>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "LimitAngularRange")

		static MonoObject* Box(const __LimitAngularRangeInterop& value);
		static __LimitAngularRangeInterop Unbox(MonoObject* value);
		static LimitAngularRange FromInterop(const __LimitAngularRangeInterop& value);
		static __LimitAngularRangeInterop ToInterop(const LimitAngularRange& value);

	private:
		ScriptLimitAngularRange();

	};
}
