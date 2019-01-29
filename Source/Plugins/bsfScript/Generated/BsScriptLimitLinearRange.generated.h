//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "../../../Foundation/bsfCore/Physics/BsJoint.h"
#include "../../../Foundation/bsfCore/Physics/BsJoint.h"

namespace bs
{
	struct __LimitLinearRangeInterop
	{
		float lower;
		float upper;
		float contactDist;
		float restitution;
		Spring spring;
	};

	class BS_SCR_BE_EXPORT ScriptLimitLinearRange : public ScriptObject<ScriptLimitLinearRange>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "LimitLinearRange")

		static MonoObject* box(const __LimitLinearRangeInterop& value);
		static __LimitLinearRangeInterop unbox(MonoObject* value);
		static LimitLinearRange fromInterop(const __LimitLinearRangeInterop& value);
		static __LimitLinearRangeInterop toInterop(const LimitLinearRange& value);

	private:
		ScriptLimitLinearRange(MonoObject* managedInstance);

	};
}
