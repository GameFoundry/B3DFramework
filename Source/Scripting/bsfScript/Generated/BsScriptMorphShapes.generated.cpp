//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptMorphShapes.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "BsScriptMorphChannel.generated.h"

namespace bs
{
	ScriptMorphShapes::ScriptMorphShapes(MonoObject* managedInstance, const SPtr<MorphShapes>& value)
		:TScriptReflectable(managedInstance, value)
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
		new (B3DAllocate<ScriptMorphShapes>()) ScriptMorphShapes(managedInstance, value);
		return managedInstance;
	}
	MonoArray* ScriptMorphShapes::InternalGetChannels(ScriptMorphShapes* thisPtr)
	{
		Vector<SPtr<MorphChannel>> nativeArray__output;
		nativeArray__output = thisPtr->GetInternal()->GetChannels();

		MonoArray* __output;
		int elementCount__output = (int)nativeArray__output.size();
		ScriptArray scriptArray__output = ScriptArray::Create<ScriptMorphChannel>(elementCount__output);
		for(int elementIndex = 0; elementIndex < elementCount__output; elementIndex++)
		{
			SPtr<MorphChannel> arrayElementPointer__output = nativeArray__output[elementIndex];
			MonoObject* arrayElement__output;
			arrayElement__output = ScriptMorphChannel::Create(arrayElementPointer__output);
			scriptArray__output.Set(elementIndex, arrayElement__output);
		}
		__output = scriptArray__output.GetInternal();

		return __output;
	}
}
