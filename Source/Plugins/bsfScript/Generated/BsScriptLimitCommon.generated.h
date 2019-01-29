//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "../../../Foundation/bsfCore/Physics/BsJoint.h"
#include "../../../Foundation/bsfCore/Physics/BsJoint.h"

namespace bs
{
	struct __LimitCommonInterop
	{
		float contactDist;
		float restitution;
		Spring spring;
	};

	class BS_SCR_BE_EXPORT ScriptLimitCommon : public ScriptObject<ScriptLimitCommon>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "LimitCommon")

		static MonoObject* box(const __LimitCommonInterop& value);
		static __LimitCommonInterop unbox(MonoObject* value);
		static LimitCommon fromInterop(const __LimitCommonInterop& value);
		static __LimitCommonInterop toInterop(const LimitCommon& value);

	private:
		ScriptLimitCommon(MonoObject* managedInstance);

	};
}
