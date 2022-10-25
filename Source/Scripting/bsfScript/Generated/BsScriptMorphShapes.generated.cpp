//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptMorphShapes.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "BsScriptMorphChannel.generated.h"

namespace bs
{
	ScriptMorphShapes::ScriptMorphShapes(MonoObject* managedInstance, const SPtr<MorphShapes>& value)
		: TScriptReflectable(managedInstance, value)
	{
	}

	void ScriptMorphShapes::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_GetChannels", (void*)&ScriptMorphShapes::InternalGetChannels);
	}

	MonoObject* ScriptMorphShapes::Create(const SPtr<MorphShapes>& value)
	{
		if(value == nullptr) return nullptr;

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.ScriptClass->CreateInstance("bool", ctorParams);
		new(bs_alloc<ScriptMorphShapes>()) ScriptMorphShapes(managedInstance, value);
		return managedInstance;
	}

	MonoArray* ScriptMorphShapes::InternalGetChannels(ScriptMorphShapes* thisPtr)
	{
		Vector<SPtr<MorphChannel>> vec__output;
		vec__output = thisPtr->GetInternal()->GetChannels();

		MonoArray* __output;
		int arraySize__output = (int)vec__output.size();
		ScriptArray array__output = ScriptArray::Create<ScriptMorphChannel>(arraySize__output);
		for(int i = 0; i < arraySize__output; i++)
		{
			SPtr<MorphChannel> arrayElemPtr__output = vec__output[i];
			MonoObject* arrayElem__output;
			arrayElem__output = ScriptMorphChannel::Create(arrayElemPtr__output);
			array__output.Set(i, arrayElem__output);
		}
		__output = array__output.GetInternal();

		return __output;
	}
} // namespace bs
