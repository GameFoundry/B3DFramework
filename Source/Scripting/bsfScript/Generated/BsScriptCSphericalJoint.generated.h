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
	class B3D_SCRIPT_INTEROP_EXPORT ScriptSphericalJoint : public TScriptGameObjectWrapper<CSphericalJoint, ScriptSphericalJoint>
	{
	public:
		B3D_SCRIPT_OBJECT_WRAPPER(kEngineAssembly, kEngineNs, "SphericalJoint")

		ScriptSphericalJoint(const GameObjectHandle<CSphericalJoint>& nativeObject, MonoObject* scriptObject);

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static void InternalGetLimit(ScriptSphericalJoint* self, __LimitConeRangeInterop* __output);
		static void InternalSetLimit(ScriptSphericalJoint* self, __LimitConeRangeInterop* limit);
		static void InternalSetFlag(ScriptSphericalJoint* self, SphericalJointFlag flag, bool enabled);
		static bool InternalHasFlag(ScriptSphericalJoint* self, SphericalJointFlag flag);
	};
}
