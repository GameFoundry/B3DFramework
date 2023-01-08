//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptShaderVariationParameterInformation.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Material/BsShader.h"
#include "BsScriptShaderVariationParameterValue.generated.h"

namespace bs
{
	ScriptShaderVariationParameterInformation::ScriptShaderVariationParameterInformation(MonoObject* managedInstance)
		:ScriptObject(managedInstance)
	{ }

	void ScriptShaderVariationParameterInformation::InitRuntimeData()
	{ }

	MonoObject*ScriptShaderVariationParameterInformation::Box(const __ShaderVariationParameterInformationInterop& value)
	{
		return MonoUtil::Box(metaData.ScriptClass->GetInternalClassInternal(), (void*)&value);
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
		SmallVector<ShaderVariationParameterValue, 4> vecValues;
		if(value.Values != nullptr)
		{
			ScriptArray arrayValues(value.Values);
			vecValues.resize(arrayValues.Size());
			for(int i = 0; i < (int)arrayValues.Size(); i++)
			{
				vecValues[i] = ScriptShaderVariationParameterValue::FromInterop(arrayValues.Get<__ShaderVariationParameterValueInterop>(i));
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
		int arraySizeValues = (int)value.Values.size();
		MonoArray* vecValues;
		ScriptArray arrayValues = ScriptArray::Create<ScriptShaderVariationParameterValue>(arraySizeValues);
		for(int i = 0; i < arraySizeValues; i++)
		{
			arrayValues.Set(i, ScriptShaderVariationParameterValue::ToInterop(value.Values[i]));
		}
		vecValues = arrayValues.GetInternal();
		output.Values = vecValues;

		return output;
	}

}
