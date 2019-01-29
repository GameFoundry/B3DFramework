//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "../../../Foundation/bsfCore/Physics/BsHingeJoint.h"

namespace bs
{
	class BS_SCR_BE_EXPORT ScriptHingeJointDrive : public ScriptObject<ScriptHingeJointDrive>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "HingeJointDrive")

		static MonoObject* box(const HingeJointDrive& value);
		static HingeJointDrive unbox(MonoObject* value);

	private:
		ScriptHingeJointDrive(MonoObject* managedInstance);

	};
}
