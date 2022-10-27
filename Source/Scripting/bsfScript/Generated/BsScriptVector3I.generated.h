//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "../../../Foundation/bsfUtility/Math/BsVector3I.h"

namespace bs
{
	class BS_SCR_BE_EXPORT ScriptVector3I : public ScriptObject<ScriptVector3I>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "Vector3I")

		static MonoObject* Box(const Vector3I& value);
		static Vector3I Unbox(MonoObject* value);

	private:
		ScriptVector3I(MonoObject* managedInstance);
	};
} // namespace bs
