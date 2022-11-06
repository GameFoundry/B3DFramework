//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "../../../Foundation/bsfCore/Physics/BsJoint.h"

namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptSpring : public ScriptObject<ScriptSpring>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "Spring")

		static MonoObject* Box(const Spring& value);
		static Spring Unbox(MonoObject* value);

	private:
		ScriptSpring(MonoObject* managedInstance);

	};
}
