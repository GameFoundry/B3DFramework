//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "../../../Foundation/bsfCore/Physics/BsJoint.h"
#include "Math/BsRadian.h"
#include "../../../Foundation/bsfCore/Physics/BsJoint.h"

namespace bs
{
	struct __LimitAngularRangeInterop
	{
		TRadian<float> Lower;
		TRadian<float> Upper;
		float ContactDist;
		float Restitution;
		Spring Spring;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptLimitAngularRange : public ScriptObject<ScriptLimitAngularRange>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "LimitAngularRange")

		static MonoObject* Box(const __LimitAngularRangeInterop& value);
		static __LimitAngularRangeInterop Unbox(MonoObject* value);
		static LimitAngularRange FromInterop(const __LimitAngularRangeInterop& value);
		static __LimitAngularRangeInterop ToInterop(const LimitAngularRange& value);

	private:
		ScriptLimitAngularRange(MonoObject* managedInstance);

	};
}
