//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObjectWrapper.h"
#include "../Extensions/BsShaderEx.h"
#include "../Extensions/BsShaderEx.h"
#include "../Extensions/BsShaderEx.h"

namespace b3d
{
	struct __ShaderParameterInterop
	{
		MonoString* Name;
		MonoString* Identifier;
		ShaderParameterType Type;
		Flags<ShaderParameterFlag> Flags;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptShaderParameter : public TScriptTypeDefinition<ScriptShaderParameter>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "ShaderParameter")

		static MonoObject* Box(const __ShaderParameterInterop& value);
		static __ShaderParameterInterop Unbox(MonoObject* value);
		static ShaderParameter FromInterop(const __ShaderParameterInterop& value);
		static __ShaderParameterInterop ToInterop(const ShaderParameter& value);

	private:
		ScriptShaderParameter();

	};
}
