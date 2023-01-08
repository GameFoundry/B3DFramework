//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptComponent.h"
#include "BsScriptCJoint.generated.h"

namespace bs { class CFixedJoint; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptFixedJoint : public TScriptComponent<ScriptFixedJoint, CFixedJoint, ScriptJointBase>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "FixedJoint")

		ScriptFixedJoint(MonoObject* managedInstance, const GameObjectHandle<CFixedJoint>& value);

	private:
	};
}
