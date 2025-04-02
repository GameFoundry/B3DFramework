//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptFont.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Text/BsFont.h"
#include "BsScriptFontBitmapInformation.generated.h"

namespace bs
{
	ScriptFont::ScriptFont(const TResourceHandle<Font>& nativeObject)
		:TScriptResourceWrapper(nativeObject)
	{
		RegisterEvents();
	}

	void ScriptFont::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetRef", (void*)&ScriptFont::InternalGetRef);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetBitmap", (void*)&ScriptFont::InternalGetBitmap);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetClosestExistingBitmapSize", (void*)&ScriptFont::InternalGetClosestExistingBitmapSize);

	}

	MonoObject* ScriptFont::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	MonoObject* ScriptFont::InternalGetRef(ScriptFont* self)
	{
		return self->GetOrCreateResourceReference();
	}

	MonoObject* ScriptFont::InternalGetBitmap(ScriptFont* self, float size)
	{
		SPtr<FontBitmapInformation> tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<Font*>(self->GetNativeObject())->GetBitmap(size);

		MonoObject* __output;
		__output = ScriptFontBitmapInformation::GetOrCreateScriptObject(tmp__output);

		return __output;
	}

	float ScriptFont::InternalGetClosestExistingBitmapSize(ScriptFont* self, float size)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<Font*>(self->GetNativeObject())->GetClosestExistingBitmapSize(size);

		float __output;
		__output = tmp__output;

		return __output;
	}
}
