//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptComponent.h"
#include "BsScriptCJoint.generated.h"

namespace bs { class CFixedJoint; }
namespace bs
{
	class BS_SCR_BE_EXPORT ScriptCFixedJoint : public TScriptComponent<ScriptCFixedJoint, CFixedJoint, ScriptCJointBase>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "FixedJoint")

		ScriptCFixedJoint(MonoObject* managedInstance, const GameObjectHandle<CFixedJoint>& value);

	private:
	};
}
