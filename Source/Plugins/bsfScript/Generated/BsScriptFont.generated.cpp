//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptFont.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../../../Foundation/bsfCore/Text/BsFont.h"
#include "BsScriptFontBitmap.generated.h"

namespace bs
{
	ScriptFont::ScriptFont(MonoObject* managedInstance, const ResourceHandle<Font>& value)
		:TScriptResource(managedInstance, value)
	{
	}

	void ScriptFont::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_GetRef", (void*)&ScriptFont::Internal_getRef);
		metaData.scriptClass->addInternalCall("Internal_getBitmap", (void*)&ScriptFont::Internal_getBitmap);
		metaData.scriptClass->addInternalCall("Internal_getClosestSize", (void*)&ScriptFont::Internal_getClosestSize);

	}

	 MonoObject*ScriptFont::createInstance()
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		return metaData.scriptClass->createInstance("bool", ctorParams);
	}
	MonoObject* ScriptFont::Internal_getRef(ScriptFont* thisPtr)
	{
		return thisPtr->getRRef();
	}

	MonoObject* ScriptFont::Internal_getBitmap(ScriptFont* thisPtr, uint32_t size)
	{
		SPtr<FontBitmap> tmp__output;
		tmp__output = thisPtr->getHandle()->getBitmap(size);

		MonoObject* __output;
		__output = ScriptFontBitmap::create(tmp__output);

		return __output;
	}

	int32_t ScriptFont::Internal_getClosestSize(ScriptFont* thisPtr, uint32_t size)
	{
		int32_t tmp__output;
		tmp__output = thisPtr->getHandle()->getClosestSize(size);

		int32_t __output;
		__output = tmp__output;

		return __output;
	}
}
