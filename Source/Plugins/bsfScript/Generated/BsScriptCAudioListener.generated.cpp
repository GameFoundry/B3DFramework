//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptCAudioListener.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../../../Foundation/bsfCore/Components/BsCAudioListener.h"

namespace bs
{
	ScriptCAudioListener::ScriptCAudioListener(MonoObject* managedInstance, const GameObjectHandle<CAudioListener>& value)
		:TScriptComponent(managedInstance, value)
	{
	}

	void ScriptCAudioListener::initRuntimeData()
	{

	}

}
