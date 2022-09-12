//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Wrappers/BsScriptPixelUtility.h"
#include "BsMonoManager.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "Debug/BsDebug.h"
#include "Generated/BsScriptPixelData.generated.h"

namespace bs
{
	ScriptPixelUtility::ScriptPixelUtility(MonoObject* instance)
		:ScriptObject(instance)
	{ }

	void ScriptPixelUtility::InitRuntimeData()
	{
		metaData.scriptClass->AddInternalCall("Internal_GetMemorySize", (void*)&ScriptPixelUtility::internal_getMemorySize);
		metaData.scriptClass->AddInternalCall("Internal_HasAlpha", (void*)&ScriptPixelUtility::internal_hasAlpha);
		metaData.scriptClass->AddInternalCall("Internal_IsFloatingPoint", (void*)&ScriptPixelUtility::internal_isFloatingPoint);
		metaData.scriptClass->AddInternalCall("Internal_IsCompressed", (void*)&ScriptPixelUtility::internal_isCompressed);
		metaData.scriptClass->AddInternalCall("Internal_IsDepth", (void*)&ScriptPixelUtility::internal_isDepth);
		metaData.scriptClass->AddInternalCall("Internal_GetMaxMipmaps", (void*)&ScriptPixelUtility::internal_getMaxMipmaps);
		metaData.scriptClass->AddInternalCall("Internal_ConvertFormat", (void*)&ScriptPixelUtility::internal_convertFormat);
		metaData.scriptClass->AddInternalCall("Internal_Compress", (void*)&ScriptPixelUtility::internal_compress);
		metaData.scriptClass->AddInternalCall("Internal_GenerateMipmaps", (void*)&ScriptPixelUtility::internal_generateMipmaps);
		metaData.scriptClass->AddInternalCall("Internal_Scale", (void*)&ScriptPixelUtility::internal_scale);
		metaData.scriptClass->AddInternalCall("Internal_LinearToSRGB", (void*)&ScriptPixelUtility::internal_linearToSRGB);
		metaData.scriptClass->AddInternalCall("Internal_SRGBToLinear", (void*)&ScriptPixelUtility::internal_SRGBToLinear);
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

		SPtr<PixelData> sourcePixelData = sourceScriptPixelData->GetInternal();
		SPtr<PixelData> outputData = bs_shared_ptr_new<PixelData>(sourcePixelData->GetWidth(), sourcePixelData->getHeight(),
			sourcePixelData->GetDepth(), newFormat);
		outputData->AllocateInternalBuffer();

		PixelUtil::bulkPixelConversion(*sourcePixelData, *outputData);

		return ScriptPixelData::Create(outputData);
	}

	MonoObject* ScriptPixelUtility::internal_compress(MonoObject* source, CompressionOptions* options)
	{
		ScriptPixelData* sourceScriptPixelData = ScriptPixelData::toNative(source);
		if (sourceScriptPixelData == nullptr)
			return nullptr;

		SPtr<PixelData> sourcePixelData = sourceScriptPixelData->GetInternal();
		SPtr<PixelData> outputData = bs_shared_ptr_new<PixelData>(sourcePixelData->GetWidth(), sourcePixelData->getHeight(),
			sourcePixelData->GetDepth(), options->format);
		outputData->AllocateInternalBuffer();

		PixelUtil::compress(*sourcePixelData, *outputData, *options);

		return ScriptPixelData::Create(outputData);
	}

	MonoArray* ScriptPixelUtility::internal_generateMipmaps(MonoObject* source, MipMapGenOptions* options)
	{
		ScriptPixelData* sourceScriptPixelData = ScriptPixelData::toNative(source);
		if (sourceScriptPixelData == nullptr)
			return nullptr;

		SPtr<PixelData> sourcePixelData = sourceScriptPixelData->GetInternal();
		Vector<SPtr<PixelData>> mipmaps = PixelUtil::genMipmaps(*sourcePixelData, *options);

		UINT32 numElements = (UINT32)mipmaps.Size();
		ScriptArray ScriptArray(ScriptPixelData::getMetaData()->scriptClass->_getInternalClass(), numElements);

		for (UINT32 i = 0; i < numElements; i++)
		{
			MonoObject* managedPixelData = ScriptPixelData::create(mipmaps[i]);
			scriptArray.Set(i, managedPixelData);
		}

		return scriptArray.GetInternal();
	}

	MonoObject* ScriptPixelUtility::internal_scale(MonoObject* source, PixelVolume* newSize, PixelUtil::Filter filter)
	{
		ScriptPixelData* sourceScriptPixelData = ScriptPixelData::toNative(source);
		if (sourceScriptPixelData == nullptr)
			return nullptr;

		SPtr<PixelData> sourcePixelData = sourceScriptPixelData->GetInternal();
		SPtr<PixelData> outputData = bs_shared_ptr_new<PixelData>(newSize->GetWidth(), newSize->getHeight(),
			newSize->GetDepth(), sourcePixelData->getFormat());
		outputData->AllocateInternalBuffer();

		PixelUtil::scale(*sourcePixelData, *outputData, filter);

		return ScriptPixelData::Create(outputData);
	}

	void ScriptPixelUtility::internal_linearToSRGB(MonoObject* source)
	{
		ScriptPixelData* sourceScriptPixelData = ScriptPixelData::toNative(source);
		if (sourceScriptPixelData == nullptr)
			return;

		SPtr<PixelData> pixelData = sourceScriptPixelData->GetInternal();
		PixelUtil::linearToSRGB(*pixelData);
	}

	void ScriptPixelUtility::internal_SRGBToLinear(MonoObject* source)
	{
		ScriptPixelData* sourceScriptPixelData = ScriptPixelData::toNative(source);
		if (sourceScriptPixelData == nullptr)
			return;

		SPtr<PixelData> pixelData = sourceScriptPixelData->GetInternal();
		PixelUtil::SRGBToLinear(*pixelData);
	}
}
