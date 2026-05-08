//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptFont.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "../../../Engine/Core/Text/B3DFont.h"
#include "B3DScriptFontBitmapInformation.generated.h"

namespace b3d
{
	ScriptFont::ScriptFont(const TResourceHandle<Font>& nativeObject)
		:TScriptResourceWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptFont::~ScriptFont()
	{
		UnregisterEvents();
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
		TShared<FontBitmapInformation> tmp__output;
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
