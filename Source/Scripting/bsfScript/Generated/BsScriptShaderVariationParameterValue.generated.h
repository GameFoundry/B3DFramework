//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "../../../Foundation/bsfCore/Material/BsShader.h"

namespace bs
{
	struct __ShaderVariationParameterValueInterop
	{
		MonoString* Name;
		int32_t Value;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptShaderVariationParameterValue : public ScriptObject<ScriptShaderVariationParameterValue>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "ShaderVariationParameterValue")

		static MonoObject* Box(const __ShaderVariationParameterValueInterop& value);
		static __ShaderVariationParameterValueInterop Unbox(MonoObject* value);
		static ShaderVariationParameterValue FromInterop(const __ShaderVariationParameterValueInterop& value);
		static __ShaderVariationParameterValueInterop ToInterop(const ShaderVariationParameterValue& value);

	private:
		ScriptShaderVariationParameterValue(MonoObject* managedInstance);

	};
}
