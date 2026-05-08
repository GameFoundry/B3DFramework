//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptMorphChannel.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "B3DScriptMorphShape.generated.h"

namespace b3d
{
	ScriptMorphChannel::ScriptMorphChannel(const TShared<MorphChannel>& nativeObject)
		:TScriptReflectableWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptMorphChannel::~ScriptMorphChannel()
	{
		UnregisterEvents();
	}

	void ScriptMorphChannel::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetName", (void*)&ScriptMorphChannel::InternalGetName);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetShapes", (void*)&ScriptMorphChannel::InternalGetShapes);

	}

	MonoObject* ScriptMorphChannel::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	MonoString* ScriptMorphChannel::InternalGetName(ScriptMorphChannel* self)
	{
		String tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<MorphChannel*>(self->GetNativeObject())->GetName();

		MonoString* __output;
		__output = MonoUtil::StringToMono(tmp__output);

		return __output;
	}

	MonoArray* ScriptMorphChannel::InternalGetShapes(ScriptMorphChannel* self)
	{
		Vector<TShared<MorphShape>> nativeArray__output;
		if(!self->IsNativeObjectValid())
			return {};

		nativeArray__output = static_cast<MorphChannel*>(self->GetNativeObject())->GetShapes();

		MonoArray* __output;
		int elementCount__output = (int)nativeArray__output.size();
		ScriptArray scriptArray__output = ScriptArray::Create<ScriptMorphShape>(elementCount__output);
		for(int elementIndex = 0; elementIndex < elementCount__output; elementIndex++)
		{
			TShared<MorphShape> arrayElementPointer__output = nativeArray__output[elementIndex];
			MonoObject* arrayElement__output;
			arrayElement__output = ScriptMorphShape::GetOrCreateScriptObject(arrayElementPointer__output);
			scriptArray__output.Set(elementIndex, arrayElement__output);
		}
		__output = scriptArray__output.GetInternal();

		return __output;
	}
}
