//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObjectWrapper.h"
#include "../../../Foundation/bsfCore/Physics/BsJoint.h"
#include "../../../Foundation/bsfCore/Physics/BsJoint.h"

namespace bs
{
	struct __LimitCommonInterop
	{
		float ContactDist;
		float Restitution;
		Spring Spring;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptLimitCommon : public TScriptStructWrapper<ScriptLimitCommon>
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
