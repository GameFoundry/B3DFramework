//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptMorphChannel.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "BsScriptMorphShape.generated.h"

namespace bs
{
	ScriptMorphChannel::ScriptMorphChannel(MonoObject* managedInstance, const SPtr<MorphChannel>& value)
		:TScriptReflectable(managedInstance, value)
	{
	}

	void ScriptMorphChannel::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_getName", (void*)&ScriptMorphChannel::Internal_getName);
		metaData.scriptClass->addInternalCall("Internal_getShapes", (void*)&ScriptMorphChannel::Internal_getShapes);

	}

	MonoObject* ScriptMorphChannel::create(const SPtr<MorphChannel>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.scriptClass->createInstance("bool", ctorParams);
		new (bs_alloc<ScriptMorphChannel>()) ScriptMorphChannel(managedInstance, value);
		return managedInstance;
	}
	MonoString* ScriptMorphChannel::Internal_getName(ScriptMorphChannel* thisPtr)
	{
		String tmp__output;
		tmp__output = thisPtr->getInternal()->getName();

		MonoString* __output;
		__output = MonoUtil::stringToMono(tmp__output);

		return __output;
	}

	MonoArray* ScriptMorphChannel::Internal_getShapes(ScriptMorphChannel* thisPtr)
	{
		Vector<SPtr<MorphShape>> vec__output;
		vec__output = thisPtr->getInternal()->getShapes();

		MonoArray* __output;
		int arraySize__output = (int)vec__output.size();
		ScriptArray array__output = ScriptArray::create<ScriptMorphShape>(arraySize__output);
		for(int i = 0; i < arraySize__output; i++)
		{
			SPtr<MorphShape> arrayElemPtr__output = vec__output[i];
			MonoObject* arrayElem__output;
			arrayElem__output = ScriptMorphShape::create(arrayElemPtr__output);
			array__output.set(i, arrayElem__output);
		}
		__output = array__output.getInternal();

		return __output;
	}
}
