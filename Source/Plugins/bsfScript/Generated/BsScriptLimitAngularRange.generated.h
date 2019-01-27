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
	struct __LimitAngularRangeInterop
	{
		Radian lower;
		Radian upper;
		float contactDist;
		float restitution;
		Spring spring;
	};

	class BS_SCR_BE_EXPORT ScriptLimitAngularRange : public ScriptObject<ScriptLimitAngularRange>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "bs", "LimitAngularRange")

		static MonoObject* box(const __LimitAngularRangeInterop& value);
		static __LimitAngularRangeInterop unbox(MonoObject* value);
		static LimitAngularRange fromInterop(const __LimitAngularRangeInterop& value);
		static __LimitAngularRangeInterop toInterop(const LimitAngularRange& value);

	private:
		ScriptLimitAngularRange(MonoObject* managedInstance);

	};
}
