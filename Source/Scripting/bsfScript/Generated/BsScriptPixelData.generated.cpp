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
	uint32_t ScriptPixelData::InternalGetRowPitch(ScriptPixelData* thisPtr)
	{
		uint32_t tmp__output;
		tmp__output = thisPtr->GetInternal()->GetRowPitch();

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	uint32_t ScriptPixelData::InternalGetSlicePitch(ScriptPixelData* thisPtr)
	{
		uint32_t tmp__output;
		tmp__output = thisPtr->GetInternal()->GetSlicePitch();

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	PixelFormat ScriptPixelData::InternalGetFormat(ScriptPixelData* thisPtr)
	{
		PixelFormat tmp__output;
		tmp__output = thisPtr->GetInternal()->GetFormat();

		PixelFormat __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptPixelData::InternalGetExtents(ScriptPixelData* thisPtr, PixelVolume* __output)
	{
		PixelVolume tmp__output;
		tmp__output = thisPtr->GetInternal()->GetExtents();

		*__output = tmp__output;
	}

	bool ScriptPixelData::InternalIsConsecutive(ScriptPixelData* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetInternal()->IsConsecutive();

		bool __output;
		__output = tmp__output;

		return __output;
	}

	uint32_t ScriptPixelData::InternalGetSize(ScriptPixelData* thisPtr)
	{
		uint32_t tmp__output;
		tmp__output = thisPtr->GetInternal()->GetSize();

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptPixelData::InternalCreate(MonoObject* managedInstance, PixelVolume* volume, PixelFormat format)
	{
		SPtr<PixelData> instance = PixelDataEx::Create(*volume, format);
		new (B3DAllocate<ScriptPixelData>())ScriptPixelData(managedInstance, instance);
	}

	void ScriptPixelData::InternalCreate0(MonoObject* managedInstance, uint32_t width, uint32_t height, uint32_t depth, PixelFormat pixelFormat)
	{
		SPtr<PixelData> instance = PixelDataEx::Create(width, height, depth, pixelFormat);
		new (B3DAllocate<ScriptPixelData>())ScriptPixelData(managedInstance, instance);
	}

	void ScriptPixelData::InternalGetPixel(ScriptPixelData* thisPtr, int32_t x, int32_t y, int32_t z, Color* __output)
	{
		Color tmp__output;
		tmp__output = PixelDataEx::GetPixel(thisPtr->GetInternal(), x, y, z);

		*__output = tmp__output;
	}

	void ScriptPixelData::InternalSetPixel(ScriptPixelData* thisPtr, Color* value, int32_t x, int32_t y, int32_t z)
	{
		PixelDataEx::SetPixel(thisPtr->GetInternal(), *value, x, y, z);
	}

	MonoArray* ScriptPixelData::InternalGetPixels(ScriptPixelData* thisPtr)
	{
		Vector<Color> nativeArray__output;
		nativeArray__output = PixelDataEx::GetPixels(thisPtr->GetInternal());

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

	void ScriptPixelData::InternalSetPixels(ScriptPixelData* thisPtr, MonoArray* value)
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
		PixelDataEx::SetPixels(thisPtr->GetInternal(), nativeArrayvalue);
	}

	MonoArray* ScriptPixelData::InternalGetRawPixels(ScriptPixelData* thisPtr)
	{
		Vector<char> nativeArray__output;
		nativeArray__output = PixelDataEx::GetRawPixels(thisPtr->GetInternal());

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

	void ScriptPixelData::InternalSetRawPixels(ScriptPixelData* thisPtr, MonoArray* value)
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
		PixelDataEx::SetRawPixels(thisPtr->GetInternal(), nativeArrayvalue);
	}
}
