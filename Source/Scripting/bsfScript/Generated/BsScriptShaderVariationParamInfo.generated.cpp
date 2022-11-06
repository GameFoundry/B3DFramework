//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptShaderVariationParamInfo.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Material/BsShader.h"
#include "BsScriptShaderVariationParamValue.generated.h"

namespace bs
{
	ScriptShaderVariationParamInfo::ScriptShaderVariationParamInfo(MonoObject* managedInstance)
		:ScriptObject(managedInstance)
	{ }

	void ScriptShaderVariationParamInfo::InitRuntimeData()
	{ }

	MonoObject*ScriptShaderVariationParamInfo::Box(const __ShaderVariationParamInfoInterop& value)
	{
		return MonoUtil::Box(metaData.ScriptClass->GetInternalClassInternal(), (void*)&value);
	}

	__ShaderVariationParamInfoInterop ScriptShaderVariationParamInfo::Unbox(MonoObject* value)
	{
		return *(__ShaderVariationParamInfoInterop*)MonoUtil::Unbox(value);
	}

	ShaderVariationParamInfo ScriptShaderVariationParamInfo::FromInterop(const __ShaderVariationParamInfoInterop& value)
	{
		ShaderVariationParamInfo output;
		String tmpName;
		tmpName = MonoUtil::MonoToString(value.Name);
		output.Name = tmpName;
		String tmpIdentifier;
		tmpIdentifier = MonoUtil::MonoToString(value.Identifier);
		output.Identifier = tmpIdentifier;
		output.IsInternal = value.IsInternal;
		SmallVector<ShaderVariationParamValue, 4> vecValues;
		if(value.Values != nullptr)
		{
			ScriptArray arrayValues(value.Values);
			vecValues.resize(arrayValues.Size());
			for(int i = 0; i < (int)arrayValues.Size(); i++)
			{
				vecValues[i] = ScriptShaderVariationParamValue::FromInterop(arrayValues.Get<__ShaderVariationParamValueInterop>(i));
			}
		}
		output.Values = vecValues;

		return output;
	}

	__ShaderVariationParamInfoInterop ScriptShaderVariationParamInfo::ToInterop(const ShaderVariationParamInfo& value)
	{
		__ShaderVariationParamInfoInterop output;
		MonoString* tmpName;
		tmpName = MonoUtil::StringToMono(value.Name);
		output.Name = tmpName;
		MonoString* tmpIdentifier;
		tmpIdentifier = MonoUtil::StringToMono(value.Identifier);
		output.Identifier = tmpIdentifier;
		output.IsInternal = value.IsInternal;
		int arraySizeValues = (int)value.Values.size();
		MonoArray* vecValues;
		ScriptArray arrayValues = ScriptArray::Create<ScriptShaderVariationParamValue>(arraySizeValues);
		for(int i = 0; i < arraySizeValues; i++)
		{
			arrayValues.Set(i, ScriptShaderVariationParamValue::ToInterop(value.Values[i]));
		}
		vecValues = arrayValues.GetInternal();
		output.Values = vecValues;

		return output;
	}

}
