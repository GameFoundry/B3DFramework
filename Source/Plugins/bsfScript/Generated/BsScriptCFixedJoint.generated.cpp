//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptCFixedJoint.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../../../Foundation/bsfCore/Components/BsCFixedJoint.h"

namespace bs
{
	ScriptCFixedJoint::ScriptCFixedJoint(MonoObject* managedInstance, const GameObjectHandle<CFixedJoint>& value)
		:TScriptComponent(managedInstance, value)
	{
	}

	void ScriptCFixedJoint::initRuntimeData()
	{

	}

}
