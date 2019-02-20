//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptFontBitmap.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "BsScriptResourceManager.h"
#include "Wrappers/BsScriptRRefBase.h"
#include "BsScriptCharDesc.generated.h"
#include "../../../Foundation/bsfCore/Image/BsTexture.h"

namespace bs
{
	ScriptFontBitmap::ScriptFontBitmap(MonoObject* managedInstance, const SPtr<FontBitmap>& value)
		:TScriptReflectable(managedInstance, value)
	{
	}

	void ScriptFontBitmap::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_getCharDesc", (void*)&ScriptFontBitmap::Internal_getCharDesc);
		metaData.scriptClass->addInternalCall("Internal_getsize", (void*)&ScriptFontBitmap::Internal_getsize);
		metaData.scriptClass->addInternalCall("Internal_setsize", (void*)&ScriptFontBitmap::Internal_setsize);
		metaData.scriptClass->addInternalCall("Internal_getbaselineOffset", (void*)&ScriptFontBitmap::Internal_getbaselineOffset);
		metaData.scriptClass->addInternalCall("Internal_setbaselineOffset", (void*)&ScriptFontBitmap::Internal_setbaselineOffset);
		metaData.scriptClass->addInternalCall("Internal_getlineHeight", (void*)&ScriptFontBitmap::Internal_getlineHeight);
		metaData.scriptClass->addInternalCall("Internal_setlineHeight", (void*)&ScriptFontBitmap::Internal_setlineHeight);
		metaData.scriptClass->addInternalCall("Internal_getmissingGlyph", (void*)&ScriptFontBitmap::Internal_getmissingGlyph);
		metaData.scriptClass->addInternalCall("Internal_setmissingGlyph", (void*)&ScriptFontBitmap::Internal_setmissingGlyph);
		metaData.scriptClass->addInternalCall("Internal_getspaceWidth", (void*)&ScriptFontBitmap::Internal_getspaceWidth);
		metaData.scriptClass->addInternalCall("Internal_setspaceWidth", (void*)&ScriptFontBitmap::Internal_setspaceWidth);
		metaData.scriptClass->addInternalCall("Internal_gettexturePages", (void*)&ScriptFontBitmap::Internal_gettexturePages);
		metaData.scriptClass->addInternalCall("Internal_settexturePages", (void*)&ScriptFontBitmap::Internal_settexturePages);

	}

	MonoObject* ScriptFontBitmap::create(const SPtr<FontBitmap>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.scriptClass->createInstance("bool", ctorParams);
		new (bs_alloc<ScriptFontBitmap>()) ScriptFontBitmap(managedInstance, value);
		return managedInstance;
	}
	void ScriptFontBitmap::Internal_getCharDesc(ScriptFontBitmap* thisPtr, uint32_t charId, __CharDescInterop* __output)
	{
		CharDesc tmp__output;
		tmp__output = thisPtr->getInternal()->getCharDesc(charId);

		__CharDescInterop interop__output;
		interop__output = ScriptCharDesc::toInterop(tmp__output);
		MonoUtil::valueCopy(__output, &interop__output, ScriptCharDesc::getMetaData()->scriptClass->_getInternalClass());
	}

	uint32_t ScriptFontBitmap::Internal_getsize(ScriptFontBitmap* thisPtr)
	{
		uint32_t tmp__output;
		tmp__output = thisPtr->getInternal()->size;

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptFontBitmap::Internal_setsize(ScriptFontBitmap* thisPtr, uint32_t value)
	{
		thisPtr->getInternal()->size = value;
	}

	int32_t ScriptFontBitmap::Internal_getbaselineOffset(ScriptFontBitmap* thisPtr)
	{
		int32_t tmp__output;
		tmp__output = thisPtr->getInternal()->baselineOffset;

		int32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptFontBitmap::Internal_setbaselineOffset(ScriptFontBitmap* thisPtr, int32_t value)
	{
		thisPtr->getInternal()->baselineOffset = value;
	}

	uint32_t ScriptFontBitmap::Internal_getlineHeight(ScriptFontBitmap* thisPtr)
	{
		uint32_t tmp__output;
		tmp__output = thisPtr->getInternal()->lineHeight;

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptFontBitmap::Internal_setlineHeight(ScriptFontBitmap* thisPtr, uint32_t value)
	{
		thisPtr->getInternal()->lineHeight = value;
	}

	void ScriptFontBitmap::Internal_getmissingGlyph(ScriptFontBitmap* thisPtr, __CharDescInterop* __output)
	{
		CharDesc tmp__output;
		tmp__output = thisPtr->getInternal()->missingGlyph;

		__CharDescInterop interop__output;
		interop__output = ScriptCharDesc::toInterop(tmp__output);
		MonoUtil::valueCopy(__output, &interop__output, ScriptCharDesc::getMetaData()->scriptClass->_getInternalClass());


	}

	void ScriptFontBitmap::Internal_setmissingGlyph(ScriptFontBitmap* thisPtr, __CharDescInterop* value)
	{
		CharDesc tmpvalue;
		tmpvalue = ScriptCharDesc::fromInterop(*value);
		thisPtr->getInternal()->missingGlyph = tmpvalue;
	}

	uint32_t ScriptFontBitmap::Internal_getspaceWidth(ScriptFontBitmap* thisPtr)
	{
		uint32_t tmp__output;
		tmp__output = thisPtr->getInternal()->spaceWidth;

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptFontBitmap::Internal_setspaceWidth(ScriptFontBitmap* thisPtr, uint32_t value)
	{
		thisPtr->getInternal()->spaceWidth = value;
	}

	MonoArray* ScriptFontBitmap::Internal_gettexturePages(ScriptFontBitmap* thisPtr)
	{
		Vector<ResourceHandle<Texture>> vec__output;
		vec__output = thisPtr->getInternal()->texturePages;

		MonoArray* __output;
		int arraySize__output = (int)vec__output.size();
		ScriptArray array__output = ScriptArray::create<ScriptRRefBase>(arraySize__output);
		for(int i = 0; i < arraySize__output; i++)
		{
			ScriptRRefBase* script__output;
			script__output = ScriptResourceManager::instance().getScriptRRef(vec__output[i]);
			if(script__output != nullptr)
				array__output.set(i, script__output->getManagedInstance());
			else
				array__output.set(i, nullptr);
		}
		__output = array__output.getInternal();

		return __output;
	}

	void ScriptFontBitmap::Internal_settexturePages(ScriptFontBitmap* thisPtr, MonoArray* value)
	{
		Vector<ResourceHandle<Texture>> vecvalue;
		if(value != nullptr)
		{
			ScriptArray arrayvalue(value);
			vecvalue.resize(arrayvalue.size());
			for(int i = 0; i < (int)arrayvalue.size(); i++)
			{
				ScriptRRefBase* scriptvalue;
				scriptvalue = ScriptRRefBase::toNative(arrayvalue.get<MonoObject*>(i));
				if(scriptvalue != nullptr)
				{
					ResourceHandle<Texture> arrayElemPtrvalue = static_resource_cast<Texture>(scriptvalue->getHandle());
					vecvalue[i] = arrayElemPtrvalue;
				}
			}

		}
		thisPtr->getInternal()->texturePages = vecvalue;
	}
}
