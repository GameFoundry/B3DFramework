//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "Wrappers/B3DScriptComponent.h"
#include "B3DScriptJoint.generated.h"
#include "../../../Foundation/Core/Components/B3DSliderJoint.h"
#include "../../../Foundation/Core/Components/B3DJoint.h"

namespace b3d { class SliderJoint; }
namespace b3d { struct __LimitLinearRangeInterop; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptSliderJoint : public TScriptGameObjectWrapper<SliderJoint, ScriptSliderJoint, ScriptJointWrapperBase>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "SliderJoint")

		ScriptSliderJoint(const TGameObjectHandle<SliderJoint>& nativeObject);
		~ScriptSliderJoint();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static float InternalGetPosition(ScriptSliderJoint* self);
		static float InternalGetSpeed(ScriptSliderJoint* self);
		static void InternalSetLimit(ScriptSliderJoint* self, __LimitLinearRangeInterop* limit);
		static void InternalGetLimit(ScriptSliderJoint* self, __LimitLinearRangeInterop* __output);
		static void InternalSetFlag(ScriptSliderJoint* self, SliderJointFlag flag, bool enabled);
		static bool InternalHasFlag(ScriptSliderJoint* self, SliderJointFlag flag);
	};
}
