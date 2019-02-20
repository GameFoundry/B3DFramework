//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptFontImportOptions.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "BsScriptCharRange.generated.h"
#include "BsScriptFontImportOptions.generated.h"

namespace bs
{
#if !BS_IS_BANSHEE3D
	ScriptFontImportOptions::ScriptFontImportOptions(MonoObject* managedInstance, const SPtr<FontImportOptions>& value)
		:TScriptReflectable(managedInstance, value)
	{
		mInternal = value;
	}

	void ScriptFontImportOptions::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_getfontSizes", (void*)&ScriptFontImportOptions::Internal_getfontSizes);
		metaData.scriptClass->addInternalCall("Internal_setfontSizes", (void*)&ScriptFontImportOptions::Internal_setfontSizes);
		metaData.scriptClass->addInternalCall("Internal_getcharIndexRanges", (void*)&ScriptFontImportOptions::Internal_getcharIndexRanges);
		metaData.scriptClass->addInternalCall("Internal_setcharIndexRanges", (void*)&ScriptFontImportOptions::Internal_setcharIndexRanges);
		metaData.scriptClass->addInternalCall("Internal_getdpi", (void*)&ScriptFontImportOptions::Internal_getdpi);
		metaData.scriptClass->addInternalCall("Internal_setdpi", (void*)&ScriptFontImportOptions::Internal_setdpi);
		metaData.scriptClass->addInternalCall("Internal_getrenderMode", (void*)&ScriptFontImportOptions::Internal_getrenderMode);
		metaData.scriptClass->addInternalCall("Internal_setrenderMode", (void*)&ScriptFontImportOptions::Internal_setrenderMode);
		metaData.scriptClass->addInternalCall("Internal_getbold", (void*)&ScriptFontImportOptions::Internal_getbold);
		metaData.scriptClass->addInternalCall("Internal_setbold", (void*)&ScriptFontImportOptions::Internal_setbold);
		metaData.scriptClass->addInternalCall("Internal_getitalic", (void*)&ScriptFontImportOptions::Internal_getitalic);
		metaData.scriptClass->addInternalCall("Internal_setitalic", (void*)&ScriptFontImportOptions::Internal_setitalic);
		metaData.scriptClass->addInternalCall("Internal_create", (void*)&ScriptFontImportOptions::Internal_create);

	}

	MonoObject* ScriptFontImportOptions::create(const SPtr<FontImportOptions>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.scriptClass->createInstance("bool", ctorParams);
		new (bs_alloc<ScriptFontImportOptions>()) ScriptFontImportOptions(managedInstance, value);
		return managedInstance;
	}
	void ScriptFontImportOptions::Internal_create(MonoObject* managedInstance)
	{
		SPtr<FontImportOptions> instance = FontImportOptions::create();
		new (bs_alloc<ScriptFontImportOptions>())ScriptFontImportOptions(managedInstance, instance);
	}
	MonoArray* ScriptFontImportOptions::Internal_getfontSizes(ScriptFontImportOptions* thisPtr)
	{
		Vector<uint32_t> vec__output;
		vec__output = thisPtr->getInternal()->fontSizes;

		MonoArray* __output;
		int arraySize__output = (int)vec__output.size();
		ScriptArray array__output = ScriptArray::create<uint32_t>(arraySize__output);
		for(int i = 0; i < arraySize__output; i++)
		{
			array__output.set(i, vec__output[i]);
		}
		__output = array__output.getInternal();

		return __output;
	}

	void ScriptFontImportOptions::Internal_setfontSizes(ScriptFontImportOptions* thisPtr, MonoArray* value)
	{
		Vector<uint32_t> vecvalue;
		if(value != nullptr)
		{
			ScriptArray arrayvalue(value);
			vecvalue.resize(arrayvalue.size());
			for(int i = 0; i < (int)arrayvalue.size(); i++)
			{
				vecvalue[i] = arrayvalue.get<uint32_t>(i);
			}

		}
		thisPtr->getInternal()->fontSizes = vecvalue;
	}

	MonoArray* ScriptFontImportOptions::Internal_getcharIndexRanges(ScriptFontImportOptions* thisPtr)
	{
		Vector<CharRange> vec__output;
		vec__output = thisPtr->getInternal()->charIndexRanges;

		MonoArray* __output;
		int arraySize__output = (int)vec__output.size();
		ScriptArray array__output = ScriptArray::create<ScriptCharRange>(arraySize__output);
		for(int i = 0; i < arraySize__output; i++)
		{
			array__output.set(i, vec__output[i]);
		}
		__output = array__output.getInternal();

		return __output;
	}

	void ScriptFontImportOptions::Internal_setcharIndexRanges(ScriptFontImportOptions* thisPtr, MonoArray* value)
	{
		Vector<CharRange> vecvalue;
		if(value != nullptr)
		{
			ScriptArray arrayvalue(value);
			vecvalue.resize(arrayvalue.size());
			for(int i = 0; i < (int)arrayvalue.size(); i++)
			{
				vecvalue[i] = arrayvalue.get<CharRange>(i);
			}

		}
		thisPtr->getInternal()->charIndexRanges = vecvalue;
	}

	uint32_t ScriptFontImportOptions::Internal_getdpi(ScriptFontImportOptions* thisPtr)
	{
		uint32_t tmp__output;
		tmp__output = thisPtr->getInternal()->dpi;

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptFontImportOptions::Internal_setdpi(ScriptFontImportOptions* thisPtr, uint32_t value)
	{
		thisPtr->getInternal()->dpi = value;
	}

	FontRenderMode ScriptFontImportOptions::Internal_getrenderMode(ScriptFontImportOptions* thisPtr)
	{
		FontRenderMode tmp__output;
		tmp__output = thisPtr->getInternal()->renderMode;

		FontRenderMode __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptFontImportOptions::Internal_setrenderMode(ScriptFontImportOptions* thisPtr, FontRenderMode value)
	{
		thisPtr->getInternal()->renderMode = value;
	}

	bool ScriptFontImportOptions::Internal_getbold(ScriptFontImportOptions* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->getInternal()->bold;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptFontImportOptions::Internal_setbold(ScriptFontImportOptions* thisPtr, bool value)
	{
		thisPtr->getInternal()->bold = value;
	}

	bool ScriptFontImportOptions::Internal_getitalic(ScriptFontImportOptions* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->getInternal()->italic;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptFontImportOptions::Internal_setitalic(ScriptFontImportOptions* thisPtr, bool value)
	{
		thisPtr->getInternal()->italic = value;
	}
#endif
}
