//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "RenderAPI/BsGpuParams.h"
#include "RenderAPI/BsGpuParamDesc.h"
#include "RenderAPI/BsGpuParamBlockBuffer.h"
#include "RenderAPI/BsGpuPipelineParamInfo.h"
#include "RenderAPI/BsGpuPipelineState.h"
#include "Math/BsVector2.h"
#include "Image/BsTexture.h"
#include "RenderAPI/BsGpuBuffer.h"
#include "RenderAPI/BsSamplerState.h"
#include "Debug/BsDebug.h"
#include "Error/BsException.h"
#include "Math/BsVector3I.h"
#include "Math/BsVector4I.h"
#include "Math/BsMatrixNxM.h"
#include "Managers/BsHardwareBufferManager.h"

using namespace bs;

const TextureSurface TextureSurface::kComplete = TextureSurface(0, 0, 0, 0);

GpuParamsBase::GpuParamsBase(const SPtr<GpuPipelineParamInfoBase>& paramInfo)
	: mParamInfo(paramInfo)
{}

SPtr<GpuParamDesc> GpuParamsBase::GetParameterInformation(GpuProgramType type) const
{
	return mParamInfo->GetParamDesc(type);
}

u32 GpuParamsBase::GetDataParameterSize(GpuProgramType type, const String& name) const
{
	GpuDataParameterInformation* desc = GetDataParameterInformation(type, name);
	if(desc != nullptr)
		return desc->ElementSize * 4;

	return 0;
}

bool GpuParamsBase::HasParameter(GpuProgramType type, const String& name) const
{
	return GetDataParameterInformation(type, name) != nullptr;
}

bool GpuParamsBase::HasTexture(GpuProgramType type, const String& name) const
{
	const SPtr<GpuParamDesc>& paramDesc = mParamInfo->GetParamDesc(type);
	if(paramDesc == nullptr)
		return false;

	auto paramIter = paramDesc->Textures.find(name);
	if(paramIter != paramDesc->Textures.end())
		return true;

	return false;
}

bool GpuParamsBase::HasBuffer(GpuProgramType type, const String& name) const
{
	const SPtr<GpuParamDesc>& paramDesc = mParamInfo->GetParamDesc(type);
	if(paramDesc == nullptr)
		return false;

	auto paramIter = paramDesc->Buffers.find(name);
	if(paramIter != paramDesc->Buffers.end())
		return true;

	return false;
}

bool GpuParamsBase::HasStorageTexture(GpuProgramType type, const String& name) const
{
	const SPtr<GpuParamDesc>& paramDesc = mParamInfo->GetParamDesc(type);
	if(paramDesc == nullptr)
		return false;

	auto paramIter = paramDesc->LoadStoreTextures.find(name);
	if(paramIter != paramDesc->LoadStoreTextures.end())
		return true;

	return false;
}

bool GpuParamsBase::HasSamplerState(GpuProgramType type, const String& name) const
{
	const SPtr<GpuParamDesc>& paramDesc = mParamInfo->GetParamDesc(type);
	if(paramDesc == nullptr)
		return false;

	auto paramIter = paramDesc->Samplers.find(name);
	if(paramIter != paramDesc->Samplers.end())
		return true;

	return false;
}

bool GpuParamsBase::HasParamBlock(GpuProgramType type, const String& name) const
{
	const SPtr<GpuParamDesc>& gpuParameterInformation = mParamInfo->GetParamDesc(type);
	if(gpuParameterInformation == nullptr)
		return false;

	return gpuParameterInformation->ParamBlocks.find(name) != gpuParameterInformation->ParamBlocks.end();
}

bool GpuParamsBase::HasParamBlock(const String& name) const
{
	for (u32 gpuProgramTypeIndex = 0; gpuProgramTypeIndex < GPT_COUNT; gpuProgramTypeIndex++)
	{
		const SPtr<GpuParamDesc>& gpuParameterInformation = mParamInfo->GetParamDesc((GpuProgramType)gpuProgramTypeIndex);
		if (gpuParameterInformation == nullptr)
			continue;

		auto found = gpuParameterInformation->ParamBlocks.find(name);
		if (found != gpuParameterInformation->ParamBlocks.end())
			return true;
	}

	return false;
}

GpuDataParameterInformation* GpuParamsBase::GetDataParameterInformation(GpuProgramType type, const String& name) const
{
	const SPtr<GpuParamDesc>& paramDesc = mParamInfo->GetParamDesc(type);
	if(paramDesc == nullptr)
		return nullptr;

	auto paramIter = paramDesc->Params.find(name);
	if(paramIter != paramDesc->Params.end())
		return &paramIter->second;

	return nullptr;
}

GpuParameterBlockInformation* GpuParamsBase::GetParameterBlockDesc(GpuProgramType type, const String& name) const
{
	const SPtr<GpuParamDesc>& paramDesc = mParamInfo->GetParamDesc(type);
	if(paramDesc == nullptr)
		return nullptr;

	auto paramBlockIter = paramDesc->ParamBlocks.find(name);
	if(paramBlockIter != paramDesc->ParamBlocks.end())
		return &paramBlockIter->second;

	return nullptr;
}

template <bool Core>
TGpuParams<Core>::TGpuParams(const SPtr<GpuPipelineParamInfoBase>& paramInfo)
	: GpuParamsBase(paramInfo)
{
	const u32 parameterBlockCount = mParamInfo->GetResourceCount(GpuPipelineParamInfo::ParamType::ParamBlock);
	const u32 textureCount = mParamInfo->GetResourceCount(GpuPipelineParamInfo::ParamType::Texture);
	const u32 storageTextureCount = mParamInfo->GetResourceCount(GpuPipelineParamInfo::ParamType::LoadStoreTexture);
	const u32 bufferCount = mParamInfo->GetResourceCount(GpuPipelineParamInfo::ParamType::Buffer);
	const u32 samplerCount = mParamInfo->GetResourceCount(GpuPipelineParamInfo::ParamType::SamplerState);

	const u32 parameterBlockEntrySize = Math::RoundToMultiple((u32)sizeof(ParamsBufferType), 16u);
	const u32 textureEntrySize = Math::RoundToMultiple((u32)sizeof(TextureData), 16u);
	const u32 bufferEntrySize = Math::RoundToMultiple((u32)sizeof(BufferType), 16u);
	const u32 samplerStateEntrySize = Math::RoundToMultiple((u32)sizeof(SamplerType), 16u);

	const u32 parameterBlockBufferSize = parameterBlockEntrySize * parameterBlockCount;
	const u32 texturesBufferSize = textureEntrySize * textureCount;
	const u32 loadStoreTexturesBufferSize = textureEntrySize * storageTextureCount;
	const u32 buffersBufferSize = bufferEntrySize * bufferCount;
	const u32 samplerStatesBufferSize = samplerStateEntrySize * samplerCount;

	u32 totalSize = parameterBlockBufferSize + texturesBufferSize + loadStoreTexturesBufferSize + buffersBufferSize + samplerStatesBufferSize;

	u8* data = (u8*)B3DAllocate(totalSize);
	mParamBlockBuffers = (ParamsBufferType*)data;
	for(u32 i = 0; i < parameterBlockCount; i++)
		new(&mParamBlockBuffers[i]) ParamsBufferType();

	data += parameterBlockBufferSize;
	mSampledTextureData = (TextureData*)data;
	for(u32 i = 0; i < textureCount; i++)
	{
		new(&mSampledTextureData[i].Texture) TextureType();
		new(&mSampledTextureData[i].Surface) TextureSurface(0, 0, 0, 0);
	}

	data += texturesBufferSize;
	mStorageTextureData = (TextureData*)data;
	for(u32 i = 0; i < storageTextureCount; i++)
	{
		new(&mStorageTextureData[i].Texture) TextureType();
		new(&mStorageTextureData[i].Surface) TextureSurface(0, 0, 0, 0);
	}

	data += loadStoreTexturesBufferSize;
	mBuffers = (BufferType*)data;
	for(u32 i = 0; i < bufferCount; i++)
		new(&mBuffers[i]) BufferType();

	data += buffersBufferSize;
	mSamplerStates = (SamplerType*)data;
	for(u32 i = 0; i < samplerCount; i++)
		new(&mSamplerStates[i]) SamplerType();

	data += samplerStatesBufferSize;
}

template <bool Core>
TGpuParams<Core>::~TGpuParams()
{
	const u32 parameterBlockCount = mParamInfo->GetResourceCount(GpuPipelineParamInfo::ParamType::ParamBlock);
	const u32 textureCount = mParamInfo->GetResourceCount(GpuPipelineParamInfo::ParamType::Texture);
	const u32 loadStoreTextureCount = mParamInfo->GetResourceCount(GpuPipelineParamInfo::ParamType::LoadStoreTexture);
	const u32 bufferCount = mParamInfo->GetResourceCount(GpuPipelineParamInfo::ParamType::Buffer);
	const u32 samplerCount = mParamInfo->GetResourceCount(GpuPipelineParamInfo::ParamType::SamplerState);

	for(u32 i = 0; i < parameterBlockCount; i++)
		mParamBlockBuffers[i].~ParamsBufferType();

	for(u32 i = 0; i < textureCount; i++)
	{
		mSampledTextureData[i].Texture.~TextureType();
		mSampledTextureData[i].Surface.~TextureSurface();
	}

	for(u32 i = 0; i < loadStoreTextureCount; i++)
	{
		mStorageTextureData[i].Texture.~TextureType();
		mStorageTextureData[i].Surface.~TextureSurface();
	}

	for(u32 i = 0; i < bufferCount; i++)
		mBuffers[i].~BufferType();

	for(u32 i = 0; i < samplerCount; i++)
		mSamplerStates[i].~SamplerType();

	// Everything is allocated in a single block, so it's enough to free the first element
	B3DFree(mParamBlockBuffers);
}

template <bool Core>
bool TGpuParams<Core>::SetParameterBlockBuffer(u32 set, u32 slot, const ParamsBufferType& paramBlockBuffer, u32 arrayIndex)
{
	const u32 sequentialResourceIndex = mParamInfo->GetSequentialResourceIndex(GpuPipelineParamInfo::ParamType::ParamBlock, set, slot, arrayIndex);
	if (sequentialResourceIndex == ~0u)
	{
		B3D_LOG(Warning, RenderBackend, "Unable to assign parameter. Cannot find parameter block with the set/slot combination: {0}/{1}", set, slot);
		return false;
	}

	mParamBlockBuffers[sequentialResourceIndex] = paramBlockBuffer;

	MarkCoreDirtyInternal();
	return true;
}

template <bool Core>
bool TGpuParams<Core>::SetParameterBlockBuffer(GpuProgramType type, const String& name, const ParamsBufferType& paramBlockBuffer, u32 arrayIndex)
{
	const SPtr<GpuParamDesc>& parameterInformation = mParamInfo->GetParamDesc(type);
	if(parameterInformation == nullptr)
	{
		B3D_LOG(Warning, RenderBackend, "Unable to assign parameter. Cannot find parameter block with name: {0}", name);
		return false;
	}

	auto iterFind = parameterInformation->ParamBlocks.find(name);
	if(iterFind == parameterInformation->ParamBlocks.end())
	{
		B3D_LOG(Warning, RenderBackend, "Unable to assign parameter. Cannot find parameter block with name: {0}", name);
		return false;
	}

	return SetParameterBlockBuffer(iterFind->second.Set, iterFind->second.Slot, paramBlockBuffer, arrayIndex);
}

template<bool Core>
bool TGpuParams<Core>::TrySetParameterBlockBuffer(GpuProgramType type, const String& name, const ParamsBufferType& parameterBlockBuffer, u32 arrayIndex)
{
	if (!HasParamBlock(type, name))
		return false;

	return SetParameterBlockBuffer(type, name, parameterBlockBuffer, arrayIndex);
}

template<bool Core>
bool TGpuParams<Core>::TrySetParameterBlockBuffer(const String& name, const ParamsBufferType& paramBlockBuffer, u32 arrayIndex)
{
	if (!HasParamBlock(name))
		return false;

	return SetParameterBlockBuffer(name, paramBlockBuffer, arrayIndex);
}

template <bool Core>
bool TGpuParams<Core>::SetParameterBlockBuffer(const String& name, const ParamsBufferType& paramBlockBuffer, u32 arrayIndex)
{
	bool foundMatchingParameter = false;
	bool isParameterBound = false;
	for(u32 i = 0; i < 6; i++)
	{
		const SPtr<GpuParamDesc>& paramDescs = mParamInfo->GetParamDesc((GpuProgramType)i);
		if(paramDescs == nullptr)
			continue;

		auto iterFind = paramDescs->ParamBlocks.find(name);
		if(iterFind == paramDescs->ParamBlocks.end())
			continue;

		if (SetParameterBlockBuffer(iterFind->second.Set, iterFind->second.Slot, paramBlockBuffer, arrayIndex))
			isParameterBound = true;

		foundMatchingParameter = true;
	}

	if (!foundMatchingParameter)
	{
		B3D_LOG(Warning, RenderBackend, "Unable to assign parameter. Cannot find parameter block with name: {0}", name);
		return false;
	}

	return isParameterBound;
}

template <bool Core>
template <class T>
void TGpuParams<Core>::GetParameter(GpuProgramType type, const String& name, TGpuParameterPrimitive<T, Core>& output) const
{
	if(!TryGetParameter(type, name, output))
		B3D_LOG(Warning, RenderBackend, "Cannot find parameter with the name: '{0}'", name);
}

template <bool Core>
void TGpuParams<Core>::GetStructParameter(GpuProgramType type, const String& name, TGpuParameterStruct<Core>& output) const
{
	if(!TryGetStructParameter(type, name, output))
		B3D_LOG(Warning, RenderBackend, "Cannot find struct parameter with the name: '{0}'", name);
}

template <bool Core>
void TGpuParams<Core>::GetTextureParameter(GpuProgramType type, const String& name, TGpuParameterSampledTexture<Core>& output) const
{
	if(!TryGetTextureParameter(type, name, output))
		B3D_LOG(Warning, RenderBackend, "Cannot find texture parameter with the name: '{0}'", name);
}

template <bool Core>
void TGpuParams<Core>::GetStorageTextureParameter(GpuProgramType type, const String& name, TGpuParameterStorageTexture<Core>& output) const
{
	if(!TryGetStorageTextureParameter(type, name, output))
		B3D_LOG(Warning, RenderBackend, "Cannot find storage texture parameter with the name: '{0}'", name);
}

template <bool Core>
void TGpuParams<Core>::GetBufferParameter(GpuProgramType type, const String& name, TGpuParameterBuffer<Core>& output) const
{
	if(!TryGetBufferParameter(type, name, output))
		B3D_LOG(Warning, RenderBackend, "Cannot find buffer parameter with the name: '{0}'", name);
}

template <bool Core>
void TGpuParams<Core>::GetSamplerStateParameter(GpuProgramType type, const String& name, TGpuParameterSampler<Core>& output) const
{
	if(!TryGetSamplerStateParameter(type, name, output))
		B3D_LOG(Warning, RenderBackend, "Cannot find sampler parameter with the name: '{0}'", name);
}

template <bool Core>
template <class T>
bool TGpuParams<Core>::TryGetParameter(GpuProgramType type, const String& name, TGpuParameterPrimitive<T, Core>& output) const
{
	const SPtr<GpuParamDesc>& paramDescs = mParamInfo->GetParamDesc(type);
	if (paramDescs == nullptr)
	{
		output = TGpuParameterPrimitive<T, Core>(nullptr, nullptr);
		return false;
	}

	auto iterFind = paramDescs->Params.find(name);
	if (iterFind == paramDescs->Params.end())
	{
		output = TGpuParameterPrimitive<T, Core>(nullptr, nullptr);
		return false;
	}

	output = TGpuParameterPrimitive<T, Core>(&iterFind->second, GetThisPtrInternal());
	return true;
}

template <bool Core>
bool TGpuParams<Core>::TryGetStructParameter(GpuProgramType type, const String& name, TGpuParameterStruct<Core>& output) const
{
	const SPtr<GpuParamDesc>& paramDescs = mParamInfo->GetParamDesc(type);
	if (paramDescs == nullptr)
	{
		output = TGpuParameterStruct<Core>(nullptr, nullptr);
		return false;
	}

	auto iterFind = paramDescs->Params.find(name);
	if (iterFind == paramDescs->Params.end() || iterFind->second.Type != GPDT_STRUCT)
	{
		output = TGpuParameterStruct<Core>(nullptr, nullptr);
		return false;
	}

	output = TGpuParameterStruct<Core>(&iterFind->second, GetThisPtrInternal());
	return true;
}

template <bool Core>
bool TGpuParams<Core>::TryGetTextureParameter(GpuProgramType type, const String& name, TGpuParameterSampledTexture<Core>& output) const
{
	const SPtr<GpuParamDesc>& paramDescs = mParamInfo->GetParamDesc(type);
	if (paramDescs == nullptr)
	{
		output = TGpuParameterSampledTexture<Core>(nullptr, nullptr);
		return false;
	}

	auto iterFind = paramDescs->Textures.find(name);
	if (iterFind == paramDescs->Textures.end())
	{
		output = TGpuParameterSampledTexture<Core>(nullptr, nullptr);
		return false;
	}

	output = TGpuParameterSampledTexture<Core>(&iterFind->second, GetThisPtrInternal());
	return true;
}

template <bool Core>
bool TGpuParams<Core>::TryGetStorageTextureParameter(GpuProgramType type, const String& name, TGpuParameterStorageTexture<Core>& output) const
{
	const SPtr<GpuParamDesc>& paramDescs = mParamInfo->GetParamDesc(type);
	if (paramDescs == nullptr)
	{
		output = TGpuParameterStorageTexture<Core>(nullptr, nullptr);
		return false;
	}

	auto iterFind = paramDescs->LoadStoreTextures.find(name);
	if (iterFind == paramDescs->LoadStoreTextures.end())
	{
		output = TGpuParameterStorageTexture<Core>(nullptr, nullptr);
		return false;
	}

	output = TGpuParameterStorageTexture<Core>(&iterFind->second, GetThisPtrInternal());
	return true;
}

template <bool Core>
bool TGpuParams<Core>::TryGetBufferParameter(GpuProgramType type, const String& name, TGpuParameterBuffer<Core>& output) const
{
	const SPtr<GpuParamDesc>& paramDescs = mParamInfo->GetParamDesc(type);
	if (paramDescs == nullptr)
	{
		output = TGpuParameterBuffer<Core>(nullptr, nullptr);
		return false;
	}

	auto iterFind = paramDescs->Buffers.find(name);
	if (iterFind == paramDescs->Buffers.end())
	{
		output = TGpuParameterBuffer<Core>(nullptr, nullptr);
		return false;
	}

	output = TGpuParameterBuffer<Core>(&iterFind->second, GetThisPtrInternal());
	return true;
}

template <bool Core>
bool TGpuParams<Core>::TryGetSamplerStateParameter(GpuProgramType type, const String& name, TGpuParameterSampler<Core>& output) const
{
	const SPtr<GpuParamDesc>& paramDescs = mParamInfo->GetParamDesc(type);
	if (paramDescs == nullptr)
	{
		output = TGpuParameterSampler<Core>(nullptr, nullptr);
		return false;
	}

	auto iterFind = paramDescs->Samplers.find(name);
	if (iterFind == paramDescs->Samplers.end())
	{
		output = TGpuParameterSampler<Core>(nullptr, nullptr);
		return false;
	}

	output = TGpuParameterSampler<Core>(&iterFind->second, GetThisPtrInternal());
	return true;
}

template <bool Core>
typename TGpuParams<Core>::ParamsBufferType TGpuParams<Core>::GetParameterBlockBuffer(u32 set, u32 slot, u32 arrayIndex) const
{
	const u32 sequentialResourceIndex = mParamInfo->GetSequentialResourceIndex(GpuPipelineParamInfo::ParamType::ParamBlock, set, slot, arrayIndex);
	if(sequentialResourceIndex == ~0u)
		return nullptr;

	return mParamBlockBuffers[sequentialResourceIndex];
}

template <bool Core>
typename TGpuParams<Core>::TextureType TGpuParams<Core>::GetTexture(u32 set, u32 slot, u32 arrayIndex) const
{
	const u32 sequentialResourceIndex = mParamInfo->GetSequentialResourceIndex(GpuPipelineParamInfo::ParamType::Texture, set, slot, arrayIndex);
	if(sequentialResourceIndex == ~0u)
		return TGpuParams<Core>::TextureType();

	return mSampledTextureData[sequentialResourceIndex].Texture;
}

template <bool Core>
typename TGpuParams<Core>::TextureType TGpuParams<Core>::GetStorageTexture(u32 set, u32 slot, u32 arrayIndex) const
{
	const u32 sequentialResourceIndex = mParamInfo->GetSequentialResourceIndex(GpuPipelineParamInfo::ParamType::LoadStoreTexture, set, slot, arrayIndex);
	if(sequentialResourceIndex == ~0u)
		return TGpuParams<Core>::TextureType();

	return mStorageTextureData[sequentialResourceIndex].Texture;
}

template <bool Core>
typename TGpuParams<Core>::BufferType TGpuParams<Core>::GetBuffer(u32 set, u32 slot, u32 arrayIndex) const
{
	const u32 sequentialResourceIndex = mParamInfo->GetSequentialResourceIndex(GpuPipelineParamInfo::ParamType::Buffer, set, slot, arrayIndex);
	if(sequentialResourceIndex == ~0u)
		return nullptr;

	return mBuffers[sequentialResourceIndex];
}

template <bool Core>
typename TGpuParams<Core>::SamplerType TGpuParams<Core>::GetSamplerState(u32 set, u32 slot, u32 arrayIndex) const
{
	const u32 sequentialArrayIndex = mParamInfo->GetSequentialResourceIndex(GpuPipelineParamInfo::ParamType::SamplerState, set, slot, arrayIndex);
	if(sequentialArrayIndex == ~0u)
		return nullptr;

	return mSamplerStates[sequentialArrayIndex];
}

template <bool Core>
const TextureSurface& TGpuParams<Core>::GetTextureSurface(u32 set, u32 slot, u32 arrayIndex) const
{
	static TextureSurface emptySurface;

	const u32 sequentialArrayIndex = mParamInfo->GetSequentialResourceIndex(GpuPipelineParamInfo::ParamType::Texture, set, slot, arrayIndex);
	if(sequentialArrayIndex == ~0u)
		return emptySurface;

	return mSampledTextureData[sequentialArrayIndex].Surface;
}

template <bool Core>
const TextureSurface& TGpuParams<Core>::GetStorageTextureSurface(u32 set, u32 slot, u32 arrayIndex) const
{
	static TextureSurface emptySurface;

	const u32 sequentialArrayIndex = mParamInfo->GetSequentialResourceIndex(GpuPipelineParamInfo::ParamType::LoadStoreTexture, set, slot, arrayIndex);
	if(sequentialArrayIndex == ~0u)
		return emptySurface;

	return mStorageTextureData[sequentialArrayIndex].Surface;
}

template <bool Core>
bool TGpuParams<Core>::SetTexture(u32 set, u32 slot, const TextureType& texture, const TextureSurface& surface, u32 arrayIndex)
{
	const u32 sequentialArrayIndex = mParamInfo->GetSequentialResourceIndex(GpuPipelineParamInfo::ParamType::Texture, set, slot, arrayIndex);
	if (sequentialArrayIndex == ~0u)
	{
		B3D_LOG(Warning, RenderBackend, "Unable to assign parameter. Cannot find sampled texture parameter with the set/slot combination: {0}/{1}", set, slot);
		return false;
	}

	mSampledTextureData[sequentialArrayIndex].Texture = texture;
	mSampledTextureData[sequentialArrayIndex].Surface = surface;

	MarkResourcesDirtyInternal();
	MarkCoreDirtyInternal();

	return true;
}

template <bool Core>
bool TGpuParams<Core>::SetStorageTexture(u32 set, u32 slot, const TextureType& texture, const TextureSurface& surface, u32 arrayIndex)
{
	const u32 sequentialArrayIndex = mParamInfo->GetSequentialResourceIndex(GpuPipelineParamInfo::ParamType::LoadStoreTexture, set, slot, arrayIndex);
	if (sequentialArrayIndex == ~0u)
	{
		B3D_LOG(Warning, RenderBackend, "Unable to assign parameter. Cannot find storage texture parameter with the set/slot combination: {0}/{1}", set, slot);
		return false;
	}

	mStorageTextureData[sequentialArrayIndex].Texture = texture;
	mStorageTextureData[sequentialArrayIndex].Surface = surface;

	MarkResourcesDirtyInternal();
	MarkCoreDirtyInternal();

	return true;
}

template <bool Core>
bool TGpuParams<Core>::SetBuffer(u32 set, u32 slot, const BufferType& buffer, u32 arrayIndex)
{
	const u32 sequentialArrayIndex = mParamInfo->GetSequentialResourceIndex(GpuPipelineParamInfo::ParamType::Buffer, set, slot, arrayIndex);
	if (sequentialArrayIndex == ~0u)
	{
		B3D_LOG(Warning, RenderBackend, "Unable to assign parameter. Cannot find buffer parameter with the set/slot combination: {0}/{1}", set, slot);
		return false;
	}

	mBuffers[sequentialArrayIndex] = buffer;

	MarkResourcesDirtyInternal();
	MarkCoreDirtyInternal();

	return true;
}

template <bool Core>
bool TGpuParams<Core>::SetSamplerState(u32 set, u32 slot, const SamplerType& sampler, u32 arrayIndex)
{
	const u32 sequentialArrayIndex = mParamInfo->GetSequentialResourceIndex(GpuPipelineParamInfo::ParamType::SamplerState, set, slot, arrayIndex);
	if (sequentialArrayIndex == ~0u)
	{
		B3D_LOG(Warning, RenderBackend, "Unable to assign parameter. Cannot find sampler parameter with the set/slot combination: {0}/{1}", set, slot);
		return false;
	}

	mSamplerStates[sequentialArrayIndex] = sampler;

	MarkResourcesDirtyInternal();
	MarkCoreDirtyInternal();

	return true;
}

template class TGpuParams<false>;
template class TGpuParams<true>;

template B3D_CORE_EXPORT void TGpuParams<false>::GetParameter<float>(GpuProgramType type, const String&, TGpuParameterPrimitive<float, false>&) const;
template B3D_CORE_EXPORT void TGpuParams<false>::GetParameter<int>(GpuProgramType type, const String&, TGpuParameterPrimitive<int, false>&) const;
template B3D_CORE_EXPORT void TGpuParams<false>::GetParameter<Color>(GpuProgramType type, const String&, TGpuParameterPrimitive<Color, false>&) const;
template B3D_CORE_EXPORT void TGpuParams<false>::GetParameter<Vector2>(GpuProgramType type, const String&, TGpuParameterPrimitive<Vector2, false>&) const;
template B3D_CORE_EXPORT void TGpuParams<false>::GetParameter<Vector3>(GpuProgramType type, const String&, TGpuParameterPrimitive<Vector3, false>&) const;
template B3D_CORE_EXPORT void TGpuParams<false>::GetParameter<Vector4>(GpuProgramType type, const String&, TGpuParameterPrimitive<Vector4, false>&) const;
template B3D_CORE_EXPORT void TGpuParams<false>::GetParameter<Vector2I>(GpuProgramType type, const String&, TGpuParameterPrimitive<Vector2I, false>&) const;
template B3D_CORE_EXPORT void TGpuParams<false>::GetParameter<Vector3I>(GpuProgramType type, const String&, TGpuParameterPrimitive<Vector3I, false>&) const;
template B3D_CORE_EXPORT void TGpuParams<false>::GetParameter<Vector4I>(GpuProgramType type, const String&, TGpuParameterPrimitive<Vector4I, false>&) const;
template B3D_CORE_EXPORT void TGpuParams<false>::GetParameter<Matrix2>(GpuProgramType type, const String&, TGpuParameterPrimitive<Matrix2, false>&) const;
template B3D_CORE_EXPORT void TGpuParams<false>::GetParameter<Matrix2x3>(GpuProgramType type, const String&, TGpuParameterPrimitive<Matrix2x3, false>&) const;
template B3D_CORE_EXPORT void TGpuParams<false>::GetParameter<Matrix2x4>(GpuProgramType type, const String&, TGpuParameterPrimitive<Matrix2x4, false>&) const;
template B3D_CORE_EXPORT void TGpuParams<false>::GetParameter<Matrix3>(GpuProgramType type, const String&, TGpuParameterPrimitive<Matrix3, false>&) const;
template B3D_CORE_EXPORT void TGpuParams<false>::GetParameter<Matrix3x2>(GpuProgramType type, const String&, TGpuParameterPrimitive<Matrix3x2, false>&) const;
template B3D_CORE_EXPORT void TGpuParams<false>::GetParameter<Matrix3x4>(GpuProgramType type, const String&, TGpuParameterPrimitive<Matrix3x4, false>&) const;
template B3D_CORE_EXPORT void TGpuParams<false>::GetParameter<Matrix4>(GpuProgramType type, const String&, TGpuParameterPrimitive<Matrix4, false>&) const;
template B3D_CORE_EXPORT void TGpuParams<false>::GetParameter<Matrix4x2>(GpuProgramType type, const String&, TGpuParameterPrimitive<Matrix4x2, false>&) const;
template B3D_CORE_EXPORT void TGpuParams<false>::GetParameter<Matrix4x3>(GpuProgramType type, const String&, TGpuParameterPrimitive<Matrix4x3, false>&) const;

template B3D_CORE_EXPORT void TGpuParams<true>::GetParameter<float>(GpuProgramType type, const String&, TGpuParameterPrimitive<float, true>&) const;
template B3D_CORE_EXPORT void TGpuParams<true>::GetParameter<int>(GpuProgramType type, const String&, TGpuParameterPrimitive<int, true>&) const;
template B3D_CORE_EXPORT void TGpuParams<true>::GetParameter<Color>(GpuProgramType type, const String&, TGpuParameterPrimitive<Color, true>&) const;
template B3D_CORE_EXPORT void TGpuParams<true>::GetParameter<Vector2>(GpuProgramType type, const String&, TGpuParameterPrimitive<Vector2, true>&) const;
template B3D_CORE_EXPORT void TGpuParams<true>::GetParameter<Vector3>(GpuProgramType type, const String&, TGpuParameterPrimitive<Vector3, true>&) const;
template B3D_CORE_EXPORT void TGpuParams<true>::GetParameter<Vector4>(GpuProgramType type, const String&, TGpuParameterPrimitive<Vector4, true>&) const;
template B3D_CORE_EXPORT void TGpuParams<true>::GetParameter<Vector2I>(GpuProgramType type, const String&, TGpuParameterPrimitive<Vector2I, true>&) const;
template B3D_CORE_EXPORT void TGpuParams<true>::GetParameter<Vector3I>(GpuProgramType type, const String&, TGpuParameterPrimitive<Vector3I, true>&) const;
template B3D_CORE_EXPORT void TGpuParams<true>::GetParameter<Vector4I>(GpuProgramType type, const String&, TGpuParameterPrimitive<Vector4I, true>&) const;
template B3D_CORE_EXPORT void TGpuParams<true>::GetParameter<Matrix2>(GpuProgramType type, const String&, TGpuParameterPrimitive<Matrix2, true>&) const;
template B3D_CORE_EXPORT void TGpuParams<true>::GetParameter<Matrix2x3>(GpuProgramType type, const String&, TGpuParameterPrimitive<Matrix2x3, true>&) const;
template B3D_CORE_EXPORT void TGpuParams<true>::GetParameter<Matrix2x4>(GpuProgramType type, const String&, TGpuParameterPrimitive<Matrix2x4, true>&) const;
template B3D_CORE_EXPORT void TGpuParams<true>::GetParameter<Matrix3>(GpuProgramType type, const String&, TGpuParameterPrimitive<Matrix3, true>&) const;
template B3D_CORE_EXPORT void TGpuParams<true>::GetParameter<Matrix3x2>(GpuProgramType type, const String&, TGpuParameterPrimitive<Matrix3x2, true>&) const;
template B3D_CORE_EXPORT void TGpuParams<true>::GetParameter<Matrix3x4>(GpuProgramType type, const String&, TGpuParameterPrimitive<Matrix3x4, true>&) const;
template B3D_CORE_EXPORT void TGpuParams<true>::GetParameter<Matrix4>(GpuProgramType type, const String&, TGpuParameterPrimitive<Matrix4, true>&) const;
template B3D_CORE_EXPORT void TGpuParams<true>::GetParameter<Matrix4x2>(GpuProgramType type, const String&, TGpuParameterPrimitive<Matrix4x2, true>&) const;
template B3D_CORE_EXPORT void TGpuParams<true>::GetParameter<Matrix4x3>(GpuProgramType type, const String&, TGpuParameterPrimitive<Matrix4x3, true>&) const;

const GpuDataParameterTypeInformationLookup GpuParams::kParamSizes;

GpuParams::GpuParams(const SPtr<GpuPipelineParamInfo>& paramInfo)
	: TGpuParams(paramInfo)
{
}

SPtr<GpuParams> GpuParams::GetThisPtrInternal() const
{
	return std::static_pointer_cast<GpuParams>(GetThisPtr());
}

SPtr<ct::GpuParams> GpuParams::GetCore() const
{
	return std::static_pointer_cast<ct::GpuParams>(mCoreSpecific);
}

SPtr<ct::CoreObject> GpuParams::CreateCore() const
{
	SPtr<GpuPipelineParamInfo> paramInfo = std::static_pointer_cast<GpuPipelineParamInfo>(mParamInfo);

	return ct::HardwareBufferManager::Instance().CreateGpuParams(paramInfo->GetCore());
}

void GpuParams::MarkCoreDirtyInternal()
{
	MarkCoreDirty();
}

void GpuParams::MarkResourcesDirtyInternal()
{
	MarkListenerResourcesDirty();
}

SPtr<GpuParams> GpuParams::Create(const SPtr<GraphicsPipelineState>& pipelineState)
{
	return HardwareBufferManager::Instance().CreateGpuParams(pipelineState->GetParamInfo());
}

SPtr<GpuParams> GpuParams::Create(const SPtr<ComputePipelineState>& pipelineState)
{
	return HardwareBufferManager::Instance().CreateGpuParams(pipelineState->GetParamInfo());
}

SPtr<GpuParams> GpuParams::Create(const SPtr<GpuPipelineParamInfo>& paramInfo)
{
	return HardwareBufferManager::Instance().CreateGpuParams(paramInfo);
}

CoreSyncData GpuParams::SyncToCore(FrameAlloc* allocator)
{
	u32 numParamBlocks = mParamInfo->GetResourceCount(GpuPipelineParamInfo::ParamType::ParamBlock);
	u32 numTextures = mParamInfo->GetResourceCount(GpuPipelineParamInfo::ParamType::Texture);
	u32 numStorageTextures = mParamInfo->GetResourceCount(GpuPipelineParamInfo::ParamType::LoadStoreTexture);
	u32 numBuffers = mParamInfo->GetResourceCount(GpuPipelineParamInfo::ParamType::Buffer);
	u32 numSamplers = mParamInfo->GetResourceCount(GpuPipelineParamInfo::ParamType::SamplerState);

	u32 sampledSurfacesSize = numTextures * sizeof(TextureSurface);
	u32 loadStoreSurfacesSize = numStorageTextures * sizeof(TextureSurface);
	u32 paramBufferSize = numParamBlocks * sizeof(SPtr<ct::GpuParamBlockBuffer>);
	u32 textureArraySize = numTextures * sizeof(SPtr<ct::Texture>);
	u32 loadStoreTextureArraySize = numStorageTextures * sizeof(SPtr<ct::Texture>);
	u32 bufferArraySize = numBuffers * sizeof(SPtr<ct::GpuBuffer>);
	u32 samplerArraySize = numSamplers * sizeof(SPtr<ct::SamplerState>);

	u32 totalSize = sampledSurfacesSize + loadStoreSurfacesSize + paramBufferSize + textureArraySize + loadStoreTextureArraySize + bufferArraySize + samplerArraySize;

	u32 sampledSurfaceOffset = 0;
	u32 loadStoreSurfaceOffset = sampledSurfaceOffset + sampledSurfacesSize;
	u32 paramBufferOffset = loadStoreSurfaceOffset + loadStoreSurfacesSize;
	u32 textureArrayOffset = paramBufferOffset + paramBufferSize;
	u32 loadStoreTextureArrayOffset = textureArrayOffset + textureArraySize;
	u32 bufferArrayOffset = loadStoreTextureArrayOffset + loadStoreTextureArraySize;
	u32 samplerArrayOffset = bufferArrayOffset + bufferArraySize;

	u8* data = allocator->Alloc(totalSize);

	TextureSurface* sampledSurfaces = (TextureSurface*)(data + sampledSurfaceOffset);
	TextureSurface* loadStoreSurfaces = (TextureSurface*)(data + loadStoreSurfaceOffset);
	SPtr<ct::GpuParamBlockBuffer>* paramBuffers = (SPtr<ct::GpuParamBlockBuffer>*)(data + paramBufferOffset);
	SPtr<ct::Texture>* textures = (SPtr<ct::Texture>*)(data + textureArrayOffset);
	SPtr<ct::Texture>* loadStoreTextures = (SPtr<ct::Texture>*)(data + loadStoreTextureArrayOffset);
	SPtr<ct::GpuBuffer>* buffers = (SPtr<ct::GpuBuffer>*)(data + bufferArrayOffset);
	SPtr<ct::SamplerState>* samplers = (SPtr<ct::SamplerState>*)(data + samplerArrayOffset);

	// Construct & copy
	for(u32 i = 0; i < numParamBlocks; i++)
	{
		new(&paramBuffers[i]) SPtr<ct::GpuParamBlockBuffer>();

		if(mParamBlockBuffers[i] != nullptr)
			paramBuffers[i] = mParamBlockBuffers[i]->GetCore();
	}

	for(u32 i = 0; i < numTextures; i++)
	{
		new(&sampledSurfaces[i]) TextureSurface();
		sampledSurfaces[i] = mSampledTextureData[i].Surface;

		new(&textures[i]) SPtr<ct::Texture>();

		if(mSampledTextureData[i].Texture.IsLoaded())
			textures[i] = mSampledTextureData[i].Texture->GetCore();
		else
			textures[i] = nullptr;
	}

	for(u32 i = 0; i < numStorageTextures; i++)
	{
		new(&loadStoreSurfaces[i]) TextureSurface();
		loadStoreSurfaces[i] = mStorageTextureData[i].Surface;

		new(&loadStoreTextures[i]) SPtr<ct::Texture>();

		if(mStorageTextureData[i].Texture.IsLoaded())
			loadStoreTextures[i] = mStorageTextureData[i].Texture->GetCore();
		else
			loadStoreTextures[i] = nullptr;
	}

	for(u32 i = 0; i < numBuffers; i++)
	{
		new(&buffers[i]) SPtr<ct::GpuBuffer>();

		if(mBuffers[i] != nullptr)
			buffers[i] = mBuffers[i]->GetCore();
		else
			buffers[i] = nullptr;
	}

	for(u32 i = 0; i < numSamplers; i++)
	{
		new(&samplers[i]) SPtr<ct::SamplerState>();

		if(mSamplerStates[i] != nullptr)
			samplers[i] = mSamplerStates[i]->GetCore();
		else
			samplers[i] = nullptr;
	}

	return CoreSyncData(data, totalSize);
}

void GpuParams::GetListenerResources(Vector<HResource>& resources)
{
	u32 numTextures = mParamInfo->GetResourceCount(GpuPipelineParamInfo::ParamType::Texture);
	u32 numStorageTextures = mParamInfo->GetResourceCount(GpuPipelineParamInfo::ParamType::LoadStoreTexture);

	for(u32 i = 0; i < numTextures; i++)
	{
		if(mSampledTextureData[i].Texture != nullptr)
			resources.push_back(mSampledTextureData[i].Texture);
	}

	for(u32 i = 0; i < numStorageTextures; i++)
	{
		if(mStorageTextureData[i].Texture != nullptr)
			resources.push_back(mStorageTextureData[i].Texture);
	}
}

namespace bs { namespace ct
{
GpuParams::GpuParams(const SPtr<GpuPipelineParamInfo>& paramInfo, GpuDeviceFlags deviceMask)
	: TGpuParams(paramInfo)
{
}

SPtr<GpuParams> GpuParams::GetThisPtrInternal() const
{
	return std::static_pointer_cast<GpuParams>(GetThisPtr());
}

void GpuParams::SyncToCore(const CoreSyncData& data)
{
	u32 numParamBlocks = mParamInfo->GetResourceCount(GpuPipelineParamInfo::ParamType::ParamBlock);
	u32 numTextures = mParamInfo->GetResourceCount(GpuPipelineParamInfo::ParamType::Texture);
	u32 numStorageTextures = mParamInfo->GetResourceCount(GpuPipelineParamInfo::ParamType::LoadStoreTexture);
	u32 numBuffers = mParamInfo->GetResourceCount(GpuPipelineParamInfo::ParamType::Buffer);
	u32 numSamplers = mParamInfo->GetResourceCount(GpuPipelineParamInfo::ParamType::SamplerState);

	u32 sampledSurfacesSize = numTextures * sizeof(TextureSurface);
	u32 loadStoreSurfacesSize = numStorageTextures * sizeof(TextureSurface);
	u32 paramBufferSize = numParamBlocks * sizeof(SPtr<GpuParamBlockBuffer>);
	u32 textureArraySize = numTextures * sizeof(SPtr<Texture>);
	u32 loadStoreTextureArraySize = numStorageTextures * sizeof(SPtr<Texture>);
	u32 bufferArraySize = numBuffers * sizeof(SPtr<GpuBuffer>);
	u32 samplerArraySize = numSamplers * sizeof(SPtr<SamplerState>);

	u32 totalSize = sampledSurfacesSize + loadStoreSurfacesSize + paramBufferSize + textureArraySize + loadStoreTextureArraySize + bufferArraySize + samplerArraySize;

	u32 sampledSurfacesOffset = 0;
	u32 loadStoreSurfaceOffset = sampledSurfacesOffset + sampledSurfacesSize;
	u32 paramBufferOffset = loadStoreSurfaceOffset + loadStoreSurfacesSize;
	u32 textureArrayOffset = paramBufferOffset + paramBufferSize;
	u32 loadStoreTextureArrayOffset = textureArrayOffset + textureArraySize;
	u32 bufferArrayOffset = loadStoreTextureArrayOffset + loadStoreTextureArraySize;
	u32 samplerArrayOffset = bufferArrayOffset + bufferArraySize;

	B3D_ASSERT(data.GetBufferSize() == totalSize);

	u8* dataPtr = data.GetBuffer();

	TextureSurface* sampledSurfaces = (TextureSurface*)(dataPtr + sampledSurfacesOffset);
	TextureSurface* loadStoreSurfaces = (TextureSurface*)(dataPtr + loadStoreSurfaceOffset);
	SPtr<GpuParamBlockBuffer>* paramBuffers = (SPtr<GpuParamBlockBuffer>*)(dataPtr + paramBufferOffset);
	SPtr<Texture>* textures = (SPtr<Texture>*)(dataPtr + textureArrayOffset);
	SPtr<Texture>* loadStoreTextures = (SPtr<Texture>*)(dataPtr + loadStoreTextureArrayOffset);
	SPtr<GpuBuffer>* buffers = (SPtr<GpuBuffer>*)(dataPtr + bufferArrayOffset);
	SPtr<SamplerState>* samplers = (SPtr<SamplerState>*)(dataPtr + samplerArrayOffset);

	// Copy & destruct
	for(u32 i = 0; i < numParamBlocks; i++)
	{
		mParamBlockBuffers[i] = paramBuffers[i];
		paramBuffers[i].~SPtr<GpuParamBlockBuffer>();
	}

	for(u32 i = 0; i < numTextures; i++)
	{
		mSampledTextureData[i].Surface = sampledSurfaces[i];
		loadStoreSurfaces[i].~TextureSurface();

		mSampledTextureData[i].Texture = textures[i];
		textures[i].~SPtr<Texture>();
	}

	for(u32 i = 0; i < numStorageTextures; i++)
	{
		mStorageTextureData[i].Surface = loadStoreSurfaces[i];
		loadStoreSurfaces[i].~TextureSurface();

		mStorageTextureData[i].Texture = loadStoreTextures[i];
		loadStoreTextures[i].~SPtr<Texture>();
	}

	for(u32 i = 0; i < numBuffers; i++)
	{
		mBuffers[i] = buffers[i];
		buffers[i].~SPtr<GpuBuffer>();
	}

	for(u32 i = 0; i < numSamplers; i++)
	{
		mSamplerStates[i] = samplers[i];
		samplers[i].~SPtr<SamplerState>();
	}
}

SPtr<GpuParams> GpuParams::Create(const SPtr<GraphicsPipelineState>& pipelineState, GpuDeviceFlags deviceMask)
{
	return HardwareBufferManager::Instance().CreateGpuParams(pipelineState->GetParamInfo(), deviceMask);
}

SPtr<GpuParams> GpuParams::Create(const SPtr<ComputePipelineState>& pipelineState, GpuDeviceFlags deviceMask)
{
	return HardwareBufferManager::Instance().CreateGpuParams(pipelineState->GetParamInfo(), deviceMask);
}

SPtr<GpuParams> GpuParams::Create(const SPtr<GpuPipelineParamInfo>& paramInfo, GpuDeviceFlags deviceMask)
{
	return HardwareBufferManager::Instance().CreateGpuParams(paramInfo, deviceMask);
}
}}
