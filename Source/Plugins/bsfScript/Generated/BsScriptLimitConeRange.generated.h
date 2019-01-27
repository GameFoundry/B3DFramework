//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "../../../../../Foundation/bsfCore/Physics/BsJoint.h"
#include "Math/BsRadian.h"
#include "../../../../../Foundation/bsfCore/Physics/BsJoint.h"

namespace bs
{
	struct __LimitConeRangeInterop
	{
		Radian yLimitAngle;
		Radian zLimitAngle;
		float contactDist;
		float restitution;
		Spring spring;
	};

	class BS_SCR_BE_EXPORT ScriptLimitConeRange : public ScriptObject<ScriptLimitConeRange>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "bs", "LimitConeRange")

		static MonoObject* box(const __LimitConeRangeInterop& value);
		static __LimitConeRangeInterop unbox(MonoObject* value);
		static LimitConeRange fromInterop(const __LimitConeRangeInterop& value);
		static __LimitConeRangeInterop toInterop(const LimitConeRange& value);

	private:
		ScriptLimitConeRange(MonoObject* managedInstance);

	};
}
