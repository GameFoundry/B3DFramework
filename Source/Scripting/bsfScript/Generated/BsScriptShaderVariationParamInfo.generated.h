//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "../../../Foundation/bsfCore/Material/BsShader.h"
#include "../../../Foundation/bsfCore/Material/BsShader.h"
#include "BsScriptShaderVariationParamValue.generated.h"

namespace bs
{
	struct __ShaderVariationParamInfoInterop
	{
		MonoString* Name;
		MonoString* Identifier;
		bool IsInternal;
		MonoArray* Values;
	};

	class BS_SCR_BE_EXPORT ScriptShaderVariationParamInfo : public ScriptObject<ScriptShaderVariationParamInfo>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "ShaderVariationParamInfo")

		static MonoObject* Box(const __ShaderVariationParamInfoInterop& value);
		static __ShaderVariationParamInfoInterop Unbox(MonoObject* value);
		static ShaderVariationParamInfo FromInterop(const __ShaderVariationParamInfoInterop& value);
		static __ShaderVariationParamInfoInterop ToInterop(const ShaderVariationParamInfo& value);

	private:
		ScriptShaderVariationParamInfo(MonoObject* managedInstance);
	};
} // namespace bs
