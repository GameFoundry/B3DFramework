//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObjectWrapper.h"
#include "../../../Foundation/bsfUtility/Utility/BsUtil.h"
#include "BsScriptObjectWrapper.h"
#include "../../../Foundation/bsfUtility/Utility/BsUtil.h"

namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptSize3 : public TScriptTypeDefinition<ScriptSize3>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "Size3")

		static MonoObject* Box(const TSize3<float>& value);
		static TSize3<float> Unbox(MonoObject* value);

	private:
		ScriptSize3();

	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptSize3UI : public TScriptTypeDefinition<ScriptSize3UI>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "Size3UI")

		static MonoObject* Box(const TSize3<uint32_t>& value);
		static TSize3<uint32_t> Unbox(MonoObject* value);

	private:
		ScriptSize3UI();

	};
}
