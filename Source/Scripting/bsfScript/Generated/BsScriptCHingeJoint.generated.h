//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptComponent.h"
#include "BsScriptCJoint.generated.h"
#include "Math/BsRadian.h"
#include "../../../Foundation/bsfCore/Physics/BsJoint.h"
#include "../../../Foundation/bsfCore/Physics/BsHingeJoint.h"
#include "../../../Foundation/bsfCore/Physics/BsHingeJoint.h"

namespace bs { class CHingeJoint; }
namespace bs { struct __LimitAngularRangeInterop; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptHingeJoint : public TScriptGameObjectWrapper<CHingeJoint, ScriptHingeJoint, ScriptJointWrapperBase>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "HingeJoint")

		ScriptHingeJoint(const GameObjectHandle<CHingeJoint>& nativeObject);
		~ScriptHingeJoint();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static void InternalGetAngle(ScriptHingeJoint* self, TRadian<float>* __output);
		static float InternalGetSpeed(ScriptHingeJoint* self);
		static void InternalGetLimit(ScriptHingeJoint* self, __LimitAngularRangeInterop* __output);
		static void InternalSetLimit(ScriptHingeJoint* self, __LimitAngularRangeInterop* limit);
		static void InternalGetDrive(ScriptHingeJoint* self, HingeJointDrive* __output);
		static void InternalSetDrive(ScriptHingeJoint* self, HingeJointDrive* drive);
		static void InternalSetFlag(ScriptHingeJoint* self, HingeJointFlag flag, bool enabled);
		static bool InternalHasFlag(ScriptHingeJoint* self, HingeJointFlag flag);
	};
}
