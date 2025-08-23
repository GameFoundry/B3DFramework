//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObjectWrapper.h"
#include "../../../Foundation/bsfCore/Components/BsJoint.h"
#include "../../../Foundation/bsfCore/Components/BsJoint.h"

namespace b3d
{
	struct __LimitLinearInterop
	{
		float Extent;
		float ContactDist;
		float Restitution;
		Spring Spring;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptLimitLinear : public TScriptTypeDefinition<ScriptLimitLinear>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "LimitLinear")

		static MonoObject* Box(const __LimitLinearInterop& value);
		static __LimitLinearInterop Unbox(MonoObject* value);
		static LimitLinear FromInterop(const __LimitLinearInterop& value);
		static __LimitLinearInterop ToInterop(const LimitLinear& value);

	private:
		ScriptLimitLinear();

	};
}
