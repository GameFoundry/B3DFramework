//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "Wrappers/B3DScriptComponent.h"
#include "B3DScriptJoint.generated.h"
#include "../../../Foundation/Core/Components/B3DD6Joint.h"
#include "../../../Foundation/Core/Components/B3DD6Joint.h"
#include "../../../Foundation/Core/Components/B3DD6Joint.h"
#include "../../../Foundation/Core/Components/B3DD6Joint.h"
#include "Math/B3DRadian.h"
#include "../../../Foundation/Core/Components/B3DJoint.h"
#include "../../../Foundation/Core/Components/B3DJoint.h"
#include "../../../Foundation/Core/Components/B3DJoint.h"
#include "../../../Foundation/Utility/Math/B3DVector3.h"
#include "../../../Foundation/Utility/Math/B3DQuaternion.h"

namespace b3d { class D6Joint; }
namespace b3d { struct __LimitLinearInterop; }
namespace b3d { struct __LimitAngularRangeInterop; }
namespace b3d { struct __LimitConeRangeInterop; }
namespace b3d { struct __D6JointDriveInterop; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptD6Joint : public TScriptGameObjectWrapper<D6Joint, ScriptD6Joint, ScriptJointWrapperBase>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "D6Joint")

		ScriptD6Joint(const TGameObjectHandle<D6Joint>& nativeObject);
		~ScriptD6Joint();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static void InternalSetMotion(ScriptD6Joint* self, D6JointAxis axis, D6JointMotion motion);
		static D6JointMotion InternalGetMotion(ScriptD6Joint* self, D6JointAxis axis);
		static void InternalGetTwist(ScriptD6Joint* self, TRadian<float>* __output);
		static void InternalGetSwingY(ScriptD6Joint* self, TRadian<float>* __output);
		static void InternalGetSwingZ(ScriptD6Joint* self, TRadian<float>* __output);
		static void InternalSetLimitLinear(ScriptD6Joint* self, __LimitLinearInterop* limit);
		static void InternalGetLimitLinear(ScriptD6Joint* self, __LimitLinearInterop* __output);
		static void InternalSetLimitTwist(ScriptD6Joint* self, __LimitAngularRangeInterop* limit);
		static void InternalGetLimitTwist(ScriptD6Joint* self, __LimitAngularRangeInterop* __output);
		static void InternalSetLimitSwing(ScriptD6Joint* self, __LimitConeRangeInterop* limit);
		static void InternalGetLimitSwing(ScriptD6Joint* self, __LimitConeRangeInterop* __output);
		static void InternalSetDrive(ScriptD6Joint* self, D6JointDriveType type, __D6JointDriveInterop* drive);
		static void InternalGetDrive(ScriptD6Joint* self, D6JointDriveType type, __D6JointDriveInterop* __output);
		static void InternalGetDrivePosition(ScriptD6Joint* self, TVector3<float>* __output);
		static void InternalGetDriveRotation(ScriptD6Joint* self, TQuaternion<float>* __output);
		static void InternalSetDriveTransform(ScriptD6Joint* self, TVector3<float>* position, TQuaternion<float>* rotation);
		static void InternalGetDriveLinearVelocity(ScriptD6Joint* self, TVector3<float>* __output);
		static void InternalGetDriveAngularVelocity(ScriptD6Joint* self, TVector3<float>* __output);
		static void InternalSetDriveVelocity(ScriptD6Joint* self, TVector3<float>* linear, TVector3<float>* angular);
	};
}
