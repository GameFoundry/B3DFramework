//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "RenderAPI/BsGpuParameters.h"

#include "BsCoreApplication.h"
#include "BsGpuDevice.h"
#include "RenderAPI/BsGpuProgramParameterDescription.h"
#include "RenderAPI/BsGpuBuffer.h"
#include "RenderAPI/BsGpuPipelineParameterLayout.h"
#include "RenderAPI/BsGpuPipelineState.h"
#include "Math/BsVector2.h"
#include "Image/BsTexture.h"
#include "RenderAPI/BsSamplerState.h"
#include "Debug/BsDebug.h"
#include "Math/BsVector3I.h"
#include "Math/BsVector4I.h"
#include "Math/BsMatrixNxM.h"

using namespace bs;

const TextureSurface TextureSurface::kComplete = TextureSurface(0, 0, 0, 0);

GpuParamsBase::GpuParamsBase(const SPtr<GpuPipelineParameterLayout>& parameterLayout)
	: mParameterLayout(parameterLayout)
{}

SPtr<GpuProgramParameterDescription> GpuParamsBase::GetParameterInformation(GpuProgramType type) const
{
	return mParameterLayout->GetParameterDescriptionForProgram(type);
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

bool GpuParamsBase::HasSampledTexture(GpuProgramType type, const String& name) const
{
	const SPtr<GpuProgramParameterDescription>& paramDesc = mParameterLayout->GetParameterDescriptionForProgram(type);
	if(paramDesc == nullptr)
		return false;

	auto paramIter = paramDesc->Textures.find(name);
	if(paramIter != paramDesc->Textures.end())
		return true;

	return false;
}

bool GpuParamsBase::HasStorageBuffer(GpuProgramType type, const String& name) const
{
	const SPtr<GpuProgramParameterDescription>& paramDesc = mParameterLayout->GetParameterDescriptionForProgram(type);
	if(paramDesc == nullptr)
		return false;

	auto paramIter = paramDesc->Buffers.find(name);
	if(paramIter != paramDesc->Buffers.end())
		return true;

	return false;
}

bool GpuParamsBase::HasStorageTexture(GpuProgramType type, const String& name) const
{
	const SPtr<GpuProgramParameterDescription>& paramDesc = mParameterLayout->GetParameterDescriptionForProgram(type);
	if(paramDesc == nullptr)
		return false;

	auto paramIter = paramDesc->StorageTextures.find(name);
	if(paramIter != paramDesc->StorageTextures.end())
		return true;

	return false;
}

bool GpuParamsBase::HasSamplerState(GpuProgramType type, const String& name) const
{
	const SPtr<GpuProgramParameterDescription>& paramDesc = mParameterLayout->GetParameterDescriptionForProgram(type);
	if(paramDesc == nullptr)
		return false;

	auto paramIter = paramDesc->Samplers.find(name);
	if(paramIter != paramDesc->Samplers.end())
		return true;

	return false;
}

bool GpuParamsBase::HasUniformBuffer(GpuProgramType type, const String& name) const
{
	const SPtr<GpuProgramParameterDescription>& gpuParameterInformation = mParameterLayout->GetParameterDescriptionForProgram(type);
	if(gpuParameterInformation == nullptr)
		return false;

	return gpuParameterInformation->DataParameterBlocks.find(name) != gpuParameterInformation->DataParameterBlocks.end();
}

bool GpuParamsBase::HasUniformBuffer(const String& name) const
{
	for (u32 gpuProgramTypeIndex = 0; gpuProgramTypeIndex < GPT_COUNT; gpuProgramTypeIndex++)
	{
		const SPtr<GpuProgramParameterDescription>& gpuParameterInformation = mParameterLayout->GetParameterDescriptionForProgram((GpuProgramType)gpuProgramTypeIndex);
		if (gpuParameterInformation == nullptr)
			continue;

		auto found = gpuParameterInformation->DataParameterBlocks.find(name);
		if (found != gpuParameterInformation->DataParameterBlocks.end())
			return true;
	}

	return false;
}

GpuDataParameterInformation* GpuParamsBase::GetDataParameterInformation(GpuProgramType type, const String& name) const
{
	const SPtr<GpuProgramParameterDescription>& paramDesc = mParameterLayout->GetParameterDescriptionForProgram(type);
	if(paramDesc == nullptr)
		return nullptr;

	auto paramIter = paramDesc->DataParameters.find(name);
	if(paramIter != paramDesc->DataParameters.end())
		return &paramIter->second;

	return nullptr;
}

GpuDataParameterBlockInformation* GpuParamsBase::GetParameterBlockDesc(GpuProgramType type, const String& name) const
{
	const SPtr<GpuProgramParameterDescription>& paramDesc = mParameterLayout->GetParameterDescriptionForProgram(type);
	if(paramDesc == nullptr)
		return nullptr;

	auto paramBlockIter = paramDesc->DataParameterBlocks.find(name);
	if(paramBlockIter != paramDesc->DataParameterBlocks.end())
		return &paramBlockIter->second;

	return nullptr;
}

template <bool Core>
TGpuParams<Core>::TGpuParams(const SPtr<GpuPipelineParameterLayout>& parameterLayout)
	: GpuParamsBase(parameterLayout)
{
	const u32 uniformBufferCount = mParameterLayout->GetResourceCount(GpuPipelineParameterLayout::GpuParameterType::UniformBuffer);
	const u32 sampledTextureCount = mParameterLayout->GetResourceCount(GpuPipelineParameterLayout::GpuParameterType::SampledTexture);
	const u32 storageTextureCount = mParameterLayout->GetResourceCount(GpuPipelineParameterLayout::GpuParameterType::StorageTexture);
	const u32 storageBufferCount = mParameterLayout->GetResourceCount(GpuPipelineParameterLayout::GpuParameterType::StorageBuffer);
	const u32 samplerCount = mParameterLayout->GetResourceCount(GpuPipelineParameterLayout::GpuParameterType::Sampler);

	const u32 uniformBufferEntrySize = Math::RoundToMultiple((u32)sizeof(UniformBufferData), 16u);
	const u32 textureEntrySize = Math::RoundToMultiple((u32)sizeof(TextureData), 16u);
	const u32 storageBufferEntrySize = Math::RoundToMultiple((u32)sizeof(StorageBufferData), 16u);
	const u32 samplerStateEntrySize = Math::RoundToMultiple((u32)sizeof(SPtr<SamplerState>), 16u);

	const u32 uniformBufferBufferSize = uniformBufferEntrySize * uniformBufferCount;
	const u32 sampledTexturesBufferSize = textureEntrySize * sampledTextureCount;
	const u32 storageTexturesBufferSize = textureEntrySize * storageTextureCount;
	const u32 storageBufferBufferSize = storageBufferEntrySize * storageBufferCount;
	const u32 samplerStatesBufferSize = samplerStateEntrySize * samplerCount;

	const u32 totalSize = uniformBufferBufferSize + sampledTexturesBufferSize + storageTexturesBufferSize + storageBufferBufferSize + samplerStatesBufferSize;

	u8* data = (u8*)B3DAllocate(totalSize);
	mUniformBufferData = (UniformBufferData*)data;
	for(u32 i = 0; i < uniformBufferCount; i++)
		new(&mUniformBufferData[i]) UniformBufferData();

	data += uniformBufferBufferSize;
	mSampledTextureData = (TextureData*)data;
	for(u32 i = 0; i < sampledTextureCount; i++)
	{
		new(&mSampledTextureData[i].Texture) TextureType();
		new(&mSampledTextureData[i].Surface) TextureSurface(0, 0, 0, 0);
	}

	data += sampledTexturesBufferSize;
	mStorageTextureData = (TextureData*)data;
	for(u32 i = 0; i < storageTextureCount; i++)
	{
		new(&mStorageTextureData[i].Texture) TextureType();
		new(&mStorageTextureData[i].Surface) TextureSurface(0, 0, 0, 0);
	}

	data += storageTexturesBufferSize;
	mStorageBufferData = (StorageBufferData*)data;
	for(u32 i = 0; i < storageBufferCount; i++)
		new(&mStorageBufferData[i]) StorageBufferData();

	data += storageBufferBufferSize;
	mSamplerStates = (SPtr<SamplerState>*)data;
	for(u32 i = 0; i < samplerCount; i++)
		new(&mSamplerStates[i]) SPtr<SamplerState>();

	data += samplerStatesBufferSize;
}

template <bool Core>
TGpuParams<Core>::~TGpuParams()
{
	const u32 uniformBufferCount = mParameterLayout->GetResourceCount(GpuPipelineParameterLayout::GpuParameterType::UniformBuffer);
	const u32 sampledTextureCount = mParameterLayout->GetResourceCount(GpuPipelineParameterLayout::GpuParameterType::SampledTexture);
	const u32 storageTextureCount = mParameterLayout->GetResourceCount(GpuPipelineParameterLayout::GpuParameterType::StorageTexture);
	const u32 storageBufferCount = mParameterLayout->GetResourceCount(GpuPipelineParameterLayout::GpuParameterType::StorageBuffer);
	const u32 samplerCount = mParameterLayout->GetResourceCount(GpuPipelineParameterLayout::GpuParameterType::Sampler);

	for(u32 i = 0; i < uniformBufferCount; i++)
		mUniformBufferData[i].~UniformBufferData();

	for(u32 i = 0; i < sampledTextureCount; i++)
	{
		mSampledTextureData[i].Texture.~TextureType();
		mSampledTextureData[i].Surface.~TextureSurface();
	}

	for(u32 i = 0; i < storageTextureCount; i++)
	{
		mStorageTextureData[i].Texture.~TextureType();
		mStorageTextureData[i].Surface.~TextureSurface();
	}

	for(u32 i = 0; i < storageBufferCount; i++)
		mStorageBufferData[i].~StorageBufferData();

	for(u32 i = 0; i < samplerCount; i++)
		mSamplerStates[i].~SPtr<SamplerState>();

	// Everything is allocated in a single block, so it's enough to free the first element
	B3DFree(mUniformBufferData);
}

template <bool Core>
bool TGpuParams<Core>::SetUniformBuffer(u32 set, u32 slot, const UniformBufferType& paramBlockBuffer, u32 arrayIndex, u32 offset)
{
	const u32 sequentialResourceIndex = mParameterLayout->GetSequentialResourceIndex(GpuPipelineParameterLayout::GpuParameterType::UniformBuffer, set, slot, arrayIndex);
	if (sequentialResourceIndex == ~0u)
	{
		B3D_LOG(Warning, RenderBackend, "Unable to assign parameter. Cannot find parameter block with the set/slot combination: {0}/{1}", set, slot);
		return false;
	}

	mUniformBufferData[sequentialResourceIndex].Buffer = paramBlockBuffer;
	mUniformBufferData[sequentialResourceIndex].Offset = offset;

	MarkCoreDirtyInternal();
	return true;
}

template <bool Core>
bool TGpuParams<Core>::SetUniformBuffer(GpuProgramType type, const String& name, const UniformBufferType& paramBlockBuffer, u32 arrayIndex, u32 offset)
{
	const SPtr<GpuProgramParameterDescription>& parameterInformation = mParameterLayout->GetParameterDescriptionForProgram(type);
	if(parameterInformation == nullptr)
	{
		B3D_LOG(Warning, RenderBackend, "Unable to assign parameter. Cannot find parameter block with name: {0}", name);
		return false;
	}

	auto iterFind = parameterInformation->DataParameterBlocks.find(name);
	if(iterFind == parameterInformation->DataParameterBlocks.end())
	{
		B3D_LOG(Warning, RenderBackend, "Unable to assign parameter. Cannot find parameter block with name: {0}", name);
		return false;
	}

	return SetUniformBuffer(iterFind->second.Set, iterFind->second.Slot, paramBlockBuffer, arrayIndex, offset);
}

template<bool Core>
bool TGpuParams<Core>::TrySetUniformBuffer(GpuProgramType type, const String& name, const UniformBufferType& parameterBlockBuffer, u32 arrayIndex, u32 offset)
{
	if (!HasUniformBuffer(type, name))
		return false;

	return SetUniformBuffer(type, name, parameterBlockBuffer, arrayIndex, offset);
}

template<bool Core>
bool TGpuParams<Core>::TrySetUniformBuffer(const String& name, const UniformBufferType& paramBlockBuffer, u32 arrayIndex, u32 offset)
{
	if (!HasUniformBuffer(name))
		return false;

	return SetUniformBuffer(name, paramBlockBuffer, arrayIndex, offset);
}

template <bool Core>
bool TGpuParams<Core>::SetUniformBuffer(const String& name, const UniformBufferType& paramBlockBuffer, u32 arrayIndex, u32 offset)
{
	bool foundMatchingParameter = false;
	bool isParameterBound = false;
	for(u32 i = 0; i < 6; i++)
	{
		const SPtr<GpuProgramParameterDescription>& paramDescs = mParameterLayout->GetParameterDescriptionForProgram((GpuProgramType)i);
		if(paramDescs == nullptr)
			continue;

		auto iterFind = paramDescs->DataParameterBlocks.find(name);
		if(iterFind == paramDescs->DataParameterBlocks.end())
			continue;

		if (SetUniformBuffer(iterFind->second.Set, iterFind->second.Slot, paramBlockBuffer, arrayIndex, offset))
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
void TGpuParams<Core>::GetSampledTextureParameter(GpuProgramType type, const String& name, TGpuParameterSampledTexture<Core>& output) const
{
	if(!TryGetSampledTextureParameter(type, name, output))
		B3D_LOG(Warning, RenderBackend, "Cannot find texture parameter with the name: '{0}'", name);
}

template <bool Core>
void TGpuParams<Core>::GetStorageTextureParameter(GpuProgramType type, const String& name, TGpuParameterStorageTexture<Core>& output) const
{
	if(!TryGetStorageTextureParameter(type, name, output))
		B3D_LOG(Warning, RenderBackend, "Cannot find storage texture parameter with the name: '{0}'", name);
}

template <bool Core>
void TGpuParams<Core>::GetStorageBufferParameter(GpuProgramType type, const String& name, TGpuParameterBuffer<Core>& output) const
{
	if(!TryGetStorageBufferParameter(type, name, output))
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
	const SPtr<GpuProgramParameterDescription>& paramDescs = mParameterLayout->GetParameterDescriptionForProgram(type);
	if (paramDescs == nullptr)
	{
		output = TGpuParameterPrimitive<T, Core>(nullptr, nullptr);
		return false;
	}

	auto iterFind = paramDescs->DataParameters.find(name);
	if (iterFind == paramDescs->DataParameters.end())
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
	const SPtr<GpuProgramParameterDescription>& paramDescs = mParameterLayout->GetParameterDescriptionForProgram(type);
	if (paramDescs == nullptr)
	{
		output = TGpuParameterStruct<Core>(nullptr, nullptr);
		return false;
	}

	auto iterFind = paramDescs->DataParameters.find(name);
	if (iterFind == paramDescs->DataParameters.end() || iterFind->second.Type != GPDT_STRUCT)
	{
		output = TGpuParameterStruct<Core>(nullptr, nullptr);
		return false;
	}

	output = TGpuParameterStruct<Core>(&iterFind->second, GetThisPtrInternal());
	return true;
}

template <bool Core>
bool TGpuParams<Core>::TryGetSampledTextureParameter(GpuProgramType type, const String& name, TGpuParameterSampledTexture<Core>& output) const
{
	const SPtr<GpuProgramParameterDescription>& paramDescs = mParameterLayout->GetParameterDescriptionForProgram(type);
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
	const SPtr<GpuProgramParameterDescription>& paramDescs = mParameterLayout->GetParameterDescriptionForProgram(type);
	if (paramDescs == nullptr)
	{
		output = TGpuParameterStorageTexture<Core>(nullptr, nullptr);
		return false;
	}

	auto iterFind = paramDescs->StorageTextures.find(name);
	if (iterFind == paramDescs->StorageTextures.end())
	{
		output = TGpuParameterStorageTexture<Core>(nullptr, nullptr);
		return false;
	}

	output = TGpuParameterStorageTexture<Core>(&iterFind->second, GetThisPtrInternal());
	return true;
}

template <bool Core>
bool TGpuParams<Core>::TryGetStorageBufferParameter(GpuProgramType type, const String& name, TGpuParameterBuffer<Core>& output) const
{
	const SPtr<GpuProgramParameterDescription>& paramDescs = mParameterLayout->GetParameterDescriptionForProgram(type);
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
	const SPtr<GpuProgramParameterDescription>& paramDescs = mParameterLayout->GetParameterDescriptionForProgram(type);
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
typename TGpuParams<Core>::UniformBufferType TGpuParams<Core>::GetUniformBuffer(u32 set, u32 slot, u32 arrayIndex) const
{
	const u32 sequentialResourceIndex = mParameterLayout->GetSequentialResourceIndex(GpuPipelineParameterLayout::GpuParameterType::UniformBuffer, set, slot, arrayIndex);
	if(sequentialResourceIndex == ~0u)
		return nullptr;

	return mUniformBufferData[sequentialResourceIndex].Buffer;
}

template <bool Core>
typename TGpuParams<Core>::TextureType TGpuParams<Core>::GetSampledTexture(u32 set, u32 slot, u32 arrayIndex) const
{
	const u32 sequentialResourceIndex = mParameterLayout->GetSequentialResourceIndex(GpuPipelineParameterLayout::GpuParameterType::SampledTexture, set, slot, arrayIndex);
	if(sequentialResourceIndex == ~0u)
		return TGpuParams<Core>::TextureType();

	return mSampledTextureData[sequentialResourceIndex].Texture;
}

template <bool Core>
typename TGpuParams<Core>::TextureType TGpuParams<Core>::GetStorageTexture(u32 set, u32 slot, u32 arrayIndex) const
{
	const u32 sequentialResourceIndex = mParameterLayout->GetSequentialResourceIndex(GpuPipelineParameterLayout::GpuParameterType::StorageTexture, set, slot, arrayIndex);
	if(sequentialResourceIndex == ~0u)
		return TGpuParams<Core>::TextureType();

	return mStorageTextureData[sequentialResourceIndex].Texture;
}

template <bool Core>
typename TGpuParams<Core>::BufferType TGpuParams<Core>::GetStorageBuffer(u32 set, u32 slot, u32 arrayIndex) const
{
	const u32 sequentialResourceIndex = mParameterLayout->GetSequentialResourceIndex(GpuPipelineParameterLayout::GpuParameterType::StorageBuffer, set, slot, arrayIndex);
	if(sequentialResourceIndex == ~0u)
		return nullptr;

	return mStorageBufferData[sequentialResourceIndex].Buffer;
}

template <bool Core>
SPtr<SamplerState> TGpuParams<Core>::GetSamplerState(u32 set, u32 slot, u32 arrayIndex) const
{
	const u32 sequentialArrayIndex = mParameterLayout->GetSequentialResourceIndex(GpuPipelineParameterLayout::GpuParameterType::Sampler, set, slot, arrayIndex);
	if(sequentialArrayIndex == ~0u)
		return nullptr;

	return mSamplerStates[sequentialArrayIndex];
}

template <bool Core>
const TextureSurface& TGpuParams<Core>::GetTextureSurface(u32 set, u32 slot, u32 arrayIndex) const
{
	static TextureSurface emptySurface;

	const u32 sequentialArrayIndex = mParameterLayout->GetSequentialResourceIndex(GpuPipelineParameterLayout::GpuParameterType::SampledTexture, set, slot, arrayIndex);
	if(sequentialArrayIndex == ~0u)
		return emptySurface;

	return mSampledTextureData[sequentialArrayIndex].Surface;
}

template <bool Core>
const TextureSurface& TGpuParams<Core>::GetStorageTextureSurface(u32 set, u32 slot, u32 arrayIndex) const
{
	static TextureSurface emptySurface;

	const u32 sequentialArrayIndex = mParameterLayout->GetSequentialResourceIndex(GpuPipelineParameterLayout::GpuParameterType::StorageTexture, set, slot, arrayIndex);
	if(sequentialArrayIndex == ~0u)
		return emptySurface;

	return mStorageTextureData[sequentialArrayIndex].Surface;
}

template <bool Core>
bool TGpuParams<Core>::SetSampledTexture(u32 set, u32 slot, const TextureType& texture, const TextureSurface& surface, u32 arrayIndex)
{
	const u32 sequentialArrayIndex = mParameterLayout->GetSequentialResourceIndex(GpuPipelineParameterLayout::GpuParameterType::SampledTexture, set, slot, arrayIndex);
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
	const u32 sequentialArrayIndex = mParameterLayout->GetSequentialResourceIndex(GpuPipelineParameterLayout::GpuParameterType::StorageTexture, set, slot, arrayIndex);
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
bool TGpuParams<Core>::SetStorageBuffer(u32 set, u32 slot, const BufferType& buffer, u32 arrayIndex, GpuStorageBufferViewInformation view)
{
	const u32 sequentialArrayIndex = mParameterLayout->GetSequentialResourceIndex(GpuPipelineParameterLayout::GpuParameterType::StorageBuffer, set, slot, arrayIndex);
	if (sequentialArrayIndex == ~0u)
	{
		B3D_LOG(Warning, RenderBackend, "Unable to assign parameter. Cannot find buffer parameter with the set/slot combination: {0}/{1}", set, slot);
		return false;
	}

	mStorageBufferData[sequentialArrayIndex].Buffer = buffer;
	mStorageBufferData[sequentialArrayIndex].View = view;

	MarkResourcesDirtyInternal();
	MarkCoreDirtyInternal();

	return true;
}

template <bool Core>
bool TGpuParams<Core>::SetSamplerState(u32 set, u32 slot, const SPtr<SamplerState>& sampler, u32 arrayIndex)
{
	const u32 sequentialArrayIndex = mParameterLayout->GetSequentialResourceIndex(GpuPipelineParameterLayout::GpuParameterType::Sampler, set, slot, arrayIndex);
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

const GpuDataParameterTypeInformationLookup GpuParameters::kParamSizes;

GpuParameters::GpuParameters(const SPtr<GpuPipelineParameterLayout>& parameterLayout)
	: TGpuParams(parameterLayout)
{
}

SPtr<GpuParameters> GpuParameters::GetThisPtrInternal() const
{
	return std::static_pointer_cast<GpuParameters>(GetShared());
}

SPtr<ct::GpuParameters> GpuParameters::GetCore() const
{
	return std::static_pointer_cast<ct::GpuParameters>(mCoreSpecific);
}

SPtr<ct::CoreObject> GpuParameters::CreateCore() const
{
	const SPtr<GpuDevice>& gpuDevice = GetCoreApplication().GetPrimaryGpuDevice();
	if(!gpuDevice)
		return nullptr;

	SPtr<GpuPipelineParameterLayout> parameterLayout = std::static_pointer_cast<GpuPipelineParameterLayout>(mParameterLayout);
	return gpuDevice->CreateGpuParameters(parameterLayout, true);
}

void GpuParameters::MarkCoreDirtyInternal()
{
	MarkCoreDirty();
}

void GpuParameters::MarkResourcesDirtyInternal()
{
	MarkListenerResourcesDirty();
}

SPtr<GpuParameters> GpuParameters::Create(const SPtr<GpuGraphicsPipelineState>& pipelineState)
{
	return Create(pipelineState->GetParameterLayout());
}

SPtr<GpuParameters> GpuParameters::Create(const SPtr<GpuComputePipelineState>& pipelineState)
{
	return Create(pipelineState->GetParameterLayout());
}

SPtr<GpuParameters> GpuParameters::Create(const SPtr<GpuPipelineParameterLayout>& parameterLayout)
{
	GpuParameters* const output = new(B3DAllocate<GpuParameters>()) GpuParameters(parameterLayout);
	SPtr<GpuParameters> shared = B3DMakeCoreFromExisting<GpuParameters>(output);
	shared->SetShared(shared);
	shared->Initialize();

	return shared;
}

CoreSyncData GpuParameters::SyncToCore(FrameAlloc* allocator)
{
	const u32 uniformBufferCount = mParameterLayout->GetResourceCount(GpuPipelineParameterLayout::GpuParameterType::UniformBuffer);
	const u32 sampledTextureCount = mParameterLayout->GetResourceCount(GpuPipelineParameterLayout::GpuParameterType::SampledTexture);
	const u32 storageTextureCount = mParameterLayout->GetResourceCount(GpuPipelineParameterLayout::GpuParameterType::StorageTexture);
	const u32 storageBufferCount = mParameterLayout->GetResourceCount(GpuPipelineParameterLayout::GpuParameterType::StorageBuffer);
	const u32 samplerCount = mParameterLayout->GetResourceCount(GpuPipelineParameterLayout::GpuParameterType::Sampler);

	const u32 sampledTextureSurfacesArraySize = sampledTextureCount * sizeof(TextureSurface);
	const u32 storageTextureSurfacesArraySize = storageTextureCount * sizeof(TextureSurface);
	const u32 uniformBufferArraySize = uniformBufferCount * sizeof(SPtr<ct::GpuBuffer>);
	const u32 uniformBufferOffsetsArraySize = uniformBufferCount * sizeof(u32);
	const u32 sampledTextureArraySize = sampledTextureCount * sizeof(SPtr<ct::Texture>);
	const u32 storageTextureArraySize = storageTextureCount * sizeof(SPtr<ct::Texture>);
	const u32 storageBufferArraySize = storageBufferCount * sizeof(SPtr<ct::GpuBuffer>);
	const u32 storageBufferOffsetArraySize = storageBufferCount * sizeof(u32);
	const u32 storageBufferFormatArraySize = storageBufferCount * sizeof(GpuBufferFormat);
	const u32 samplerArraySize = samplerCount * sizeof(SPtr<SamplerState>);

	const u32 totalSize = sampledTextureSurfacesArraySize + storageTextureSurfacesArraySize + uniformBufferArraySize + uniformBufferOffsetsArraySize + sampledTextureArraySize + storageTextureArraySize + storageBufferArraySize + storageBufferOffsetArraySize + storageBufferFormatArraySize + samplerArraySize;

	const u32 sampledTextureSurfaceArrayOffset = 0;
	const u32 storageTextureSurfaceArrayOffset = sampledTextureSurfaceArrayOffset + sampledTextureSurfacesArraySize;
	const u32 uniformBufferArrayOffset = storageTextureSurfaceArrayOffset + storageTextureSurfacesArraySize;
	const u32 uniformBufferOffsetArrayOffset = uniformBufferArrayOffset + uniformBufferArraySize;
	const u32 sampledTextureArrayOffset = uniformBufferOffsetArrayOffset + uniformBufferOffsetsArraySize;
	const u32 storageTextureArrayOffset = sampledTextureArrayOffset + sampledTextureArraySize;
	const u32 storageBufferArrayOffset = storageTextureArrayOffset + storageTextureArraySize;
	const u32 storageBufferOffsetArrayOffset = storageBufferArrayOffset + storageBufferArraySize;
	const u32 storageBufferFormatArrayOffset = storageBufferOffsetArrayOffset + storageBufferOffsetArraySize;
	const u32 samplerArrayOffset = storageBufferFormatArrayOffset + storageBufferFormatArraySize;

	u8* data = allocator->Alloc(totalSize);

	TextureSurface* sampledTextureSurfaces = (TextureSurface*)(data + sampledTextureSurfaceArrayOffset);
	TextureSurface* storageTextureSurfaces = (TextureSurface*)(data + storageTextureSurfaceArrayOffset);
	SPtr<ct::GpuBuffer>* uniformBuffers = (SPtr<ct::GpuBuffer>*)(data + uniformBufferArrayOffset);
	u32* uniformBufferOffsets = (u32*)(data + uniformBufferOffsetArrayOffset);
	SPtr<ct::Texture>* sampledTextures = (SPtr<ct::Texture>*)(data + sampledTextureArrayOffset);
	SPtr<ct::Texture>* storageTextures = (SPtr<ct::Texture>*)(data + storageTextureArrayOffset);
	SPtr<ct::GpuBuffer>* storageBuffers = (SPtr<ct::GpuBuffer>*)(data + storageBufferArrayOffset);
	u32* storageBufferOffsets = (u32*)(data + storageBufferOffsetArrayOffset);
	GpuBufferFormat* storageBufferFormats = (GpuBufferFormat*)(data + storageBufferFormatArrayOffset);
	SPtr<SamplerState>* samplers = (SPtr<SamplerState>*)(data + samplerArrayOffset);

	// Construct & copy
	for(u32 i = 0; i < uniformBufferCount; i++)
	{
		uniformBufferOffsets[i] = mUniformBufferData->Offset;

		new(&uniformBuffers[i]) SPtr<ct::GpuBuffer>();

		if(mUniformBufferData[i].Buffer != nullptr)
			uniformBuffers[i] = mUniformBufferData[i].Buffer->GetCore();
		else
			uniformBuffers[i] = nullptr;
	}

	for(u32 i = 0; i < sampledTextureCount; i++)
	{
		new(&sampledTextureSurfaces[i]) TextureSurface();
		sampledTextureSurfaces[i] = mSampledTextureData[i].Surface;

		new(&sampledTextures[i]) SPtr<ct::Texture>();

		if(mSampledTextureData[i].Texture.IsLoaded())
			sampledTextures[i] = mSampledTextureData[i].Texture->GetCore();
		else
			sampledTextures[i] = nullptr;
	}

	for(u32 i = 0; i < storageTextureCount; i++)
	{
		new(&storageTextureSurfaces[i]) TextureSurface();
		storageTextureSurfaces[i] = mStorageTextureData[i].Surface;

		new(&storageTextures[i]) SPtr<ct::Texture>();

		if(mStorageTextureData[i].Texture.IsLoaded())
			storageTextures[i] = mStorageTextureData[i].Texture->GetCore();
		else
			storageTextures[i] = nullptr;
	}

	for(u32 i = 0; i < storageBufferCount; i++)
	{
		storageBufferOffsets[i] = mStorageBufferData->View.Offset;
		storageBufferFormats[i] = mStorageBufferData->View.Format;

		new(&storageBuffers[i]) SPtr<ct::GpuBuffer>();

		if(mStorageBufferData[i].Buffer != nullptr)
			storageBuffers[i] = mStorageBufferData[i].Buffer->GetCore();
		else
			storageBuffers[i] = nullptr;
	}

	for(u32 i = 0; i < samplerCount; i++)
	{
		new(&samplers[i]) SPtr<SamplerState>();
		samplers[i] = mSamplerStates[i];
	}

	return CoreSyncData(data, totalSize);
}

void GpuParameters::GetListenerResources(Vector<HResource>& resources)
{
	u32 numTextures = mParameterLayout->GetResourceCount(GpuPipelineParameterLayout::GpuParameterType::SampledTexture);
	u32 numStorageTextures = mParameterLayout->GetResourceCount(GpuPipelineParameterLayout::GpuParameterType::StorageTexture);

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
GpuParameters::GpuParameters(const SPtr<GpuPipelineParameterLayout>& parameterLayout)
	: TGpuParams(parameterLayout)
{
}

SPtr<GpuParameters> GpuParameters::GetThisPtrInternal() const
{
	return std::static_pointer_cast<GpuParameters>(GetShared());
}

void GpuParameters::SyncToCore(const CoreSyncData& data, FrameAlloc& allocator)
{
	const u32 uniformBufferCount = mParameterLayout->GetResourceCount(GpuPipelineParameterLayout::GpuParameterType::UniformBuffer);
	const u32 sampledTextureCount = mParameterLayout->GetResourceCount(GpuPipelineParameterLayout::GpuParameterType::SampledTexture);
	const u32 storageTextureCount = mParameterLayout->GetResourceCount(GpuPipelineParameterLayout::GpuParameterType::StorageTexture);
	const u32 storageBufferCount = mParameterLayout->GetResourceCount(GpuPipelineParameterLayout::GpuParameterType::StorageBuffer);
	const u32 samplerCount = mParameterLayout->GetResourceCount(GpuPipelineParameterLayout::GpuParameterType::Sampler);

	const u32 sampledTextureSurfacesArraySize = sampledTextureCount * sizeof(TextureSurface);
	const u32 storageTextureSurfacesArraySize = storageTextureCount * sizeof(TextureSurface);
	const u32 uniformBufferArraySize = uniformBufferCount * sizeof(SPtr<GpuBuffer>);
	const u32 uniformBufferOffsetsArraySize = uniformBufferCount * sizeof(u32);
	const u32 sampledTextureArraySize = sampledTextureCount * sizeof(SPtr<Texture>);
	const u32 storageTextureArraySize = storageTextureCount * sizeof(SPtr<Texture>);
	const u32 storageBufferArraySize = storageBufferCount * sizeof(SPtr<GpuBuffer>);
	const u32 storageBufferOffsetArraySize = storageBufferCount * sizeof(u32);
	const u32 storageBufferFormatArraySize = storageBufferCount * sizeof(GpuBufferFormat);
	const u32 samplerArraySize = samplerCount * sizeof(SPtr<SamplerState>);

	const u32 totalSize = sampledTextureSurfacesArraySize + storageTextureSurfacesArraySize + uniformBufferArraySize + uniformBufferOffsetsArraySize + sampledTextureArraySize + storageTextureArraySize + storageBufferArraySize + storageBufferOffsetArraySize + storageBufferFormatArraySize + samplerArraySize;

	const u32 sampledTextureSurfacesArrayOffset = 0;
	const u32 storageTextureSurfacesArrayOffset = sampledTextureSurfacesArrayOffset + sampledTextureSurfacesArraySize;
	const u32 uniformBufferArrayOffset = storageTextureSurfacesArrayOffset + storageTextureSurfacesArraySize;
	const u32 uniformBufferOffsetArrayOffset = uniformBufferArrayOffset + uniformBufferArraySize;
	const u32 sampledTextureArrayOffset = uniformBufferOffsetArrayOffset + uniformBufferOffsetsArraySize;
	const u32 storageTextureArrayOffset = sampledTextureArrayOffset + sampledTextureArraySize;
	const u32 storageBufferArrayOffset = storageTextureArrayOffset + storageTextureArraySize;
	const u32 storageBufferOffsetArrayOffset = storageBufferArrayOffset + storageBufferArraySize;
	const u32 storageBufferFormatArrayOffset = storageBufferOffsetArrayOffset + storageBufferOffsetArraySize;
	const u32 samplerArrayOffset = storageBufferFormatArrayOffset + storageBufferFormatArraySize;

	B3D_ASSERT(data.GetBufferSize() == totalSize);

	u8* dataPtr = data.GetBuffer();

	TextureSurface* sampledTextureSurfaces = (TextureSurface*)(dataPtr + sampledTextureSurfacesArrayOffset);
	TextureSurface* storageSurfaces = (TextureSurface*)(dataPtr + storageTextureSurfacesArrayOffset);
	SPtr<GpuBuffer>* uniformBuffers = (SPtr<GpuBuffer>*)(dataPtr + uniformBufferArrayOffset);
	u32* uniformBufferOffsets = (u32*)(dataPtr + uniformBufferOffsetArrayOffset);
	SPtr<Texture>* sampledTextures = (SPtr<Texture>*)(dataPtr + sampledTextureArrayOffset);
	SPtr<Texture>* storageTextures = (SPtr<Texture>*)(dataPtr + storageTextureArrayOffset);
	SPtr<GpuBuffer>* storageBuffers = (SPtr<GpuBuffer>*)(dataPtr + storageBufferArrayOffset);
	u32* storageBufferOffsets = (u32*)(dataPtr + storageBufferOffsetArrayOffset);
	GpuBufferFormat* storageBufferFormats = (GpuBufferFormat*)(dataPtr + storageBufferFormatArrayOffset);
	SPtr<SamplerState>* samplers = (SPtr<SamplerState>*)(dataPtr + samplerArrayOffset);

	// Copy & destruct
	for(u32 i = 0; i < uniformBufferCount; i++)
	{
		mUniformBufferData[i].Buffer = uniformBuffers[i];
		mUniformBufferData[i].Offset = uniformBufferOffsets[i];

		uniformBuffers[i].~SPtr<GpuBuffer>();
	}

	for(u32 i = 0; i < sampledTextureCount; i++)
	{
		mSampledTextureData[i].Surface = sampledTextureSurfaces[i];
		storageSurfaces[i].~TextureSurface();

		mSampledTextureData[i].Texture = sampledTextures[i];
		sampledTextures[i].~SPtr<Texture>();
	}

	for(u32 i = 0; i < storageTextureCount; i++)
	{
		mStorageTextureData[i].Surface = storageSurfaces[i];
		storageSurfaces[i].~TextureSurface();

		mStorageTextureData[i].Texture = storageTextures[i];
		storageTextures[i].~SPtr<Texture>();
	}

	for(u32 i = 0; i < storageBufferCount; i++)
	{
		mStorageBufferData[i].Buffer = storageBuffers[i];
		mStorageBufferData[i].View.Offset = storageBufferOffsets[i];
		mStorageBufferData[i].View.Format = storageBufferFormats[i];

		storageBuffers[i].~SPtr<GpuBuffer>();
	}

	for(u32 i = 0; i < samplerCount; i++)
	{
		mSamplerStates[i] = samplers[i];
		samplers[i].~SPtr<SamplerState>();
	}
}
}}
