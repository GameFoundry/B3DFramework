//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptComponent.h"
#include "BsScriptCJoint.generated.h"

namespace bs { class CFixedJoint; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptFixedJoint : public TScriptGameObjectWrapper<CFixedJoint, ScriptFixedJoint, ScriptJointWrapperBase>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "FixedJoint")

		ScriptFixedJoint(const GameObjectHandle<CFixedJoint>& nativeObject);
		~ScriptFixedJoint();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
	};
}
