//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "../Extensions/BsShaderEx.h"
#include "../Extensions/BsShaderEx.h"
#include "../Extensions/BsShaderEx.h"

namespace bs
{
	struct __ShaderParameterInterop
	{
		MonoString* name;
		MonoString* identifier;
		ShaderParameterType type;
		Flags<ShaderParameterFlag> flags;
	};

	class BS_SCR_BE_EXPORT ScriptShaderParameter : public ScriptObject<ScriptShaderParameter>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "ShaderParameter")

		static MonoObject* box(const __ShaderParameterInterop& value);
		static __ShaderParameterInterop unbox(MonoObject* value);
		static ShaderParameter fromInterop(const __ShaderParameterInterop& value);
		static __ShaderParameterInterop toInterop(const ShaderParameter& value);

	private:
		ScriptShaderParameter(MonoObject* managedInstance);

	};
}
