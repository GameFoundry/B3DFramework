//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptMorphShape.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"

namespace bs
{
	ScriptMorphShape::ScriptMorphShape(MonoObject* managedInstance, const SPtr<MorphShape>& value)
		:TScriptReflectable(managedInstance, value)
	{
	}

	void ScriptMorphShape::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_getName", (void*)&ScriptMorphShape::Internal_getName);
		metaData.scriptClass->addInternalCall("Internal_getWeight", (void*)&ScriptMorphShape::Internal_getWeight);

	}

	MonoObject* ScriptMorphShape::create(const SPtr<MorphShape>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.scriptClass->createInstance("bool", ctorParams);
		new (bs_alloc<ScriptMorphShape>()) ScriptMorphShape(managedInstance, value);
		return managedInstance;
	}
	MonoString* ScriptMorphShape::Internal_getName(ScriptMorphShape* thisPtr)
	{
		String tmp__output;
		tmp__output = thisPtr->getInternal()->getName();

		MonoString* __output;
		__output = MonoUtil::stringToMono(tmp__output);

		return __output;
	}

	float ScriptMorphShape::Internal_getWeight(ScriptMorphShape* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->getInternal()->getWeight();

		float __output;
		__output = tmp__output;

		return __output;
	}
}
