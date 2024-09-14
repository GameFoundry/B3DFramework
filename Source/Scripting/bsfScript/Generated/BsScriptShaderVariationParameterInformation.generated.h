//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObjectWrapper.h"
#include "../../../Foundation/bsfCore/Material/BsShader.h"
#include "../../../Foundation/bsfCore/Material/BsShader.h"
#include "BsScriptShaderVariationParameterValue.generated.h"

namespace bs
{
	struct __ShaderVariationParameterInformationInterop
	{
		MonoString* Name;
		MonoString* Identifier;
		bool IsInternal;
		MonoArray* Values;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptShaderVariationParameterInformation : public TScriptStructWrapper<ScriptShaderVariationParameterInformation>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "ShaderVariationParameterInformation")

		static MonoObject* Box(const __ShaderVariationParameterInformationInterop& value);
		static __ShaderVariationParameterInformationInterop Unbox(MonoObject* value);
		static ShaderVariationParameterInformation FromInterop(const __ShaderVariationParameterInformationInterop& value);
		static __ShaderVariationParameterInformationInterop ToInterop(const ShaderVariationParameterInformation& value);

	private:
		ScriptShaderVariationParameterInformation();

	};
}
