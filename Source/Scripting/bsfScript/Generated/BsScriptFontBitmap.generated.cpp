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
		: TScriptReflectable(managedInstance, value)
	{
	}

	void ScriptFontBitmap::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_GetCharDesc", (void*)&ScriptFontBitmap::InternalGetCharDesc);
		metaData.ScriptClass->AddInternalCall("Internal_GetSize", (void*)&ScriptFontBitmap::InternalGetSize);
		metaData.ScriptClass->AddInternalCall("Internal_SetSize", (void*)&ScriptFontBitmap::InternalSetSize);
		metaData.ScriptClass->AddInternalCall("Internal_GetBaselineOffset", (void*)&ScriptFontBitmap::InternalGetBaselineOffset);
		metaData.ScriptClass->AddInternalCall("Internal_SetBaselineOffset", (void*)&ScriptFontBitmap::InternalSetBaselineOffset);
		metaData.ScriptClass->AddInternalCall("Internal_GetLineHeight", (void*)&ScriptFontBitmap::InternalGetLineHeight);
		metaData.ScriptClass->AddInternalCall("Internal_SetLineHeight", (void*)&ScriptFontBitmap::InternalSetLineHeight);
		metaData.ScriptClass->AddInternalCall("Internal_GetMissingGlyph", (void*)&ScriptFontBitmap::InternalGetMissingGlyph);
		metaData.ScriptClass->AddInternalCall("Internal_SetMissingGlyph", (void*)&ScriptFontBitmap::InternalSetMissingGlyph);
		metaData.ScriptClass->AddInternalCall("Internal_GetSpaceWidth", (void*)&ScriptFontBitmap::InternalGetSpaceWidth);
		metaData.ScriptClass->AddInternalCall("Internal_SetSpaceWidth", (void*)&ScriptFontBitmap::InternalSetSpaceWidth);
		metaData.ScriptClass->AddInternalCall("Internal_GetTexturePages", (void*)&ScriptFontBitmap::InternalGetTexturePages);
		metaData.ScriptClass->AddInternalCall("Internal_SetTexturePages", (void*)&ScriptFontBitmap::InternalSetTexturePages);
	}

	MonoObject* ScriptFontBitmap::Create(const SPtr<FontBitmap>& value)
	{
		if(value == nullptr) return nullptr;

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.ScriptClass->CreateInstance("bool", ctorParams);
		new(bs_alloc<ScriptFontBitmap>()) ScriptFontBitmap(managedInstance, value);
		return managedInstance;
	}

	void ScriptFontBitmap::InternalGetCharDesc(ScriptFontBitmap* thisPtr, uint32_t charId, __CharDescInterop* __output)
	{
		CharDesc tmp__output;
		tmp__output = thisPtr->GetInternal()->GetCharDesc(charId);

		__CharDescInterop interop__output;
		interop__output = ScriptCharDesc::ToInterop(tmp__output);
		MonoUtil::ValueCopy(__output, &interop__output, ScriptCharDesc::GetMetaData()->ScriptClass->GetInternalClassInternal());
	}

	uint32_t ScriptFontBitmap::InternalGetSize(ScriptFontBitmap* thisPtr)
	{
		uint32_t tmp__output;
		tmp__output = thisPtr->GetInternal()->Size;

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptFontBitmap::InternalSetSize(ScriptFontBitmap* thisPtr, uint32_t value)
	{
		thisPtr->GetInternal()->Size = value;
	}

	int32_t ScriptFontBitmap::InternalGetBaselineOffset(ScriptFontBitmap* thisPtr)
	{
		int32_t tmp__output;
		tmp__output = thisPtr->GetInternal()->BaselineOffset;

		int32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptFontBitmap::InternalSetBaselineOffset(ScriptFontBitmap* thisPtr, int32_t value)
	{
		thisPtr->GetInternal()->BaselineOffset = value;
	}

	uint32_t ScriptFontBitmap::InternalGetLineHeight(ScriptFontBitmap* thisPtr)
	{
		uint32_t tmp__output;
		tmp__output = thisPtr->GetInternal()->LineHeight;

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptFontBitmap::InternalSetLineHeight(ScriptFontBitmap* thisPtr, uint32_t value)
	{
		thisPtr->GetInternal()->LineHeight = value;
	}

	void ScriptFontBitmap::InternalGetMissingGlyph(ScriptFontBitmap* thisPtr, __CharDescInterop* __output)
	{
		CharDesc tmp__output;
		tmp__output = thisPtr->GetInternal()->MissingGlyph;

		__CharDescInterop interop__output;
		interop__output = ScriptCharDesc::ToInterop(tmp__output);
		MonoUtil::ValueCopy(__output, &interop__output, ScriptCharDesc::GetMetaData()->ScriptClass->GetInternalClassInternal());
	}

	void ScriptFontBitmap::InternalSetMissingGlyph(ScriptFontBitmap* thisPtr, __CharDescInterop* value)
	{
		CharDesc tmpvalue;
		tmpvalue = ScriptCharDesc::FromInterop(*value);
		thisPtr->GetInternal()->MissingGlyph = tmpvalue;
	}

	uint32_t ScriptFontBitmap::InternalGetSpaceWidth(ScriptFontBitmap* thisPtr)
	{
		uint32_t tmp__output;
		tmp__output = thisPtr->GetInternal()->SpaceWidth;

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptFontBitmap::InternalSetSpaceWidth(ScriptFontBitmap* thisPtr, uint32_t value)
	{
		thisPtr->GetInternal()->SpaceWidth = value;
	}

	MonoArray* ScriptFontBitmap::InternalGetTexturePages(ScriptFontBitmap* thisPtr)
	{
		Vector<ResourceHandle<Texture>> vec__output;
		vec__output = thisPtr->GetInternal()->TexturePages;

		MonoArray* __output;
		int arraySize__output = (int)vec__output.size();
		ScriptArray array__output = ScriptArray::Create<ScriptRRefBase>(arraySize__output);
		for(int i = 0; i < arraySize__output; i++)
		{
			ScriptRRefBase* script__output;
			script__output = ScriptResourceManager::Instance().GetScriptRRef(vec__output[i]);
			if(script__output != nullptr)
				array__output.Set(i, script__output->GetManagedInstance());
			else
				array__output.Set(i, nullptr);
		}
		__output = array__output.GetInternal();

		return __output;
	}

	void ScriptFontBitmap::InternalSetTexturePages(ScriptFontBitmap* thisPtr, MonoArray* value)
	{
		Vector<ResourceHandle<Texture>> vecvalue;
		if(value != nullptr)
		{
			ScriptArray arrayvalue(value);
			vecvalue.resize(arrayvalue.Size());
			for(int i = 0; i < (int)arrayvalue.Size(); i++)
			{
				ScriptRRefBase* scriptvalue;
				scriptvalue = ScriptRRefBase::ToNative(arrayvalue.Get<MonoObject*>(i));
				if(scriptvalue != nullptr)
				{
					ResourceHandle<Texture> arrayElemPtrvalue = static_resource_cast<Texture>(scriptvalue->GetHandle());
					vecvalue[i] = arrayElemPtrvalue;
				}
			}
		}
		thisPtr->GetInternal()->TexturePages = vecvalue;
	}
} // namespace bs
