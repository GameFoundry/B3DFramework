//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "../../../Foundation/bsfUtility/Math/BsVector4I.h"

namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptVector4I : public ScriptObject<ScriptVector4I>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "Vector4I")

		static MonoObject* Box(const Vector4I& value);
		static Vector4I Unbox(MonoObject* value);

	private:
		ScriptVector4I(MonoObject* managedInstance);

	};
}
