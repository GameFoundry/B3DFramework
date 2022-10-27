//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "../../../Foundation/bsfCore/Physics/BsJoint.h"
#include "Math/BsRadian.h"
#include "../../../Foundation/bsfCore/Physics/BsJoint.h"

namespace bs
{
	struct __LimitConeRangeInterop
	{
		Radian YLimitAngle;
		Radian ZLimitAngle;
		float ContactDist;
		float Restitution;
		Spring Spring;
	};

	class BS_SCR_BE_EXPORT ScriptLimitConeRange : public ScriptObject<ScriptLimitConeRange>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "LimitConeRange")

		static MonoObject* Box(const __LimitConeRangeInterop& value);
		static __LimitConeRangeInterop Unbox(MonoObject* value);
		static LimitConeRange FromInterop(const __LimitConeRangeInterop& value);
		static __LimitConeRangeInterop ToInterop(const LimitConeRange& value);

	private:
		ScriptLimitConeRange(MonoObject* managedInstance);
	};
} // namespace bs
