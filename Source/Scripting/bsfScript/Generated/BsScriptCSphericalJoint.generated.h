//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptComponent.h"
#include "BsScriptCJoint.generated.h"
#include "../../../Foundation/bsfCore/Physics/BsJoint.h"
#include "../../../Foundation/bsfCore/Physics/BsSphericalJoint.h"

namespace bs { struct __LimitConeRangeInterop; }
namespace bs { class CSphericalJoint; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptSphericalJoint : public TScriptComponent<ScriptSphericalJoint, CSphericalJoint, ScriptJointBase>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "SphericalJoint")

		ScriptSphericalJoint(MonoObject* managedInstance, const GameObjectHandle<CSphericalJoint>& value);

	private:
		static void InternalGetLimit(ScriptSphericalJoint* thisPtr, __LimitConeRangeInterop* __output);
		static void InternalSetLimit(ScriptSphericalJoint* thisPtr, __LimitConeRangeInterop* limit);
		static void InternalSetFlag(ScriptSphericalJoint* thisPtr, SphericalJointFlag flag, bool enabled);
		static bool InternalHasFlag(ScriptSphericalJoint* thisPtr, SphericalJointFlag flag);
	};
}
