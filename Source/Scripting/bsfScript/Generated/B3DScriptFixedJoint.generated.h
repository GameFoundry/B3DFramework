//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "Wrappers/B3DScriptComponent.h"
#include "B3DScriptJoint.generated.h"

namespace b3d { class FixedJoint; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptFixedJoint : public TScriptGameObjectWrapper<FixedJoint, ScriptFixedJoint, ScriptJointWrapperBase>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "FixedJoint")

		ScriptFixedJoint(const TGameObjectHandle<FixedJoint>& nativeObject);
		~ScriptFixedJoint();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
	};
}
