//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptPixelData.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "Wrappers/BsScriptColor.h"
#include "BsScriptPixelVolume.generated.h"
#include "BsScriptPixelData.generated.h"
#include "../Extensions/BsPixelDataEx.h"

namespace bs
{
	ScriptPixelData::ScriptPixelData(MonoObject* managedInstance, const SPtr<PixelData>& value)
		:TScriptReflectable(managedInstance, value)
	{
	}

	void ScriptPixelData::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_GetRowPitch", (void*)&ScriptPixelData::InternalGetRowPitch);
		metaData.ScriptClass->AddInternalCall("Internal_GetSlicePitch", (void*)&ScriptPixelData::InternalGetSlicePitch);
		metaData.ScriptClass->AddInternalCall("Internal_GetFormat", (void*)&ScriptPixelData::InternalGetFormat);
		metaData.ScriptClass->AddInternalCall("Internal_GetExtents", (void*)&ScriptPixelData::InternalGetExtents);
		metaData.ScriptClass->AddInternalCall("Internal_IsConsecutive", (void*)&ScriptPixelData::InternalIsConsecutive);
		metaData.ScriptClass->AddInternalCall("Internal_GetSize", (void*)&ScriptPixelData::InternalGetSize);
		metaData.ScriptClass->AddInternalCall("Internal_Create", (void*)&ScriptPixelData::InternalCreate);
		metaData.ScriptClass->AddInternalCall("Internal_Create0", (void*)&ScriptPixelData::InternalCreate0);
		metaData.ScriptClass->AddInternalCall("Internal_GetPixel", (void*)&ScriptPixelData::InternalGetPixel);
		metaData.ScriptClass->AddInternalCall("Internal_SetPixel", (void*)&ScriptPixelData::InternalSetPixel);
		metaData.ScriptClass->AddInternalCall("Internal_GetPixels", (void*)&ScriptPixelData::InternalGetPixels);
		metaData.ScriptClass->AddInternalCall("Internal_SetPixels", (void*)&ScriptPixelData::InternalSetPixels);
		metaData.ScriptClass->AddInternalCall("Internal_GetRawPixels", (void*)&ScriptPixelData::InternalGetRawPixels);
		metaData.ScriptClass->AddInternalCall("Internal_SetRawPixels", (void*)&ScriptPixelData::InternalSetRawPixels);

	}

	MonoObject* ScriptPixelData::Create(const SPtr<PixelData>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.ScriptClass->CreateInstance("bool", ctorParams);
		new (B3DAllocate<ScriptPixelData>()) ScriptPixelData(managedInstance, value);
		return managedInstance;
	}
	uint32_t ScriptPixelData::InternalGetRowPitch(ScriptPixelData* self)
	{
		uint32_t tmp__output;
		tmp__output = self->GetInternal()->GetRowPitch();

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	uint32_t ScriptPixelData::InternalGetSlicePitch(ScriptPixelData* self)
	{
		uint32_t tmp__output;
		tmp__output = self->GetInternal()->GetSlicePitch();

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	PixelFormat ScriptPixelData::InternalGetFormat(ScriptPixelData* self)
	{
		PixelFormat tmp__output;
		tmp__output = self->GetInternal()->GetFormat();

		PixelFormat __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptPixelData::InternalGetExtents(ScriptPixelData* self, PixelVolume* __output)
	{
		PixelVolume tmp__output;
		tmp__output = self->GetInternal()->GetExtents();

		*__output = tmp__output;
	}

	bool ScriptPixelData::InternalIsConsecutive(ScriptPixelData* self)
	{
		bool tmp__output;
		tmp__output = self->GetInternal()->IsConsecutive();

		bool __output;
		__output = tmp__output;

		return __output;
	}

	uint32_t ScriptPixelData::InternalGetSize(ScriptPixelData* self)
	{
		uint32_t tmp__output;
		tmp__output = self->GetInternal()->GetSize();

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptPixelData::InternalCreate(MonoObject* managedInstance, PixelVolume* volume, PixelFormat format)
	{
		SPtr<PixelData> nativeObject = PixelDataEx::Create(*volume, format);
		new (B3DAllocate<ScriptPixelData>())ScriptPixelData(managedInstance, nativeObject);
	}

	void ScriptPixelData::InternalCreate0(MonoObject* managedInstance, uint32_t width, uint32_t height, uint32_t depth, PixelFormat pixelFormat)
	{
		SPtr<PixelData> nativeObject = PixelDataEx::Create(width, height, depth, pixelFormat);
		new (B3DAllocate<ScriptPixelData>())ScriptPixelData(managedInstance, nativeObject);
	}

	void ScriptPixelData::InternalGetPixel(ScriptPixelData* self, int32_t x, int32_t y, int32_t z, Color* __output)
	{
		Color tmp__output;
		tmp__output = PixelDataEx::GetPixel(self->GetInternal(), x, y, z);

		*__output = tmp__output;
	}

	void ScriptPixelData::InternalSetPixel(ScriptPixelData* self, Color* value, int32_t x, int32_t y, int32_t z)
	{
		PixelDataEx::SetPixel(self->GetInternal(), *value, x, y, z);
	}

	MonoArray* ScriptPixelData::InternalGetPixels(ScriptPixelData* self)
	{
		Vector<Color> nativeArray__output;
		nativeArray__output = PixelDataEx::GetPixels(self->GetInternal());

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
		PixelDataEx::SetPixels(self->GetInternal(), nativeArrayvalue);
	}

	MonoArray* ScriptPixelData::InternalGetRawPixels(ScriptPixelData* self)
	{
		Vector<char> nativeArray__output;
		nativeArray__output = PixelDataEx::GetRawPixels(self->GetInternal());

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
		PixelDataEx::SetRawPixels(self->GetInternal(), nativeArrayvalue);
	}
}
