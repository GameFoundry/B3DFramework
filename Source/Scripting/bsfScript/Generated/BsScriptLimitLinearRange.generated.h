//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObjectWrapper.h"
#include "../../../Foundation/bsfCore/Physics/BsJoint.h"
#include "../../../Foundation/bsfCore/Physics/BsJoint.h"

namespace bs
{
	struct __LimitLinearRangeInterop
	{
		float Lower;
		float Upper;
		float ContactDist;
		float Restitution;
		Spring Spring;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptLimitLinearRange : public TScriptStructWrapper<ScriptLimitLinearRange>
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
