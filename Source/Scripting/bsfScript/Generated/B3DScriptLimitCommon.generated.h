//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptObjectWrapper.h"
#include "../../../Foundation/Core/Components/B3DJoint.h"
#include "../../../Foundation/Core/Components/B3DJoint.h"

namespace b3d
{
	struct __LimitCommonInterop
	{
		float ContactDist;
		float Restitution;
		Spring Spring;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptLimitCommon : public TScriptTypeDefinition<ScriptLimitCommon>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "LimitCommon")

		static MonoObject* Box(const __LimitCommonInterop& value);
		static __LimitCommonInterop Unbox(MonoObject* value);
		static LimitCommon FromInterop(const __LimitCommonInterop& value);
		static __LimitCommonInterop ToInterop(const LimitCommon& value);

	private:
		ScriptLimitCommon();

	};
}
