//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptObjectWrapper.h"
#include "../../../Foundation/Core/Material/B3DShader.h"

namespace b3d
{
	struct __ShaderVariationParameterValueInterop
	{
		MonoString* Name;
		int32_t Value;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptShaderVariationParameterValue : public TScriptTypeDefinition<ScriptShaderVariationParameterValue>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "ShaderVariationParameterValue")

		static MonoObject* Box(const __ShaderVariationParameterValueInterop& value);
		static __ShaderVariationParameterValueInterop Unbox(MonoObject* value);
		static ShaderVariationParameterValue FromInterop(const __ShaderVariationParameterValueInterop& value);
		static __ShaderVariationParameterValueInterop ToInterop(const ShaderVariationParameterValue& value);

	private:
		ScriptShaderVariationParameterValue();

	};
}
