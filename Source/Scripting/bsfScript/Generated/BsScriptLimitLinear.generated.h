//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "../../../Foundation/bsfCore/Physics/BsJoint.h"
#include "../../../Foundation/bsfCore/Physics/BsJoint.h"

namespace bs
{
	struct __LimitLinearInterop
	{
		float Extent;
		float ContactDist;
		float Restitution;
		Spring Spring;
	};

	class BS_SCR_BE_EXPORT ScriptLimitLinear : public ScriptObject<ScriptLimitLinear>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "LimitLinear")

		static MonoObject* Box(const __LimitLinearInterop& value);
		static __LimitLinearInterop Unbox(MonoObject* value);
		static LimitLinear FromInterop(const __LimitLinearInterop& value);
		static __LimitLinearInterop ToInterop(const LimitLinear& value);

	private:
		ScriptLimitLinear(MonoObject* managedInstance);
	};
} // namespace bs
