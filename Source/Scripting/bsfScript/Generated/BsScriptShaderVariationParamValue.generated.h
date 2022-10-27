//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "../../../Foundation/bsfCore/Material/BsShader.h"

namespace bs
{
	struct __ShaderVariationParamValueInterop
	{
		MonoString* Name;
		int32_t Value;
	};

	class BS_SCR_BE_EXPORT ScriptShaderVariationParamValue : public ScriptObject<ScriptShaderVariationParamValue>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "ShaderVariationParamValue")

		static MonoObject* Box(const __ShaderVariationParamValueInterop& value);
		static __ShaderVariationParamValueInterop Unbox(MonoObject* value);
		static ShaderVariationParamValue FromInterop(const __ShaderVariationParamValueInterop& value);
		static __ShaderVariationParamValueInterop ToInterop(const ShaderVariationParamValue& value);

	private:
		ScriptShaderVariationParamValue(MonoObject* managedInstance);
	};
} // namespace bs
