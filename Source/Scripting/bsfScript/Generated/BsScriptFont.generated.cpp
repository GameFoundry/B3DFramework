//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptFont.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Text/BsFont.h"
#include "BsScriptFontBitmap.generated.h"

namespace bs
{
	ScriptFont::ScriptFont(MonoObject* managedInstance, const ResourceHandle<Font>& value)
		:TScriptResource(managedInstance, value)
	{
	}

	void ScriptFont::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_GetRef", (void*)&ScriptFont::InternalGetRef);
		metaData.ScriptClass->AddInternalCall("Internal_GetBitmap", (void*)&ScriptFont::InternalGetBitmap);
		metaData.ScriptClass->AddInternalCall("Internal_GetClosestSize", (void*)&ScriptFont::InternalGetClosestSize);

	}

	 MonoObject*ScriptFont::CreateInstance()
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		return metaData.ScriptClass->CreateInstance("bool", ctorParams);
	}
	MonoObject* ScriptFont::InternalGetRef(ScriptFont* thisPtr)
	{
		return thisPtr->GetRRef();
	}

	MonoObject* ScriptFont::InternalGetBitmap(ScriptFont* thisPtr, uint32_t size)
	{
		SPtr<FontBitmap> tmp__output;
		tmp__output = thisPtr->GetHandle()->GetBitmap(size);

		MonoObject* __output;
		__output = ScriptFontBitmap::Create(tmp__output);

		return __output;
	}

	int32_t ScriptFont::InternalGetClosestSize(ScriptFont* thisPtr, uint32_t size)
	{
		int32_t tmp__output;
		tmp__output = thisPtr->GetHandle()->GetClosestSize(size);

		int32_t __output;
		__output = tmp__output;

		return __output;
	}
}
