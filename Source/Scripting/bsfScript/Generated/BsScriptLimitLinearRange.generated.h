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
		float Lower;
		float Upper;
		float ContactDist;
		float Restitution;
		Spring Spring;
	};

	class BS_SCR_BE_EXPORT ScriptLimitLinearRange : public ScriptObject<ScriptLimitLinearRange>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "LimitLinearRange")

		static MonoObject* Box(const __LimitLinearRangeInterop& value);
		static __LimitLinearRangeInterop Unbox(MonoObject* value);
		static LimitLinearRange FromInterop(const __LimitLinearRangeInterop& value);
		static __LimitLinearRangeInterop ToInterop(const LimitLinearRange& value);

	private:
		ScriptLimitLinearRange(MonoObject* managedInstance);
	};
} // namespace bs
