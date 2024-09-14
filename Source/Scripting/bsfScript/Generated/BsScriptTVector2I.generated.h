//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObjectWrapper.h"
#include "../../../Foundation/bsfUtility/Math/BsVector2I.h"
#include "BsScriptObjectWrapper.h"
#include "../../../Foundation/bsfUtility/Math/BsVector2I.h"

namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptVector2I : public TScriptStructWrapper<ScriptVector2I>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "Vector2I")

		static MonoObject* Box(const TVector2I<int32_t>& value);
		static TVector2I<int32_t> Unbox(MonoObject* value);

	private:
		ScriptVector2I();

	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptVector2UI : public TScriptStructWrapper<ScriptVector2UI>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "Vector2UI")

		static MonoObject* Box(const TVector2I<uint32_t>& value);
		static TVector2I<uint32_t> Unbox(MonoObject* value);

	private:
		ScriptVector2UI();

	};
}
