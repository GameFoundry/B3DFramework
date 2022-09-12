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

	void ScriptFontBitmap::InitRuntimeData()
	{
		metaData.scriptClass->AddInternalCall("Internal_getCharDesc", (void*)&ScriptFontBitmap::Internal_getCharDesc);
		metaData.scriptClass->AddInternalCall("Internal_getsize", (void*)&ScriptFontBitmap::Internal_getsize);
		metaData.scriptClass->AddInternalCall("Internal_setsize", (void*)&ScriptFontBitmap::Internal_setsize);
		metaData.scriptClass->AddInternalCall("Internal_getbaselineOffset", (void*)&ScriptFontBitmap::Internal_getbaselineOffset);
		metaData.scriptClass->AddInternalCall("Internal_setbaselineOffset", (void*)&ScriptFontBitmap::Internal_setbaselineOffset);
		metaData.scriptClass->AddInternalCall("Internal_getlineHeight", (void*)&ScriptFontBitmap::Internal_getlineHeight);
		metaData.scriptClass->AddInternalCall("Internal_setlineHeight", (void*)&ScriptFontBitmap::Internal_setlineHeight);
		metaData.scriptClass->AddInternalCall("Internal_getmissingGlyph", (void*)&ScriptFontBitmap::Internal_getmissingGlyph);
		metaData.scriptClass->AddInternalCall("Internal_setmissingGlyph", (void*)&ScriptFontBitmap::Internal_setmissingGlyph);
		metaData.scriptClass->AddInternalCall("Internal_getspaceWidth", (void*)&ScriptFontBitmap::Internal_getspaceWidth);
		metaData.scriptClass->AddInternalCall("Internal_setspaceWidth", (void*)&ScriptFontBitmap::Internal_setspaceWidth);
		metaData.scriptClass->AddInternalCall("Internal_gettexturePages", (void*)&ScriptFontBitmap::Internal_gettexturePages);
		metaData.scriptClass->AddInternalCall("Internal_settexturePages", (void*)&ScriptFontBitmap::Internal_settexturePages);

	}

	MonoObject* ScriptFontBitmap::create(const SPtr<FontBitmap>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.scriptClass->CreateInstance("bool", ctorParams);
		new (bs_alloc<ScriptFontBitmap>()) ScriptFontBitmap(managedInstance, value);
		return managedInstance;
	}
	void ScriptFontBitmap::Internal_getCharDesc(ScriptFontBitmap* thisPtr, uint32_t charId, __CharDescInterop* __output)
	{
		CharDesc tmp__output;
		tmp__output = thisPtr->GetInternal()->getCharDesc(charId);

		__CharDescInterop interop__output;
		interop__output = ScriptCharDesc::toInterop(tmp__output);
		MonoUtil::valueCopy(__output, &interop__output, ScriptCharDesc::getMetaData()->scriptClass->_getInternalClass());
	}

	uint32_t ScriptFontBitmap::Internal_getsize(ScriptFontBitmap* thisPtr)
	{
		uint32_t tmp__output;
		tmp__output = thisPtr->GetInternal()->size;

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptFontBitmap::Internal_setsize(ScriptFontBitmap* thisPtr, uint32_t value)
	{
		thisPtr->GetInternal()->size = value;
	}

	int32_t ScriptFontBitmap::Internal_getbaselineOffset(ScriptFontBitmap* thisPtr)
	{
		int32_t tmp__output;
		tmp__output = thisPtr->GetInternal()->baselineOffset;

		int32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptFontBitmap::Internal_setbaselineOffset(ScriptFontBitmap* thisPtr, int32_t value)
	{
		thisPtr->GetInternal()->baselineOffset = value;
	}

	uint32_t ScriptFontBitmap::Internal_getlineHeight(ScriptFontBitmap* thisPtr)
	{
		uint32_t tmp__output;
		tmp__output = thisPtr->GetInternal()->lineHeight;

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptFontBitmap::Internal_setlineHeight(ScriptFontBitmap* thisPtr, uint32_t value)
	{
		thisPtr->GetInternal()->lineHeight = value;
	}

	void ScriptFontBitmap::Internal_getmissingGlyph(ScriptFontBitmap* thisPtr, __CharDescInterop* __output)
	{
		CharDesc tmp__output;
		tmp__output = thisPtr->GetInternal()->missingGlyph;

		__CharDescInterop interop__output;
		interop__output = ScriptCharDesc::toInterop(tmp__output);
		MonoUtil::valueCopy(__output, &interop__output, ScriptCharDesc::getMetaData()->scriptClass->_getInternalClass());


	}

	void ScriptFontBitmap::Internal_setmissingGlyph(ScriptFontBitmap* thisPtr, __CharDescInterop* value)
	{
		CharDesc tmpvalue;
		tmpvalue = ScriptCharDesc::fromInterop(*value);
		thisPtr->GetInternal()->missingGlyph = tmpvalue;
	}

	uint32_t ScriptFontBitmap::Internal_getspaceWidth(ScriptFontBitmap* thisPtr)
	{
		uint32_t tmp__output;
		tmp__output = thisPtr->GetInternal()->spaceWidth;

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptFontBitmap::Internal_setspaceWidth(ScriptFontBitmap* thisPtr, uint32_t value)
	{
		thisPtr->GetInternal()->spaceWidth = value;
	}

	MonoArray* ScriptFontBitmap::Internal_gettexturePages(ScriptFontBitmap* thisPtr)
	{
		Vector<ResourceHandle<Texture>> vec__output;
		vec__output = thisPtr->GetInternal()->texturePages;

		MonoArray* __output;
		int arraySize__output = (int)vec__output.Size();
		ScriptArray array__output = ScriptArray::create<ScriptRRefBase>(arraySize__output);
		for(int i = 0; i < arraySize__output; i++)
		{
			ScriptRRefBase* script__output;
			script__output = ScriptResourceManager::instance().GetScriptRRef(vec__output[i]);
			if(script__output != nullptr)
				array__output.Set(i, script__output->GetManagedInstance());
			else
				array__output.Set(i, nullptr);
		}
		__output = array__output.GetInternal();

		return __output;
	}

	void ScriptFontBitmap::Internal_settexturePages(ScriptFontBitmap* thisPtr, MonoArray* value)
	{
		Vector<ResourceHandle<Texture>> vecvalue;
		if(value != nullptr)
		{
			ScriptArray Arrayvalue(value);
			vecvalue.Resize(arrayvalue.size());
			for(int i = 0; i < (int)arrayvalue.Size(); i++)
			{
				ScriptRRefBase* scriptvalue;
				scriptvalue = ScriptRRefBase::toNative(arrayvalue.get<MonoObject*>(i));
				if(scriptvalue != nullptr)
				{
					ResourceHandle<Texture> arrayElemPtrvalue = static_resource_cast<Texture>(scriptvalue->GetHandle());
					vecvalue[i] = arrayElemPtrvalue;
				}
			}

		}
		thisPtr->GetInternal()->texturePages = vecvalue;
	}
}
