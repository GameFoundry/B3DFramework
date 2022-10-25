//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptComponent.h"
#include "BsScriptCJoint.generated.h"
#include "../../../Foundation/bsfCore/Physics/BsSliderJoint.h"
#include "../../../Foundation/bsfCore/Physics/BsJoint.h"

namespace bs
{
	struct __LimitLinearRangeInterop;
}

namespace bs
{
	class CSliderJoint;
}

namespace bs
{
	class BS_SCR_BE_EXPORT ScriptCSliderJoint : public TScriptComponent<ScriptCSliderJoint, CSliderJoint, ScriptCJointBase>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "SliderJoint")

		ScriptCSliderJoint(MonoObject* managedInstance, const GameObjectHandle<CSliderJoint>& value);

	private:
		static float InternalGetPosition(ScriptCSliderJoint* thisPtr);
		static float InternalGetSpeed(ScriptCSliderJoint* thisPtr);
		static void InternalGetLimit(ScriptCSliderJoint* thisPtr, __LimitLinearRangeInterop* __output);
		static void InternalSetLimit(ScriptCSliderJoint* thisPtr, __LimitLinearRangeInterop* limit);
		static void InternalSetFlag(ScriptCSliderJoint* thisPtr, SliderJointFlag flag, bool enabled);
		static bool InternalHasFlag(ScriptCSliderJoint* thisPtr, SliderJointFlag flag);
	};
} // namespace bs
