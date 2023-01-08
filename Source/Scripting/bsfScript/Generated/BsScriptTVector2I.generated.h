//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "../../../Foundation/bsfUtility/Math/BsVector2I.h"
#include "BsScriptObject.h"
#include "../../../Foundation/bsfUtility/Math/BsVector2I.h"

namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptVector2I : public ScriptObject<ScriptVector2I>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "Vector2I")

		static MonoObject* Box(const TVector2I<int32_t>& value);
		static TVector2I<int32_t> Unbox(MonoObject* value);

	private:
		ScriptVector2I(MonoObject* managedInstance);

	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptVector2UI : public ScriptObject<ScriptVector2UI>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "Vector2UI")

		static MonoObject* Box(const TVector2I<uint32_t>& value);
		static TVector2I<uint32_t> Unbox(MonoObject* value);

	private:
		ScriptVector2UI(MonoObject* managedInstance);

	};
}
