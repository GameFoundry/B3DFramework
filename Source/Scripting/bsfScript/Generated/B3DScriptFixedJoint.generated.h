//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptComponent.h"
#include "BsScriptJoint.generated.h"

namespace b3d { class FixedJoint; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptFixedJoint : public TScriptGameObjectWrapper<FixedJoint, ScriptFixedJoint, ScriptJointWrapperBase>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "FixedJoint")

		ScriptFixedJoint(const GameObjectHandle<FixedJoint>& nativeObject);
		~ScriptFixedJoint();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
	};
}
