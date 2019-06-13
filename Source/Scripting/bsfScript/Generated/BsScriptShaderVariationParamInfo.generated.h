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
		MonoString* name;
		MonoString* identifier;
		bool isInternal;
		MonoArray* values;
	};

	class BS_SCR_BE_EXPORT ScriptShaderVariationParamInfo : public ScriptObject<ScriptShaderVariationParamInfo>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "ShaderVariationParamInfo")

		static MonoObject* box(const __ShaderVariationParamInfoInterop& value);
		static __ShaderVariationParamInfoInterop unbox(MonoObject* value);
		static ShaderVariationParamInfo fromInterop(const __ShaderVariationParamInfoInterop& value);
		static __ShaderVariationParamInfoInterop toInterop(const ShaderVariationParamInfo& value);

	private:
		ScriptShaderVariationParamInfo(MonoObject* managedInstance);

	};
}
