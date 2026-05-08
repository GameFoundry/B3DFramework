//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptPixelData.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "B3DScriptColor.generated.h"
#include "B3DScriptPixelVolume.generated.h"
#include "B3DScriptPixelData.generated.h"
#include "../Extensions/B3DPixelDataEx.h"

namespace b3d
{
	ScriptPixelData::ScriptPixelData(const TShared<PixelData>& nativeObject)
		:TScriptReflectableWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptPixelData::~ScriptPixelData()
	{
		UnregisterEvents();
	}

	void ScriptPixelData::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetRowPitch", (void*)&ScriptPixelData::InternalGetRowPitch);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetSlicePitch", (void*)&ScriptPixelData::InternalGetSlicePitch);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetFormat", (void*)&ScriptPixelData::InternalGetFormat);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetExtents", (void*)&ScriptPixelData::InternalGetExtents);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_IsConsecutive", (void*)&ScriptPixelData::InternalIsConsecutive);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetSize", (void*)&ScriptPixelData::InternalGetSize);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create", (void*)&ScriptPixelData::InternalCreate);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create0", (void*)&ScriptPixelData::InternalCreate0);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetPixel", (void*)&ScriptPixelData::InternalGetPixel);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetPixel", (void*)&ScriptPixelData::InternalSetPixel);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetPixels", (void*)&ScriptPixelData::InternalGetPixels);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetPixels", (void*)&ScriptPixelData::InternalSetPixels);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetRawPixels", (void*)&ScriptPixelData::InternalGetRawPixels);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetRawPixels", (void*)&ScriptPixelData::InternalSetRawPixels);

	}

	MonoObject* ScriptPixelData::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	uint32_t ScriptPixelData::InternalGetRowPitch(ScriptPixelData* self)
	{
		uint32_t tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<PixelData*>(self->GetNativeObject())->GetRowPitch();

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	uint32_t ScriptPixelData::InternalGetSlicePitch(ScriptPixelData* self)
	{
		uint32_t tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<PixelData*>(self->GetNativeObject())->GetSlicePitch();

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	PixelFormat ScriptPixelData::InternalGetFormat(ScriptPixelData* self)
	{
		PixelFormat tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<PixelData*>(self->GetNativeObject())->GetFormat();

		PixelFormat __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptPixelData::InternalGetExtents(ScriptPixelData* self, PixelVolume* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		PixelVolume tmp__output;
		tmp__output = static_cast<PixelData*>(self->GetNativeObject())->GetExtents();

		*__output = tmp__output;
	}

	bool ScriptPixelData::InternalIsConsecutive(ScriptPixelData* self)
	{
		bool tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<PixelData*>(self->GetNativeObject())->IsConsecutive();

		bool __output;
		__output = tmp__output;

		return __output;
	}

	uint32_t ScriptPixelData::InternalGetSize(ScriptPixelData* self)
	{
		uint32_t tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<PixelData*>(self->GetNativeObject())->GetSize();

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptPixelData::InternalCreate(MonoObject* scriptObject, PixelVolume* volume, PixelFormat format)
	{
		TShared<PixelData> nativeObject = PixelDataEx::Create(*volume, format);
		ScriptObjectWrapper::Create<ScriptPixelData>(nativeObject, scriptObject);
	}

	void ScriptPixelData::InternalCreate0(MonoObject* scriptObject, uint32_t width, uint32_t height, uint32_t depth, PixelFormat pixelFormat)
	{
		TShared<PixelData> nativeObject = PixelDataEx::Create(width, height, depth, pixelFormat);
		ScriptObjectWrapper::Create<ScriptPixelData>(nativeObject, scriptObject);
	}

	void ScriptPixelData::InternalGetPixel(ScriptPixelData* self, int32_t x, int32_t y, int32_t z, Color* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		Color tmp__output;
		tmp__output = PixelDataEx::GetPixel(std::static_pointer_cast<PixelData>(self->GetBaseNativeObjectAsShared()), x, y, z);

		*__output = tmp__output;
	}

	void ScriptPixelData::InternalSetPixel(ScriptPixelData* self, Color* value, int32_t x, int32_t y, int32_t z)
	{
		if(!self->IsNativeObjectValid())
			return;

		PixelDataEx::SetPixel(std::static_pointer_cast<PixelData>(self->GetBaseNativeObjectAsShared()), *value, x, y, z);
	}

	MonoArray* ScriptPixelData::InternalGetPixels(ScriptPixelData* self)
	{
		Vector<Color> nativeArray__output;
		if(!self->IsNativeObjectValid())
			return {};

		nativeArray__output = PixelDataEx::GetPixels(std::static_pointer_cast<PixelData>(self->GetBaseNativeObjectAsShared()));

		MonoArray* __output;
		int elementCount__output = (int)nativeArray__output.size();
		ScriptArray scriptArray__output = ScriptArray::Create<ScriptColor>(elementCount__output);
		for(int elementIndex = 0; elementIndex < elementCount__output; elementIndex++)
		{
			scriptArray__output.Set(elementIndex, nativeArray__output[elementIndex]);
		}
		__output = scriptArray__output.GetInternal();

		return __output;
	}

	void ScriptPixelData::InternalSetPixels(ScriptPixelData* self, MonoArray* value)
	{
		if(!self->IsNativeObjectValid())
			return;

		Vector<Color> nativeArrayvalue;
		if(value != nullptr)
		{
			ScriptArray scriptArrayvalue(value);
			nativeArrayvalue.resize(scriptArrayvalue.Size());
			for(int elementIndex = 0; elementIndex < (int)scriptArrayvalue.Size(); elementIndex++)
			{
				nativeArrayvalue[elementIndex] = scriptArrayvalue.Get<Color>(elementIndex);
			}
		}
		PixelDataEx::SetPixels(std::static_pointer_cast<PixelData>(self->GetBaseNativeObjectAsShared()), nativeArrayvalue);
	}

	MonoArray* ScriptPixelData::InternalGetRawPixels(ScriptPixelData* self)
	{
		Vector<char> nativeArray__output;
		if(!self->IsNativeObjectValid())
			return {};

		nativeArray__output = PixelDataEx::GetRawPixels(std::static_pointer_cast<PixelData>(self->GetBaseNativeObjectAsShared()));

		MonoArray* __output;
		int elementCount__output = (int)nativeArray__output.size();
		ScriptArray scriptArray__output = ScriptArray::Create<char>(elementCount__output);
		for(int elementIndex = 0; elementIndex < elementCount__output; elementIndex++)
		{
			scriptArray__output.Set(elementIndex, nativeArray__output[elementIndex]);
		}
		__output = scriptArray__output.GetInternal();

		return __output;
	}

	void ScriptPixelData::InternalSetRawPixels(ScriptPixelData* self, MonoArray* value)
	{
		if(!self->IsNativeObjectValid())
			return;

		Vector<char> nativeArrayvalue;
		if(value != nullptr)
		{
			ScriptArray scriptArrayvalue(value);
			nativeArrayvalue.resize(scriptArrayvalue.Size());
			for(int elementIndex = 0; elementIndex < (int)scriptArrayvalue.Size(); elementIndex++)
			{
				nativeArrayvalue[elementIndex] = scriptArrayvalue.Get<char>(elementIndex);
			}
		}
		PixelDataEx::SetRawPixels(std::static_pointer_cast<PixelData>(self->GetBaseNativeObjectAsShared()), nativeArrayvalue);
	}
}
