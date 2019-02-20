//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
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

	void ScriptPixelData::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_getRowPitch", (void*)&ScriptPixelData::Internal_getRowPitch);
		metaData.scriptClass->addInternalCall("Internal_getSlicePitch", (void*)&ScriptPixelData::Internal_getSlicePitch);
		metaData.scriptClass->addInternalCall("Internal_getFormat", (void*)&ScriptPixelData::Internal_getFormat);
		metaData.scriptClass->addInternalCall("Internal_getExtents", (void*)&ScriptPixelData::Internal_getExtents);
		metaData.scriptClass->addInternalCall("Internal_isConsecutive", (void*)&ScriptPixelData::Internal_isConsecutive);
		metaData.scriptClass->addInternalCall("Internal_getSize", (void*)&ScriptPixelData::Internal_getSize);
		metaData.scriptClass->addInternalCall("Internal_create", (void*)&ScriptPixelData::Internal_create);
		metaData.scriptClass->addInternalCall("Internal_create0", (void*)&ScriptPixelData::Internal_create0);
		metaData.scriptClass->addInternalCall("Internal_getPixel", (void*)&ScriptPixelData::Internal_getPixel);
		metaData.scriptClass->addInternalCall("Internal_setPixel", (void*)&ScriptPixelData::Internal_setPixel);
		metaData.scriptClass->addInternalCall("Internal_getPixels", (void*)&ScriptPixelData::Internal_getPixels);
		metaData.scriptClass->addInternalCall("Internal_setPixels", (void*)&ScriptPixelData::Internal_setPixels);
		metaData.scriptClass->addInternalCall("Internal_getRawPixels", (void*)&ScriptPixelData::Internal_getRawPixels);
		metaData.scriptClass->addInternalCall("Internal_setRawPixels", (void*)&ScriptPixelData::Internal_setRawPixels);

	}

	MonoObject* ScriptPixelData::create(const SPtr<PixelData>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.scriptClass->createInstance("bool", ctorParams);
		new (bs_alloc<ScriptPixelData>()) ScriptPixelData(managedInstance, value);
		return managedInstance;
	}
	uint32_t ScriptPixelData::Internal_getRowPitch(ScriptPixelData* thisPtr)
	{
		uint32_t tmp__output;
		tmp__output = thisPtr->getInternal()->getRowPitch();

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	uint32_t ScriptPixelData::Internal_getSlicePitch(ScriptPixelData* thisPtr)
	{
		uint32_t tmp__output;
		tmp__output = thisPtr->getInternal()->getSlicePitch();

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	PixelFormat ScriptPixelData::Internal_getFormat(ScriptPixelData* thisPtr)
	{
		PixelFormat tmp__output;
		tmp__output = thisPtr->getInternal()->getFormat();

		PixelFormat __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptPixelData::Internal_getExtents(ScriptPixelData* thisPtr, PixelVolume* __output)
	{
		PixelVolume tmp__output;
		tmp__output = thisPtr->getInternal()->getExtents();

		*__output = tmp__output;
	}

	bool ScriptPixelData::Internal_isConsecutive(ScriptPixelData* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->getInternal()->isConsecutive();

		bool __output;
		__output = tmp__output;

		return __output;
	}

	uint32_t ScriptPixelData::Internal_getSize(ScriptPixelData* thisPtr)
	{
		uint32_t tmp__output;
		tmp__output = thisPtr->getInternal()->getSize();

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptPixelData::Internal_create(MonoObject* managedInstance, PixelVolume* volume, PixelFormat format)
	{
		SPtr<PixelData> instance = PixelDataEx::create(*volume, format);
		new (bs_alloc<ScriptPixelData>())ScriptPixelData(managedInstance, instance);
	}

	void ScriptPixelData::Internal_create0(MonoObject* managedInstance, uint32_t width, uint32_t height, uint32_t depth, PixelFormat pixelFormat)
	{
		SPtr<PixelData> instance = PixelDataEx::create(width, height, depth, pixelFormat);
		new (bs_alloc<ScriptPixelData>())ScriptPixelData(managedInstance, instance);
	}

	void ScriptPixelData::Internal_getPixel(ScriptPixelData* thisPtr, int32_t x, int32_t y, int32_t z, Color* __output)
	{
		Color tmp__output;
		tmp__output = PixelDataEx::getPixel(thisPtr->getInternal(), x, y, z);

		*__output = tmp__output;
	}

	void ScriptPixelData::Internal_setPixel(ScriptPixelData* thisPtr, Color* value, int32_t x, int32_t y, int32_t z)
	{
		PixelDataEx::setPixel(thisPtr->getInternal(), *value, x, y, z);
	}

	MonoArray* ScriptPixelData::Internal_getPixels(ScriptPixelData* thisPtr)
	{
		Vector<Color> vec__output;
		vec__output = PixelDataEx::getPixels(thisPtr->getInternal());

		MonoArray* __output;
		int arraySize__output = (int)vec__output.size();
		ScriptArray array__output = ScriptArray::create<ScriptColor>(arraySize__output);
		for(int i = 0; i < arraySize__output; i++)
		{
			array__output.set(i, vec__output[i]);
		}
		__output = array__output.getInternal();

		return __output;
	}

	void ScriptPixelData::Internal_setPixels(ScriptPixelData* thisPtr, MonoArray* value)
	{
		Vector<Color> vecvalue;
		if(value != nullptr)
		{
			ScriptArray arrayvalue(value);
			vecvalue.resize(arrayvalue.size());
			for(int i = 0; i < (int)arrayvalue.size(); i++)
			{
				vecvalue[i] = arrayvalue.get<Color>(i);
			}
		}
		PixelDataEx::setPixels(thisPtr->getInternal(), vecvalue);
	}

	MonoArray* ScriptPixelData::Internal_getRawPixels(ScriptPixelData* thisPtr)
	{
		Vector<char> vec__output;
		vec__output = PixelDataEx::getRawPixels(thisPtr->getInternal());

		MonoArray* __output;
		int arraySize__output = (int)vec__output.size();
		ScriptArray array__output = ScriptArray::create<char>(arraySize__output);
		for(int i = 0; i < arraySize__output; i++)
		{
			array__output.set(i, vec__output[i]);
		}
		__output = array__output.getInternal();

		return __output;
	}

	void ScriptPixelData::Internal_setRawPixels(ScriptPixelData* thisPtr, MonoArray* value)
	{
		Vector<char> vecvalue;
		if(value != nullptr)
		{
			ScriptArray arrayvalue(value);
			vecvalue.resize(arrayvalue.size());
			for(int i = 0; i < (int)arrayvalue.size(); i++)
			{
				vecvalue[i] = arrayvalue.get<char>(i);
			}
		}
		PixelDataEx::setRawPixels(thisPtr->getInternal(), vecvalue);
	}
}
