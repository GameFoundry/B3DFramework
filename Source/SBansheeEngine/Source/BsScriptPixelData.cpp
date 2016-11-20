//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScriptPixelData.h"
#include "BsScriptMeta.h"
#include "BsMonoField.h"
#include "BsMonoClass.h"
#include "BsMonoManager.h"
#include "BsMonoUtil.h"
#include "BsPixelUtil.h"
#include "BsScriptColor.h"

namespace bs
{
	ScriptPixelData::ScriptPixelData(MonoObject* managedInstance)
		:ScriptObject(managedInstance)
	{

	}

	ScriptPixelData::~ScriptPixelData()
	{

	}

	void ScriptPixelData::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_CreateInstance", &ScriptPixelData::internal_createInstance);
		metaData.scriptClass->addInternalCall("Internal_GetPixel", &ScriptPixelData::internal_getPixel);
		metaData.scriptClass->addInternalCall("Internal_SetPixel", &ScriptPixelData::internal_setPixel);
		metaData.scriptClass->addInternalCall("Internal_GetPixels", &ScriptPixelData::internal_getPixels);
		metaData.scriptClass->addInternalCall("Internal_SetPixels", &ScriptPixelData::internal_setPixels);
		metaData.scriptClass->addInternalCall("Internal_GetRawPixels", &ScriptPixelData::internal_getRawPixels);
		metaData.scriptClass->addInternalCall("Internal_SetRawPixels", &ScriptPixelData::internal_setRawPixels);
		metaData.scriptClass->addInternalCall("Internal_GetExtents", &ScriptPixelData::internal_getExtents);
		metaData.scriptClass->addInternalCall("Internal_GetFormat", &ScriptPixelData::internal_getFormat);
		metaData.scriptClass->addInternalCall("Internal_GetRowPitch", &ScriptPixelData::internal_getRowPitch);
		metaData.scriptClass->addInternalCall("Internal_GetSlicePitch", &ScriptPixelData::internal_getSlicePitch);
		metaData.scriptClass->addInternalCall("Internal_GetSize", &ScriptPixelData::internal_getSize);
		metaData.scriptClass->addInternalCall("Internal_GetIsConsecutive", &ScriptPixelData::internal_getIsConsecutive);
	}

	void ScriptPixelData::initialize(const SPtr<PixelData>& pixelData)
	{
		mPixelData = pixelData;
	}

	MonoObject* ScriptPixelData::create(const SPtr<PixelData>& pixelData)
	{
		MonoObject* pixelDataObj = metaData.scriptClass->createInstance();

		ScriptPixelData* scriptPixelData = ScriptPixelData::toNative(pixelDataObj);
		scriptPixelData->initialize(pixelData);

		return pixelDataObj;
	}

	void ScriptPixelData::internal_createInstance(MonoObject* instance, PixelVolume* volume, PixelFormat format)
	{
		SPtr<PixelData> pixelData = bs_shared_ptr_new<PixelData>(*volume, format);
		pixelData->allocateInternalBuffer();

		ScriptPixelData* scriptPixelData = new (bs_alloc<ScriptPixelData>()) ScriptPixelData(instance);
		scriptPixelData->initialize(pixelData);
	}

	void ScriptPixelData::internal_getPixel(ScriptPixelData* thisPtr, int x, int y, int z, Color* value)
	{
		if (!checkIsLocked(thisPtr))
			*value = thisPtr->mPixelData->getColorAt(x, y, z);
		else
			*value = Color();
	}

	void ScriptPixelData::internal_setPixel(ScriptPixelData* thisPtr, int x, int y, int z, Color* value)
	{
		if (!checkIsLocked(thisPtr))
			thisPtr->mPixelData->setColorAt(*value, x, y, z);
	}

	void ScriptPixelData::internal_getPixels(ScriptPixelData* thisPtr, MonoArray** value)
	{
		if (!checkIsLocked(thisPtr))
			return;

		SPtr<PixelData> pixelData = thisPtr->mPixelData;
		PixelVolume pixelVolume = pixelData->getExtents();
		UINT32 depth = pixelVolume.getDepth();
		UINT32 height = pixelVolume.getHeight();
		UINT32 width = pixelVolume.getWidth();

		::MonoClass* colorClass = ScriptColor::getMetaData()->scriptClass->_getInternalClass();
		UINT32 totalNumElements = width * height * depth;

		ScriptArray scriptArray(colorClass, totalNumElements);

		PixelFormat format = pixelData->getFormat();
		UINT32 pixelSize = PixelUtil::getNumElemBytes(format);
		UINT8* data = pixelData->getData();

		UINT32 rowPitch = pixelData->getRowPitch();
		UINT32 slicePitch = pixelData->getSlicePitch();

		// Note: Can I copy bytes more directly?
		for (UINT32 z = 0; z < depth; z++)
		{
			UINT32 zArrayIdx = z * width * height;
			UINT32 zDataIdx = z * slicePitch * pixelSize;

			for (UINT32 y = 0; y < height; y++)
			{
				UINT32 yArrayIdx = y * width;
				UINT32 yDataIdx = y * rowPitch * pixelSize;

				for (UINT32 x = 0; x < width; x++)
				{
					UINT32 arrayIdx = x + yArrayIdx + zArrayIdx;
					UINT32 dataIdx = x * pixelSize + yDataIdx + zDataIdx;

					UINT8* dest = data + dataIdx;
					scriptArray.set(arrayIdx, *(Color*)dest);
				}
			}
		}

		*value = scriptArray.getInternal();

	}

	void ScriptPixelData::internal_setPixels(ScriptPixelData* thisPtr, MonoArray* value)
	{
		if (!checkIsLocked(thisPtr))
			return;

		SPtr<PixelData> pixelData = thisPtr->mPixelData;
		PixelVolume pixelVolume = pixelData->getExtents();
		UINT32 depth = pixelVolume.getDepth();
		UINT32 height = pixelVolume.getHeight();
		UINT32 width = pixelVolume.getWidth();

		ScriptArray scriptArray(value);

		UINT32 arrayLen = scriptArray.size();
		UINT32 totalNumElements = width * height * depth;
		if (arrayLen != totalNumElements)
		{
			LOGERR("Unable to set colors, invalid array size.")
			return;
		}

		PixelFormat format = pixelData->getFormat();
		UINT32 pixelSize = PixelUtil::getNumElemBytes(format);
		UINT8* data = pixelData->getData();

		UINT32 rowPitch = pixelData->getRowPitch();
		UINT32 slicePitch = pixelData->getSlicePitch();

		for (UINT32 z = 0; z < depth; z++)
		{
			UINT32 zArrayIdx = z * width * height;
			UINT32 zDataIdx = z * slicePitch * pixelSize;

			for (UINT32 y = 0; y < height; y++)
			{
				UINT32 yArrayIdx = y * width;
				UINT32 yDataIdx = y * rowPitch * pixelSize;

				for (UINT32 x = 0; x < width; x++)
				{
					UINT32 arrayIdx = x + yArrayIdx + zArrayIdx;
					UINT32 dataIdx = x * pixelSize + yDataIdx + zDataIdx;

					UINT8* dest = data + dataIdx;

					Color color = scriptArray.get<Color>(arrayIdx);
					PixelUtil::packColor(color, format, dest);
				}
			}
		}
	}

	void ScriptPixelData::internal_getRawPixels(ScriptPixelData* thisPtr, MonoArray** value)
	{
		if (!checkIsLocked(thisPtr))
			return;

		SPtr<PixelData> pixelData = thisPtr->mPixelData;
		PixelVolume pixelVolume = pixelData->getExtents();
		UINT32 depth = pixelVolume.getDepth();
		UINT32 height = pixelVolume.getHeight();
		UINT32 width = pixelVolume.getWidth();

		ScriptArray scriptArray(MonoUtil::getByteClass(), pixelData->getSize());

		PixelFormat format = pixelData->getFormat();
		UINT32 pixelSize = PixelUtil::getNumElemBytes(format);
		UINT8* data = pixelData->getData();

		UINT32 rowPitch = pixelData->getRowPitch();
		UINT32 slicePitch = pixelData->getSlicePitch();

		// Note: Can I copy bytes more directly?
		for (UINT32 z = 0; z < depth; z++)
		{
			UINT32 zArrayIdx = z * width * height;
			UINT32 zDataIdx = z * slicePitch * pixelSize;

			for (UINT32 y = 0; y < height; y++)
			{
				UINT32 yArrayIdx = y * width;
				UINT32 yDataIdx = y * rowPitch * pixelSize;

				for (UINT32 x = 0; x < width; x++)
				{
					UINT32 arrayIdx = x + yArrayIdx + zArrayIdx;
					UINT32 dataIdx = x * pixelSize + yDataIdx + zDataIdx;

					UINT8* dest = data + dataIdx;
					scriptArray.set(arrayIdx, *dest);
				}
			}
		}

		*value = scriptArray.getInternal();
	}

	void ScriptPixelData::internal_setRawPixels(ScriptPixelData* thisPtr, MonoArray* value)
	{
		if (!checkIsLocked(thisPtr))
			return;

		SPtr<PixelData> pixelData = thisPtr->mPixelData;
		PixelVolume pixelVolume = pixelData->getExtents();
		UINT32 depth = pixelVolume.getDepth();
		UINT32 height = pixelVolume.getHeight();
		UINT32 width = pixelVolume.getWidth();

		ScriptArray scriptArray(value);
		UINT32 arrayLen = scriptArray.size();
		if (pixelData->getSize() != arrayLen)
		{
			LOGERR("Unable to set colors, invalid array size.")
			return;
		}

		PixelFormat format = pixelData->getFormat();
		UINT32 pixelSize = PixelUtil::getNumElemBytes(format);
		UINT8* data = pixelData->getData();

		UINT32 rowPitch = pixelData->getRowPitch();
		UINT32 slicePitch = pixelData->getSlicePitch();

		// Note: Can I copy bytes more directly?
		for (UINT32 z = 0; z < depth; z++)
		{
			UINT32 zArrayIdx = z * width * height;
			UINT32 zDataIdx = z * slicePitch * pixelSize;

			for (UINT32 y = 0; y < height; y++)
			{
				UINT32 yArrayIdx = y * width;
				UINT32 yDataIdx = y * rowPitch * pixelSize;

				for (UINT32 x = 0; x < width; x++)
				{
					UINT32 arrayIdx = x + yArrayIdx + zArrayIdx;
					UINT32 dataIdx = x * pixelSize + yDataIdx + zDataIdx;

					UINT8* dest = data + dataIdx;
					*dest = scriptArray.get<char>(arrayIdx);
				}
			}
		}
	}

	void ScriptPixelData::internal_getExtents(ScriptPixelData* thisPtr, PixelVolume* value)
	{
		*value = thisPtr->mPixelData->getExtents();
	}

	void ScriptPixelData::internal_getFormat(ScriptPixelData* thisPtr, PixelFormat* value)
	{
		*value = thisPtr->mPixelData->getFormat();
	}

	void ScriptPixelData::internal_getRowPitch(ScriptPixelData* thisPtr, int* value)
	{
		*value = thisPtr->mPixelData->getRowPitch();
	}

	void ScriptPixelData::internal_getSlicePitch(ScriptPixelData* thisPtr, int* value)
	{
		*value = thisPtr->mPixelData->getSlicePitch();
	}

	void ScriptPixelData::internal_getSize(ScriptPixelData* thisPtr, int* value)
	{
		*value = thisPtr->mPixelData->getSize();
	}

	void ScriptPixelData::internal_getIsConsecutive(ScriptPixelData* thisPtr, bool* value)
	{
		*value = thisPtr->mPixelData->isConsecutive();
	}

	bool ScriptPixelData::checkIsLocked(ScriptPixelData* thisPtr)
	{
		if (thisPtr->mPixelData->isLocked())
		{
			LOGWRN("Attempting to access a locked pixel data buffer.");
			return true;
		}

		return false;
	}
}