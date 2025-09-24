//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptShaderVariationParameterInformation.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Material/BsShader.h"
#include "BsScriptShaderVariationParameterValue.generated.h"

namespace b3d
{
	ScriptShaderVariationParameterInformation::ScriptShaderVariationParameterInformation()
	{ }

	MonoObject* ScriptShaderVariationParameterInformation::Box(const __ShaderVariationParameterInformationInterop& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	__ShaderVariationParameterInformationInterop ScriptShaderVariationParameterInformation::Unbox(MonoObject* value)
	{
		return *(__ShaderVariationParameterInformationInterop*)MonoUtil::Unbox(value);
	}

	ShaderVariationParameterInformation ScriptShaderVariationParameterInformation::FromInterop(const __ShaderVariationParameterInformationInterop& value)
	{
		ShaderVariationParameterInformation output;
		String tmpName;
		tmpName = MonoUtil::MonoToString(value.Name);
		output.Name = tmpName;
		String tmpIdentifier;
		tmpIdentifier = MonoUtil::MonoToString(value.Identifier);
		output.Identifier = tmpIdentifier;
		output.IsInternal = value.IsInternal;
		TInlineArray<ShaderVariationParameterValue, 4> vecValues;
		if(value.Values != nullptr)
		{
			ScriptArray scriptArrayValues(value.Values);
			vecValues.resize(scriptArrayValues.Size());
			for(int elementIndex = 0; elementIndex < (int)scriptArrayValues.Size(); elementIndex++)
			{
				vecValues[elementIndex] = ScriptShaderVariationParameterValue::FromInterop(scriptArrayValues.Get<__ShaderVariationParameterValueInterop>(elementIndex));
			}
		}
		output.Values = vecValues;

		return output;
	}

	__ShaderVariationParameterInformationInterop ScriptShaderVariationParameterInformation::ToInterop(const ShaderVariationParameterInformation& value)
	{
		__ShaderVariationParameterInformationInterop output;
		MonoString* tmpName;
		tmpName = MonoUtil::StringToMono(value.Name);
		output.Name = tmpName;
		MonoString* tmpIdentifier;
		tmpIdentifier = MonoUtil::StringToMono(value.Identifier);
		output.Identifier = tmpIdentifier;
		output.IsInternal = value.IsInternal;
		int elementCountValues = (int)value.Values.size();
		MonoArray* vecValues;
		ScriptArray scriptArrayValues = ScriptArray::Create<ScriptShaderVariationParameterValue>(elementCountValues);
		for(int elementIndex = 0; elementIndex < elementCountValues; elementIndex++)
		{
			scriptArrayValues.Set(elementIndex, ScriptShaderVariationParameterValue::ToInterop(value.Values[elementIndex]));
		}
		vecValues = scriptArrayValues.GetInternal();
		output.Values = vecValues;

		return output;
	}

}
