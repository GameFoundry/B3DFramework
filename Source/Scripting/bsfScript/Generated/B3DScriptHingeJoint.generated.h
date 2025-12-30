//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "Wrappers/B3DScriptComponent.h"
#include "B3DScriptJoint.generated.h"
#include "Math/B3DRadian.h"
#include "../../../Foundation/Core/Components/B3DJoint.h"
#include "../../../Foundation/Core/Components/B3DHingeJoint.h"
#include "../../../Foundation/Core/Components/B3DHingeJoint.h"

namespace b3d { class HingeJoint; }
namespace b3d { struct __LimitAngularRangeInterop; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptHingeJoint : public TScriptGameObjectWrapper<HingeJoint, ScriptHingeJoint, ScriptJointWrapperBase>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "HingeJoint")

		ScriptHingeJoint(const TGameObjectHandle<HingeJoint>& nativeObject);
		~ScriptHingeJoint();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static void InternalGetAngle(ScriptHingeJoint* self, TRadian<float>* __output);
		static float InternalGetSpeed(ScriptHingeJoint* self);
		static void InternalSetLimit(ScriptHingeJoint* self, __LimitAngularRangeInterop* limit);
		static void InternalGetLimit(ScriptHingeJoint* self, __LimitAngularRangeInterop* __output);
		static void InternalSetDrive(ScriptHingeJoint* self, HingeJointDrive* drive);
		static void InternalGetDrive(ScriptHingeJoint* self, HingeJointDrive* __output);
		static void InternalSetFlag(ScriptHingeJoint* self, HingeJointFlag flag, bool enabled);
		static bool InternalHasFlag(ScriptHingeJoint* self, HingeJointFlag flag);
	};
}
