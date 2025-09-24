//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptResourceWrapper.h"
#include "../../../Foundation/Core/Material/B3DShader.h"
#include "../Extensions/B3DShaderEx.h"

namespace b3d { class Shader; }
namespace b3d { struct __ShaderVariationParameterInformationInterop; }
namespace b3d { class ShaderEx; }
namespace b3d { struct __ShaderParameterInterop; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptShader : public TScriptResourceWrapper<Shader, ScriptShader>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "Shader")

		ScriptShader(const TResourceHandle<Shader>& nativeObject);
		~ScriptShader();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static MonoObject* InternalGetRef(ScriptShader* self);

		static MonoArray* InternalGetVariationParams(ScriptShader* self);
		static MonoArray* InternalGetParameters(ScriptShader* self);
	};
}
