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
		MonoString* Name;
		MonoString* Identifier;
		ShaderParameterType Type;
		Flags<ShaderParameterFlag> Flags;
	};

	class BS_SCR_BE_EXPORT ScriptShaderParameter : public ScriptObject<ScriptShaderParameter>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "ShaderParameter")

		static MonoObject* Box(const __ShaderParameterInterop& value);
		static __ShaderParameterInterop Unbox(MonoObject* value);
		static ShaderParameter FromInterop(const __ShaderParameterInterop& value);
		static __ShaderParameterInterop ToInterop(const ShaderParameter& value);

	private:
		ScriptShaderParameter(MonoObject* managedInstance);
	};
} // namespace bs
