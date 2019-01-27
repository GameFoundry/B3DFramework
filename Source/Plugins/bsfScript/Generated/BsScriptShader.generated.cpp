//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptShader.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../../../Foundation/bsfCore/Material/BsShader.h"
#include "BsScriptShaderParameter.generated.h"
#include "../../../Extensions/BsShaderEx.h"

namespace bs
{
	ScriptShader::ScriptShader(MonoObject* managedInstance, const ResourceHandle<Shader>& value)
		:TScriptResource(managedInstance, value)
	{
	}

	void ScriptShader::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_GetRef", (void*)&ScriptShader::Internal_getRef);
		metaData.scriptClass->addInternalCall("Internal_getParameters", (void*)&ScriptShader::Internal_getParameters);

	}

	 MonoObject*ScriptShader::createInstance()
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		return metaData.scriptClass->createInstance("bool", ctorParams);
	}
	MonoObject* ScriptShader::Internal_getRef(ScriptShader* thisPtr)
	{
		return thisPtr->getRRef();
	}

	MonoArray* ScriptShader::Internal_getParameters(ScriptShader* thisPtr)
	{
		Vector<ShaderParameter> vec__output;
		vec__output = ShaderEx::getParameters(thisPtr->getHandle());

		MonoArray* __output;
		int arraySize__output = (int)vec__output.size();
		ScriptArray array__output = ScriptArray::create<ScriptShaderParameter>(arraySize__output);
		for(int i = 0; i < arraySize__output; i++)
		{
			array__output.set(i, ScriptShaderParameter::toInterop(vec__output[i]));
		}
		__output = array__output.getInternal();

		return __output;
	}
}
