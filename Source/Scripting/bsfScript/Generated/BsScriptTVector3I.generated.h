//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "../../../Foundation/bsfUtility/Math/BsVector3I.h"
#include "BsScriptObject.h"
#include "../../../Foundation/bsfUtility/Math/BsVector3I.h"

namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptVector3I : public ScriptObject<ScriptVector3I>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "Vector3I")

		static MonoObject* Box(const TVector3I<int32_t>& value);
		static TVector3I<int32_t> Unbox(MonoObject* value);

	private:
		ScriptVector3I(MonoObject* managedInstance);

	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptVector3UI : public ScriptObject<ScriptVector3UI>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "Vector3UI")

		static MonoObject* Box(const TVector3I<uint32_t>& value);
		static TVector3I<uint32_t> Unbox(MonoObject* value);

	private:
		ScriptVector3UI(MonoObject* managedInstance);

	};
}
