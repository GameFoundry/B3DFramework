//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptPhysics.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Physics/BsPhysics.h"

namespace bs
{
	ScriptPhysics::ScriptPhysics(MonoObject* managedInstance)
		:ScriptObject(managedInstance)
	{
	}

	void ScriptPhysics::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_toggleCollision", (void*)&ScriptPhysics::Internal_toggleCollision);
		metaData.scriptClass->addInternalCall("Internal_isCollisionEnabled", (void*)&ScriptPhysics::Internal_isCollisionEnabled);
		metaData.scriptClass->addInternalCall("Internal__isUpdateInProgress", (void*)&ScriptPhysics::Internal__isUpdateInProgress);

	}

	void ScriptPhysics::Internal_toggleCollision(uint64_t groupA, uint64_t groupB, bool enabled)
	{
		Physics::instance().toggleCollision(groupA, groupB, enabled);
	}

	bool ScriptPhysics::Internal_isCollisionEnabled(uint64_t groupA, uint64_t groupB)
	{
		bool tmp__output;
		tmp__output = Physics::instance().isCollisionEnabled(groupA, groupB);

		bool __output;
		__output = tmp__output;

		return __output;
	}

	bool ScriptPhysics::Internal__isUpdateInProgress()
	{
		bool tmp__output;
		tmp__output = Physics::instance()._isUpdateInProgress();

		bool __output;
		__output = tmp__output;

		return __output;
	}
}
