//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
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

	void ScriptMorphChannel::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_GetName", (void*)&ScriptMorphChannel::InternalGetName);
		metaData.ScriptClass->AddInternalCall("Internal_GetShapes", (void*)&ScriptMorphChannel::InternalGetShapes);

	}

	MonoObject* ScriptMorphChannel::Create(const SPtr<MorphChannel>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.ScriptClass->CreateInstance("bool", ctorParams);
		new (B3DAllocate<ScriptMorphChannel>()) ScriptMorphChannel(managedInstance, value);
		return managedInstance;
	}
	MonoString* ScriptMorphChannel::InternalGetName(ScriptMorphChannel* self)
	{
		String tmp__output;
		tmp__output = self->GetInternal()->GetName();

		MonoString* __output;
		__output = MonoUtil::StringToMono(tmp__output);

		return __output;
	}

	MonoArray* ScriptMorphChannel::InternalGetShapes(ScriptMorphChannel* self)
	{
		Vector<SPtr<MorphShape>> nativeArray__output;
		nativeArray__output = self->GetInternal()->GetShapes();

		MonoArray* __output;
		int elementCount__output = (int)nativeArray__output.size();
		ScriptArray scriptArray__output = ScriptArray::Create<ScriptMorphShape>(elementCount__output);
		for(int elementIndex = 0; elementIndex < elementCount__output; elementIndex++)
		{
			SPtr<MorphShape> arrayElementPointer__output = nativeArray__output[elementIndex];
			MonoObject* arrayElement__output;
			arrayElement__output = ScriptMorphShape::Create(arrayElementPointer__output);
			scriptArray__output.Set(elementIndex, arrayElement__output);
		}
		__output = scriptArray__output.GetInternal();

		return __output;
	}
}
