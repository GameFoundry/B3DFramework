//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScriptPixelUtility.h"
#include "BsMonoManager.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "BsDebug.h"
#include "BsScriptPixelData.h"

namespace bs
{
	ScriptPixelUtility::ScriptPixelUtility(MonoObject* instance)
		:ScriptObject(instance)
	{ }

	void ScriptPixelUtility::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_GetMemorySize", &ScriptPixelUtility::internal_getMemorySize);
		metaData.scriptClass->addInternalCall("Internal_HasAlpha", &ScriptPixelUtility::internal_hasAlpha);
		metaData.scriptClass->addInternalCall("Internal_IsFloatingPoint", &ScriptPixelUtility::internal_isFloatingPoint);
		metaData.scriptClass->addInternalCall("Internal_IsCompressed", &ScriptPixelUtility::internal_isCompressed);
		metaData.scriptClass->addInternalCall("Internal_IsDepth", &ScriptPixelUtility::internal_isDepth);
		metaData.scriptClass->addInternalCall("Internal_GetMaxMipmaps", &ScriptPixelUtility::internal_getMaxMipmaps);
		metaData.scriptClass->addInternalCall("Internal_ConvertFormat", &ScriptPixelUtility::internal_convertFormat);
		metaData.scriptClass->addInternalCall("Internal_Compress", &ScriptPixelUtility::internal_compress);
		metaData.scriptClass->addInternalCall("Internal_GenerateMipmaps", &ScriptPixelUtility::internal_generateMipmaps);
		metaData.scriptClass->addInternalCall("Internal_Scale", &ScriptPixelUtility::internal_scale);
		metaData.scriptClass->addInternalCall("Internal_ApplyGamma", &ScriptPixelUtility::internal_applyGamma);
	}

	void ScriptPixelUtility::internal_getMemorySize(UINT32 width, UINT32 height, UINT32 depth, PixelFormat format, UINT32* value)
	{
		*value = PixelUtil::getMemorySize(width, height, depth, format);
	}

	void ScriptPixelUtility::internal_hasAlpha(PixelFormat format, bool* value)
	{
		*value = PixelUtil::hasAlpha(format);
	}

	void ScriptPixelUtility::internal_isFloatingPoint(PixelFormat format, bool* value)
	{
		*value = PixelUtil::isFloatingPoint(format);
	}

	void ScriptPixelUtility::internal_isCompressed(PixelFormat format, bool* value)
	{
		*value = PixelUtil::isCompressed(format);
	}

	void ScriptPixelUtility::internal_isDepth(PixelFormat format, bool* value)
	{
		*value = PixelUtil::isDepth(format);
	}

	void ScriptPixelUtility::internal_getMaxMipmaps(UINT32 width, UINT32 height, UINT32 depth, PixelFormat format, UINT32* value)
	{
		*value = PixelUtil::getMaxMipmaps(width, height, depth, format);
	}

	MonoObject* ScriptPixelUtility::internal_convertFormat(MonoObject* source, PixelFormat newFormat)
	{
		ScriptPixelData* sourceScriptPixelData = ScriptPixelData::toNative(source);
		if (sourceScriptPixelData == nullptr)
			return nullptr;

		SPtr<PixelData> sourcePixelData = sourceScriptPixelData->getInternalValue();
		SPtr<PixelData> outputData = bs_shared_ptr_new<PixelData>(sourcePixelData->getWidth(), sourcePixelData->getHeight(), 
			sourcePixelData->getDepth(), newFormat);
		outputData->allocateInternalBuffer();

		PixelUtil::bulkPixelConversion(*sourcePixelData, *outputData);

		return ScriptPixelData::create(outputData);
	}

	MonoObject* ScriptPixelUtility::internal_compress(MonoObject* source, CompressionOptions* options)
	{
		ScriptPixelData* sourceScriptPixelData = ScriptPixelData::toNative(source);
		if (sourceScriptPixelData == nullptr)
			return nullptr;

		SPtr<PixelData> sourcePixelData = sourceScriptPixelData->getInternalValue();
		SPtr<PixelData> outputData = bs_shared_ptr_new<PixelData>(sourcePixelData->getWidth(), sourcePixelData->getHeight(), 
			sourcePixelData->getDepth(), options->format);
		outputData->allocateInternalBuffer();

		PixelUtil::compress(*sourcePixelData, *outputData, *options);

		return ScriptPixelData::create(outputData);
	}

	MonoArray* ScriptPixelUtility::internal_generateMipmaps(MonoObject* source, MipMapGenOptions* options)
	{
		ScriptPixelData* sourceScriptPixelData = ScriptPixelData::toNative(source);
		if (sourceScriptPixelData == nullptr)
			return nullptr;

		SPtr<PixelData> sourcePixelData = sourceScriptPixelData->getInternalValue();
		Vector<SPtr<PixelData>> mipmaps = PixelUtil::genMipmaps(*sourcePixelData, *options);

		UINT32 numElements = (UINT32)mipmaps.size();
		ScriptArray scriptArray(ScriptPixelData::getMetaData()->scriptClass->_getInternalClass(), numElements);

		for (UINT32 i = 0; i < numElements; i++)
		{
			MonoObject* managedPixelData = ScriptPixelData::create(mipmaps[i]);
			scriptArray.set(i, managedPixelData);
		}

		return scriptArray.getInternal();
	}

	MonoObject* ScriptPixelUtility::internal_scale(MonoObject* source, PixelVolume* newSize, PixelUtil::Filter filter)
	{
		ScriptPixelData* sourceScriptPixelData = ScriptPixelData::toNative(source);
		if (sourceScriptPixelData == nullptr)
			return nullptr;

		SPtr<PixelData> sourcePixelData = sourceScriptPixelData->getInternalValue();
		SPtr<PixelData> outputData = bs_shared_ptr_new<PixelData>(newSize->getWidth(), newSize->getHeight(),
			newSize->getDepth(), sourcePixelData->getFormat());
		outputData->allocateInternalBuffer();

		PixelUtil::scale(*sourcePixelData, *outputData, filter);

		return ScriptPixelData::create(outputData);
	}

	void ScriptPixelUtility::internal_applyGamma(MonoObject* source, float gamma)
	{
		ScriptPixelData* sourceScriptPixelData = ScriptPixelData::toNative(source);
		if (sourceScriptPixelData == nullptr)
			return;

		SPtr<PixelData> pixelData = sourceScriptPixelData->getInternalValue();
		PixelUtil::applyGamma(pixelData->getData(), gamma, pixelData->getSize(), PixelUtil::getNumElemBits(pixelData->getFormat()));
	}
}