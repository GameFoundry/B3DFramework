//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Wrappers/BsScriptPixelUtility.h"
#include "BsMonoManager.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "Debug/BsDebug.h"
#include "Generated/BsScriptPixelData.generated.h"

using namespace bs;
ScriptPixelUtility::ScriptPixelUtility(MonoObject* instance)
	: ScriptObject(instance)
{}

void ScriptPixelUtility::InitRuntimeData()
{
	metaData.ScriptClass->AddInternalCall("Internal_GetMemorySize", (void*)&ScriptPixelUtility::InternalGetMemorySize);
	metaData.ScriptClass->AddInternalCall("Internal_HasAlpha", (void*)&ScriptPixelUtility::InternalHasAlpha);
	metaData.ScriptClass->AddInternalCall("Internal_IsFloatingPoint", (void*)&ScriptPixelUtility::InternalIsFloatingPoint);
	metaData.ScriptClass->AddInternalCall("Internal_IsCompressed", (void*)&ScriptPixelUtility::InternalIsCompressed);
	metaData.ScriptClass->AddInternalCall("Internal_IsDepth", (void*)&ScriptPixelUtility::InternalIsDepth);
	metaData.ScriptClass->AddInternalCall("Internal_GetMaxMipmaps", (void*)&ScriptPixelUtility::InternalGetMaxMipmaps);
	metaData.ScriptClass->AddInternalCall("Internal_ConvertFormat", (void*)&ScriptPixelUtility::InternalConvertFormat);
	metaData.ScriptClass->AddInternalCall("Internal_Compress", (void*)&ScriptPixelUtility::InternalCompress);
	metaData.ScriptClass->AddInternalCall("Internal_GenerateMipmaps", (void*)&ScriptPixelUtility::InternalGenerateMipmaps);
	metaData.ScriptClass->AddInternalCall("Internal_Scale", (void*)&ScriptPixelUtility::InternalScale);
	metaData.ScriptClass->AddInternalCall("Internal_LinearToSRGB", (void*)&ScriptPixelUtility::InternalLinearToSrgb);
	metaData.ScriptClass->AddInternalCall("Internal_SRGBToLinear", (void*)&ScriptPixelUtility::InternalSrgbToLinear);
}

void ScriptPixelUtility::InternalGetMemorySize(u32 width, u32 height, u32 depth, PixelFormat format, u32* value)
{
	*value = PixelUtil::GetMemorySize(width, height, depth, format);
}

void ScriptPixelUtility::InternalHasAlpha(PixelFormat format, bool* value)
{
	*value = PixelUtil::HasAlpha(format);
}

void ScriptPixelUtility::InternalIsFloatingPoint(PixelFormat format, bool* value)
{
	*value = PixelUtil::IsFloatingPoint(format);
}

void ScriptPixelUtility::InternalIsCompressed(PixelFormat format, bool* value)
{
	*value = PixelUtil::IsCompressed(format);
}

void ScriptPixelUtility::InternalIsDepth(PixelFormat format, bool* value)
{
	*value = PixelUtil::IsDepth(format);
}

void ScriptPixelUtility::InternalGetMaxMipmaps(u32 width, u32 height, u32 depth, PixelFormat format, u32* value)
{
	*value = PixelUtil::GetMaxMipmaps(width, height, depth, format);
}

MonoObject* ScriptPixelUtility::InternalConvertFormat(MonoObject* source, PixelFormat newFormat)
{
	ScriptPixelData* sourceScriptPixelData = ScriptPixelData::GetScriptObjectWrapper(source);
	if(sourceScriptPixelData == nullptr)
		return nullptr;

	SPtr<PixelData> sourcePixelData = sourceScriptPixelData->GetNativeObjectAsShared();
	SPtr<PixelData> outputData = B3DMakeShared<PixelData>(sourcePixelData->GetWidth(), sourcePixelData->GetHeight(), sourcePixelData->GetDepth(), newFormat);
	outputData->AllocateInternalBuffer();

	PixelUtil::BulkPixelConversion(*sourcePixelData, *outputData);

	return ScriptPixelData::GetOrCreateScriptObject(outputData);
}

MonoObject* ScriptPixelUtility::InternalCompress(MonoObject* source, CompressionOptions* options)
{
	ScriptPixelData* sourceScriptPixelData = ScriptPixelData::GetScriptObjectWrapper(source);
	if(sourceScriptPixelData == nullptr)
		return nullptr;

	SPtr<PixelData> sourcePixelData = sourceScriptPixelData->GetNativeObjectAsShared();
	SPtr<PixelData> outputData = B3DMakeShared<PixelData>(sourcePixelData->GetWidth(), sourcePixelData->GetHeight(), sourcePixelData->GetDepth(), options->Format);
	outputData->AllocateInternalBuffer();

	PixelUtil::Compress(*sourcePixelData, *outputData, *options);

	return ScriptPixelData::GetOrCreateScriptObject(outputData);
}

MonoArray* ScriptPixelUtility::InternalGenerateMipmaps(MonoObject* source, MipMapGenOptions* options)
{
	ScriptPixelData* sourceScriptPixelData = ScriptPixelData::GetScriptObjectWrapper(source);
	if(sourceScriptPixelData == nullptr)
		return nullptr;

	SPtr<PixelData> sourcePixelData = sourceScriptPixelData->GetNativeObjectAsShared();
	Vector<SPtr<PixelData>> mipmaps = PixelUtil::GenMipmaps(*sourcePixelData, *options);

	u32 numElements = (u32)mipmaps.size();
	ScriptArray scriptArray(ScriptPixelData::GetMetaData()->ScriptClass->GetInternalClass(), numElements);

	for(u32 i = 0; i < numElements; i++)
	{
		MonoObject* managedPixelData = ScriptPixelData::GetOrCreateScriptObject(mipmaps[i]);
		scriptArray.Set(i, managedPixelData);
	}

	return scriptArray.GetInternal();
}

MonoObject* ScriptPixelUtility::InternalScale(MonoObject* source, PixelVolume* newSize, PixelUtil::Filter filter)
{
	ScriptPixelData* sourceScriptPixelData = ScriptPixelData::GetScriptObjectWrapper(source);
	if(sourceScriptPixelData == nullptr)
		return nullptr;

	SPtr<PixelData> sourcePixelData = sourceScriptPixelData->GetNativeObjectAsShared();
	SPtr<PixelData> outputData = B3DMakeShared<PixelData>(newSize->GetWidth(), newSize->GetHeight(), newSize->GetDepth(), sourcePixelData->GetFormat());
	outputData->AllocateInternalBuffer();

	PixelUtil::Scale(*sourcePixelData, *outputData, filter);

	return ScriptPixelData::GetOrCreateScriptObject(outputData);
}

void ScriptPixelUtility::InternalLinearToSrgb(MonoObject* source)
{
	ScriptPixelData* sourceScriptPixelData = ScriptPixelData::GetScriptObjectWrapper(source);
	if(sourceScriptPixelData == nullptr)
		return;

	SPtr<PixelData> pixelData = sourceScriptPixelData->GetNativeObjectAsShared();
	PixelUtil::LinearToSrgb(*pixelData);
}

void ScriptPixelUtility::InternalSrgbToLinear(MonoObject* source)
{
	ScriptPixelData* sourceScriptPixelData = ScriptPixelData::GetScriptObjectWrapper(source);
	if(sourceScriptPixelData == nullptr)
		return;

	SPtr<PixelData> pixelData = sourceScriptPixelData->GetNativeObjectAsShared();
	PixelUtil::SRGBToLinear(*pixelData);
}
