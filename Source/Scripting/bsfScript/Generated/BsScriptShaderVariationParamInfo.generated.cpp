//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
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

	MonoObject*ScriptShaderVariationParamInfo::box(const __ShaderVariationParamInfoInterop& value)
	{
		return MonoUtil::Box(metaData.scriptClass->_getInternalClass(), (void*)&value);
	}

	__ShaderVariationParamInfoInterop ScriptShaderVariationParamInfo::Unbox(MonoObject* value)
	{
		return *(__ShaderVariationParamInfoInterop*)MonoUtil::unbox(value);
	}

	ShaderVariationParamInfo ScriptShaderVariationParamInfo::FromInterop(const __ShaderVariationParamInfoInterop& value)
	{
		ShaderVariationParamInfo output;
		String tmpname;
		tmpname = MonoUtil::monoToString(value.name);
		output.name = tmpname;
		String tmpidentifier;
		tmpidentifier = MonoUtil::monoToString(value.identifier);
		output.identifier = tmpidentifier;
		output.isInternal = value.isInternal;
		SmallVector<ShaderVariationParamValue, 4> vecvalues;
		if(value.values != nullptr)
		{
			ScriptArray Arrayvalues(value.values);
			vecvalues.resize(arrayvalues.size());
			for(int i = 0; i < (int)arrayvalues.size(); i++)
			{
				vecvalues[i] = ScriptShaderVariationParamValue::fromInterop(arrayvalues.get<__ShaderVariationParamValueInterop>(i));
			}
		}
		output.values = vecvalues;

		return output;
	}

	__ShaderVariationParamInfoInterop ScriptShaderVariationParamInfo::ToInterop(const ShaderVariationParamInfo& value)
	{
		__ShaderVariationParamInfoInterop output;
		MonoString* tmpname;
		tmpname = MonoUtil::stringToMono(value.name);
		output.name = tmpname;
		MonoString* tmpidentifier;
		tmpidentifier = MonoUtil::stringToMono(value.identifier);
		output.identifier = tmpidentifier;
		output.isInternal = value.isInternal;
		int arraySizevalues = (int)value.values.size();
		MonoArray* vecvalues;
		ScriptArray arrayvalues = ScriptArray::create<ScriptShaderVariationParamValue>(arraySizevalues);
		for(int i = 0; i < arraySizevalues; i++)
		{
			arrayvalues.set(i, ScriptShaderVariationParamValue::toInterop(value.values[i]));
		}
		vecvalues = arrayvalues.getInternal();
		output.values = vecvalues;

		return output;
	}

}
