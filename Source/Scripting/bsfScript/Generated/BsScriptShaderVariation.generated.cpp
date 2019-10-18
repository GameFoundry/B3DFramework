//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptShaderVariation.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"

namespace bs
{
	ScriptShaderVariation::ScriptShaderVariation(MonoObject* managedInstance, const SPtr<ShaderVariation>& value)
		:TScriptReflectable(managedInstance, value)
	{
	}

	void ScriptShaderVariation::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_ShaderVariation", (void*)&ScriptShaderVariation::Internal_ShaderVariation);
		metaData.scriptClass->addInternalCall("Internal_getInt", (void*)&ScriptShaderVariation::Internal_getInt);
		metaData.scriptClass->addInternalCall("Internal_getUInt", (void*)&ScriptShaderVariation::Internal_getUInt);
		metaData.scriptClass->addInternalCall("Internal_getFloat", (void*)&ScriptShaderVariation::Internal_getFloat);
		metaData.scriptClass->addInternalCall("Internal_getBool", (void*)&ScriptShaderVariation::Internal_getBool);
		metaData.scriptClass->addInternalCall("Internal_setInt", (void*)&ScriptShaderVariation::Internal_setInt);
		metaData.scriptClass->addInternalCall("Internal_setUInt", (void*)&ScriptShaderVariation::Internal_setUInt);
		metaData.scriptClass->addInternalCall("Internal_setFloat", (void*)&ScriptShaderVariation::Internal_setFloat);
		metaData.scriptClass->addInternalCall("Internal_setBool", (void*)&ScriptShaderVariation::Internal_setBool);
		metaData.scriptClass->addInternalCall("Internal_removeParam", (void*)&ScriptShaderVariation::Internal_removeParam);
		metaData.scriptClass->addInternalCall("Internal_hasParam", (void*)&ScriptShaderVariation::Internal_hasParam);
		metaData.scriptClass->addInternalCall("Internal_clearParams", (void*)&ScriptShaderVariation::Internal_clearParams);
		metaData.scriptClass->addInternalCall("Internal_getParamNames", (void*)&ScriptShaderVariation::Internal_getParamNames);

	}

	MonoObject* ScriptShaderVariation::create(const SPtr<ShaderVariation>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.scriptClass->createInstance("bool", ctorParams);
		new (bs_alloc<ScriptShaderVariation>()) ScriptShaderVariation(managedInstance, value);
		return managedInstance;
	}
	void ScriptShaderVariation::Internal_ShaderVariation(MonoObject* managedInstance)
	{
		SPtr<ShaderVariation> instance = bs_shared_ptr_new<ShaderVariation>();
		new (bs_alloc<ScriptShaderVariation>())ScriptShaderVariation(managedInstance, instance);
	}

	int32_t ScriptShaderVariation::Internal_getInt(ScriptShaderVariation* thisPtr, MonoString* name)
	{
		int32_t tmp__output;
		String tmpname;
		tmpname = MonoUtil::monoToString(name);
		tmp__output = thisPtr->getInternal()->getInt(tmpname);

		int32_t __output;
		__output = tmp__output;

		return __output;
	}

	uint32_t ScriptShaderVariation::Internal_getUInt(ScriptShaderVariation* thisPtr, MonoString* name)
	{
		uint32_t tmp__output;
		String tmpname;
		tmpname = MonoUtil::monoToString(name);
		tmp__output = thisPtr->getInternal()->getUInt(tmpname);

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	float ScriptShaderVariation::Internal_getFloat(ScriptShaderVariation* thisPtr, MonoString* name)
	{
		float tmp__output;
		String tmpname;
		tmpname = MonoUtil::monoToString(name);
		tmp__output = thisPtr->getInternal()->getFloat(tmpname);

		float __output;
		__output = tmp__output;

		return __output;
	}

	bool ScriptShaderVariation::Internal_getBool(ScriptShaderVariation* thisPtr, MonoString* name)
	{
		bool tmp__output;
		String tmpname;
		tmpname = MonoUtil::monoToString(name);
		tmp__output = thisPtr->getInternal()->getBool(tmpname);

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptShaderVariation::Internal_setInt(ScriptShaderVariation* thisPtr, MonoString* name, int32_t value)
	{
		String tmpname;
		tmpname = MonoUtil::monoToString(name);
		thisPtr->getInternal()->setInt(tmpname, value);
	}

	void ScriptShaderVariation::Internal_setUInt(ScriptShaderVariation* thisPtr, MonoString* name, uint32_t value)
	{
		String tmpname;
		tmpname = MonoUtil::monoToString(name);
		thisPtr->getInternal()->setUInt(tmpname, value);
	}

	void ScriptShaderVariation::Internal_setFloat(ScriptShaderVariation* thisPtr, MonoString* name, float value)
	{
		String tmpname;
		tmpname = MonoUtil::monoToString(name);
		thisPtr->getInternal()->setFloat(tmpname, value);
	}

	void ScriptShaderVariation::Internal_setBool(ScriptShaderVariation* thisPtr, MonoString* name, bool value)
	{
		String tmpname;
		tmpname = MonoUtil::monoToString(name);
		thisPtr->getInternal()->setBool(tmpname, value);
	}

	void ScriptShaderVariation::Internal_removeParam(ScriptShaderVariation* thisPtr, MonoString* paramName)
	{
		String tmpparamName;
		tmpparamName = MonoUtil::monoToString(paramName);
		thisPtr->getInternal()->removeParam(tmpparamName);
	}

	bool ScriptShaderVariation::Internal_hasParam(ScriptShaderVariation* thisPtr, MonoString* paramName)
	{
		bool tmp__output;
		String tmpparamName;
		tmpparamName = MonoUtil::monoToString(paramName);
		tmp__output = thisPtr->getInternal()->hasParam(tmpparamName);

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptShaderVariation::Internal_clearParams(ScriptShaderVariation* thisPtr)
	{
		thisPtr->getInternal()->clearParams();
	}

	MonoArray* ScriptShaderVariation::Internal_getParamNames(ScriptShaderVariation* thisPtr)
	{
		Vector<String> vec__output;
		vec__output = thisPtr->getInternal()->getParamNames();

		MonoArray* __output;
		int arraySize__output = (int)vec__output.size();
		ScriptArray array__output = ScriptArray::create<String>(arraySize__output);
		for(int i = 0; i < arraySize__output; i++)
		{
			array__output.set(i, vec__output[i]);
		}
		__output = array__output.getInternal();

		return __output;
	}
}
