//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "../../../Foundation/bsfCore/Physics/BsJoint.h"

namespace bs
{
	class BS_SCR_BE_EXPORT ScriptSpring : public ScriptObject<ScriptSpring>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "Spring")

		static MonoObject* Box(const Spring& value);
		static Spring Unbox(MonoObject* value);

	private:
		ScriptSpring(MonoObject* managedInstance);
	};
} // namespace bs
