//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptComponent.h"
#include "BsScriptCJoint.generated.h"
#include "../../../Foundation/bsfCore/Physics/BsSliderJoint.h"
#include "../../../Foundation/bsfCore/Physics/BsJoint.h"

namespace bs { struct __LimitLinearRangeInterop; }
namespace bs { class CSliderJoint; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptSliderJoint : public TScriptGameObjectWrapper<CSliderJoint, ScriptSliderJoint, ScriptJointWrapperBase>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "SliderJoint")

		ScriptSliderJoint(const GameObjectHandle<CSliderJoint>& nativeObject);
		~ScriptSliderJoint();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static float InternalGetPosition(ScriptSliderJoint* self);
		static float InternalGetSpeed(ScriptSliderJoint* self);
		static void InternalGetLimit(ScriptSliderJoint* self, __LimitLinearRangeInterop* __output);
		static void InternalSetLimit(ScriptSliderJoint* self, __LimitLinearRangeInterop* limit);
		static void InternalSetFlag(ScriptSliderJoint* self, SliderJointFlag flag, bool enabled);
		static bool InternalHasFlag(ScriptSliderJoint* self, SliderJointFlag flag);
	};
}
