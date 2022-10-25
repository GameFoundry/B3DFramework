//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "../../../Foundation/bsfCore/Physics/BsD6Joint.h"

namespace bs
{
	class BS_SCR_BE_EXPORT ScriptD6JointDrive : public ScriptObject<ScriptD6JointDrive>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "D6JointDrive")

		static MonoObject* Box(const D6JointDrive& value);
		static D6JointDrive Unbox(MonoObject* value);

	private:
		ScriptD6JointDrive(MonoObject* managedInstance);
	};
} // namespace bs
