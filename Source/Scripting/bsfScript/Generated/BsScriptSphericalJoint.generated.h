//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptComponent.h"
#include "BsScriptJoint.generated.h"
#include "../../../Foundation/bsfCore/Components/BsJoint.h"
#include "../../../Foundation/bsfCore/Components/BsSphericalJoint.h"

namespace b3d { class SphericalJoint; }
namespace b3d { struct __LimitConeRangeInterop; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptSphericalJoint : public TScriptGameObjectWrapper<SphericalJoint, ScriptSphericalJoint, ScriptJointWrapperBase>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "SphericalJoint")

		ScriptSphericalJoint(const GameObjectHandle<SphericalJoint>& nativeObject);
		~ScriptSphericalJoint();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static void InternalSetLimit(ScriptSphericalJoint* self, __LimitConeRangeInterop* limit);
		static void InternalGetLimit(ScriptSphericalJoint* self, __LimitConeRangeInterop* __output);
		static void InternalSetFlag(ScriptSphericalJoint* self, SphericalJointFlag flag, bool enabled);
		static bool InternalHasFlag(ScriptSphericalJoint* self, SphericalJointFlag flag);
	};
}
