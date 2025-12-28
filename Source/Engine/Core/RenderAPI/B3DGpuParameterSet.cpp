//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "RenderAPI/B3DGpuParameterSet.h"

#include "B3DApplication.h"
#include "B3DGpuDevice.h"
#include "RenderAPI/B3DGpuProgramParameterDescription.h"
#include "RenderAPI/B3DGpuBuffer.h"
#include "RenderAPI/B3DGpuPipelineParameterLayout.h"
#include "RenderAPI/B3DGpuPipelineState.h"
#include "RenderAPI/B3DGpuParameterSetPool.h"
#include "Renderer/B3DRenderer.h"
#include "Math/B3DVector2.h"
#include "Image/B3DTexture.h"
#include "RenderAPI/B3DSamplerState.h"
#include "Debug/B3DDebug.h"
#include "Math/B3DVector3I.h"
#include "Math/B3DVector4I.h"
#include "Math/B3DMatrixNxM.h"
#include "CoreObject/B3DCoreObjectSync.h"

using namespace b3d;

const TextureSurface TextureSurface::kComplete = TextureSurface(0, 0, 0, 0);

GpuParametersSetBase::GpuParametersSetBase(const SPtr<GpuPipelineParameterSetLayout>& parameterLayout, u32 setIndex)
	: mParameterSetLayout(parameterLayout), mSet(setIndex)
{}

bool GpuParametersSetBase::HasParameter(const StringView& name) const
{
	return mParameterSetLayout->HasUniformBufferMember(name);
}

bool GpuParametersSetBase::HasSampledTexture(const StringView& name) const
{
	return mParameterSetLayout->HasUniformOfType(name, GpuParameterType::SampledTexture);
}

bool GpuParametersSetBase::HasStorageBuffer(const StringView& name) const
{
	return mParameterSetLayout->HasUniformOfType(name, GpuParameterType::StorageBuffer);
}

bool GpuParametersSetBase::HasStorageTexture(const StringView& name) const
{
	return mParameterSetLayout->HasUniformOfType(name, GpuParameterType::StorageTexture);
}

bool GpuParametersSetBase::HasSamplerState(const StringView& name) const
{
	return mParameterSetLayout->HasUniformOfType(name, GpuParameterType::Sampler);
}

bool GpuParametersSetBase::HasUniformBuffer(const StringView& name) const
{
	return mParameterSetLayout->HasUniformOfType(name, GpuParameterType::UniformBuffer);
}

template <bool IsRenderProxy>
TGpuParameterSet<IsRenderProxy>::TGpuParameterSet(const SPtr<GpuPipelineParameterSetLayout>& parameterSetLayout, u32 setIndex)
	: GpuParametersSetBase(parameterSetLayout, setIndex)
{
	const u32 uniformBufferCount = mParameterSetLayout->GetResourceCount(GpuParameterType::UniformBuffer);
	const u32 sampledTextureCount = mParameterSetLayout->GetResourceCount(GpuParameterType::SampledTexture);
	const u32 storageTextureCount = mParameterSetLayout->GetResourceCount(GpuParameterType::StorageTexture);
	const u32 storageBufferCount = mParameterSetLayout->GetResourceCount(GpuParameterType::StorageBuffer);
	const u32 samplerCount = mParameterSetLayout->GetResourceCount(GpuParameterType::Sampler);

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

template <bool IsRenderProxy>
TGpuParameterSet<IsRenderProxy>::~TGpuParameterSet()
{
	const u32 uniformBufferCount = mParameterSetLayout->GetResourceCount(GpuParameterType::UniformBuffer);
	const u32 sampledTextureCount = mParameterSetLayout->GetResourceCount(GpuParameterType::SampledTexture);
	const u32 storageTextureCount = mParameterSetLayout->GetResourceCount(GpuParameterType::StorageTexture);
	const u32 storageBufferCount = mParameterSetLayout->GetResourceCount(GpuParameterType::StorageBuffer);
	const u32 samplerCount = mParameterSetLayout->GetResourceCount(GpuParameterType::Sampler);

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

template <bool IsRenderProxy>
bool TGpuParameterSet<IsRenderProxy>::SetUniformBuffer(u32 slot, const UniformBufferType& buffer, u32 arrayIndex, u32 offset)
{
	const u32 sequentialResourceIndex = mParameterSetLayout->GetSequentialResourceIndex(slot, arrayIndex);
	if (sequentialResourceIndex == ~0u)
	{
		B3D_LOG(Warning, LogRenderBackend, "Unable to assign parameter. Cannot find uniform buffer with the set/slot combination: {0}/{1}", mSet, slot);
		return false;
	}

	mUniformBufferData[sequentialResourceIndex].Buffer = buffer;
	mUniformBufferData[sequentialResourceIndex].Offset = offset;

	MarkRenderProxyDataDirtyInternal();
	return true;
}

template <bool IsRenderProxy>
bool TGpuParameterSet<IsRenderProxy>::SetUniformBuffer(const StringView& name, const UniformBufferType& buffer, u32 arrayIndex, u32 offset)
{
	const u32 slot = mParameterSetLayout->GetSlot(name);
	if(slot == ~0u)
	{
		B3D_LOG(Warning, LogRenderBackend, "Unable to assign parameter. Cannot find uniform buffer with name: {0}", name);
		return false;
	}

	return SetUniformBuffer(slot, buffer, arrayIndex, offset);
}

template<bool IsRenderProxy>
bool TGpuParameterSet<IsRenderProxy>::TrySetUniformBuffer(const StringView& name, const UniformBufferType& parameterBlockBuffer, u32 arrayIndex, u32 offset)
{
	if (!HasUniformBuffer(name))
		return false;

	return SetUniformBuffer(name, parameterBlockBuffer, arrayIndex, offset);
}

template <bool IsRenderProxy>
template <class T>
void TGpuParameterSet<IsRenderProxy>::GetParameter(const StringView& name, TGpuParameterPrimitive<T, IsRenderProxy>& output) const
{
	if(!TryGetParameter(name, output))
		B3D_LOG(Warning, LogRenderBackend, "Cannot find parameter with the name: '{0}'", name);
}

template <bool IsRenderProxy>
void TGpuParameterSet<IsRenderProxy>::GetStructParameter(const StringView& name, TGpuParameterStruct<IsRenderProxy>& output) const
{
	if(!TryGetStructParameter(name, output))
		B3D_LOG(Warning, LogRenderBackend, "Cannot find struct parameter with the name: '{0}'", name);
}

template <bool IsRenderProxy>
void TGpuParameterSet<IsRenderProxy>::GetSampledTextureParameter(const StringView& name, TGpuParameterSampledTexture<IsRenderProxy>& output) const
{
	if(!TryGetSampledTextureParameter(name, output))
		B3D_LOG(Warning, LogRenderBackend, "Cannot find texture parameter with the name: '{0}'", name);
}

template <bool IsRenderProxy>
void TGpuParameterSet<IsRenderProxy>::GetStorageTextureParameter(const StringView& name, TGpuParameterStorageTexture<IsRenderProxy>& output) const
{
	if(!TryGetStorageTextureParameter(name, output))
		B3D_LOG(Warning, LogRenderBackend, "Cannot find storage texture parameter with the name: '{0}'", name);
}

template <bool IsRenderProxy>
void TGpuParameterSet<IsRenderProxy>::GetStorageBufferParameter(const StringView& name, TGpuParameterStorageBuffer<IsRenderProxy>& output) const
{
	if(!TryGetStorageBufferParameter(name, output))
		B3D_LOG(Warning, LogRenderBackend, "Cannot find storage buffer parameter with the name: '{0}'", name);
}

template <bool IsRenderProxy>
void TGpuParameterSet<IsRenderProxy>::GetUniformBufferParameter(const StringView& name, TGpuParameterUniformBuffer<IsRenderProxy>& output) const
{
	if(!TryGetUniformBufferParameter(name, output))
		B3D_LOG(Warning, LogRenderBackend, "Cannot find uniform buffer parameter with the name: '{0}'", name);
}

template <bool IsRenderProxy>
void TGpuParameterSet<IsRenderProxy>::GetSamplerStateParameter(const StringView& name, TGpuParameterSampler<IsRenderProxy>& output) const
{
	if(!TryGetSamplerStateParameter(name, output))
		B3D_LOG(Warning, LogRenderBackend, "Cannot find sampler parameter with the name: '{0}'", name);
}

template <bool IsRenderProxy>
template <class T>
bool TGpuParameterSet<IsRenderProxy>::TryGetParameter(const StringView& name, TGpuParameterPrimitive<T, IsRenderProxy>& output) const
{
	const GpuUniformBufferMemberInformation* parameterInformation = mParameterSetLayout->TryGetUniformBufferMemberInformation(name);
	if(parameterInformation == nullptr)
	{
		output = TGpuParameterPrimitive<T, IsRenderProxy>();
		return false;
	}

	output = TGpuParameterPrimitive<T, IsRenderProxy>(parameterInformation, GetSelf());
	return true;
}

template <bool IsRenderProxy>
bool TGpuParameterSet<IsRenderProxy>::TryGetStructParameter(const StringView& name, TGpuParameterStruct<IsRenderProxy>& output) const
{
	const GpuUniformBufferMemberInformation* parameterInformation = mParameterSetLayout->TryGetUniformBufferMemberInformation(name);
	if (parameterInformation == nullptr)
	{
		output = TGpuParameterStruct<IsRenderProxy>();
		return false;
	}

	output = TGpuParameterStruct<IsRenderProxy>(parameterInformation, GetSelf());
	return true;
}

template <bool IsRenderProxy>
bool TGpuParameterSet<IsRenderProxy>::TryGetSampledTextureParameter(const StringView& name, TGpuParameterSampledTexture<IsRenderProxy>& output) const
{
	const u32 slot = mParameterSetLayout->GetSlot(name);
	if (slot == ~0u)
	{
		output = TGpuParameterSampledTexture<IsRenderProxy>();
		return false;
	}

	output = TGpuParameterSampledTexture<IsRenderProxy>(GpuParameterBinding(mSet, slot), GetSelf());
	return true;
}

template <bool IsRenderProxy>
bool TGpuParameterSet<IsRenderProxy>::TryGetStorageTextureParameter(const StringView& name, TGpuParameterStorageTexture<IsRenderProxy>& output) const
{
	const u32 slot = mParameterSetLayout->GetSlot(name);
	if (slot == ~0u)
	{
		output = TGpuParameterStorageTexture<IsRenderProxy>();
		return false;
	}

	output = TGpuParameterStorageTexture<IsRenderProxy>(GpuParameterBinding(mSet, slot), GetSelf());
	return true;
}

template <bool IsRenderProxy>
bool TGpuParameterSet<IsRenderProxy>::TryGetStorageBufferParameter(const StringView& name, TGpuParameterStorageBuffer<IsRenderProxy>& output) const
{
	const u32 slot = mParameterSetLayout->GetSlot(name);
	if (slot == ~0u)
	{
		output = TGpuParameterStorageBuffer<IsRenderProxy>();
		return false;
	}

	output = TGpuParameterStorageBuffer<IsRenderProxy>(GpuParameterBinding(mSet, slot), GetSelf());
	return true;
}

template <bool IsRenderProxy>
bool TGpuParameterSet<IsRenderProxy>::TryGetUniformBufferParameter(const StringView& name, TGpuParameterUniformBuffer<IsRenderProxy>& output) const
{
	const u32 slot = mParameterSetLayout->GetSlot(name);
	if (slot == ~0u)
	{
		output = TGpuParameterUniformBuffer<IsRenderProxy>();
		return false;
	}

	output = TGpuParameterUniformBuffer<IsRenderProxy>(GpuParameterBinding(mSet, slot), GetSelf());
	return true;
}

template <bool IsRenderProxy>
bool TGpuParameterSet<IsRenderProxy>::TryGetSamplerStateParameter(const StringView& name, TGpuParameterSampler<IsRenderProxy>& output) const
{
	const u32 slot = mParameterSetLayout->GetSlot(name);
	if (slot == ~0u)
	{
		output = TGpuParameterSampler<IsRenderProxy>();
		return false;
	}

	output = TGpuParameterSampler<IsRenderProxy>(GpuParameterBinding(mSet, slot), GetSelf());
	return true;
}

template <bool IsRenderProxy>
typename TGpuParameterSet<IsRenderProxy>::UniformBufferType TGpuParameterSet<IsRenderProxy>::GetUniformBuffer(u32 slot, u32 arrayIndex) const
{
	const u32 sequentialResourceIndex = mParameterSetLayout->GetSequentialResourceIndex(slot, arrayIndex);
	if(sequentialResourceIndex == ~0u)
		return nullptr;

	return mUniformBufferData[sequentialResourceIndex].Buffer;
}

template <bool IsRenderProxy>
typename TGpuParameterSet<IsRenderProxy>::TextureType TGpuParameterSet<IsRenderProxy>::GetSampledTexture(u32 slot, u32 arrayIndex) const
{
	const u32 sequentialResourceIndex = mParameterSetLayout->GetSequentialResourceIndex(slot, arrayIndex);
	if(sequentialResourceIndex == ~0u)
		return TGpuParameterSet<IsRenderProxy>::TextureType();

	return mSampledTextureData[sequentialResourceIndex].Texture;
}

template <bool IsRenderProxy>
typename TGpuParameterSet<IsRenderProxy>::TextureType TGpuParameterSet<IsRenderProxy>::GetStorageTexture(u32 slot, u32 arrayIndex) const
{
	const u32 sequentialResourceIndex = mParameterSetLayout->GetSequentialResourceIndex(slot, arrayIndex);
	if(sequentialResourceIndex == ~0u)
		return TGpuParameterSet<IsRenderProxy>::TextureType();

	return mStorageTextureData[sequentialResourceIndex].Texture;
}

template <bool IsRenderProxy>
typename TGpuParameterSet<IsRenderProxy>::BufferType TGpuParameterSet<IsRenderProxy>::GetStorageBuffer(u32 slot, u32 arrayIndex) const
{
	const u32 sequentialResourceIndex = mParameterSetLayout->GetSequentialResourceIndex(slot, arrayIndex);
	if(sequentialResourceIndex == ~0u)
		return nullptr;

	return mStorageBufferData[sequentialResourceIndex].Buffer;
}

template <bool IsRenderProxy>
SPtr<SamplerState> TGpuParameterSet<IsRenderProxy>::GetSamplerState(u32 slot, u32 arrayIndex) const
{
	const u32 sequentialArrayIndex = mParameterSetLayout->GetSequentialResourceIndex(slot, arrayIndex);
	if(sequentialArrayIndex == ~0u)
		return nullptr;

	return mSamplerStates[sequentialArrayIndex];
}

template <bool IsRenderProxy>
const TextureSurface& TGpuParameterSet<IsRenderProxy>::GetTextureSurface(u32 slot, u32 arrayIndex) const
{
	static TextureSurface emptySurface;

	const u32 sequentialArrayIndex = mParameterSetLayout->GetSequentialResourceIndex(slot, arrayIndex);
	if(sequentialArrayIndex == ~0u)
		return emptySurface;

	return mSampledTextureData[sequentialArrayIndex].Surface;
}

template <bool IsRenderProxy>
const TextureSurface& TGpuParameterSet<IsRenderProxy>::GetStorageTextureSurface(u32 slot, u32 arrayIndex) const
{
	static TextureSurface emptySurface;

	const u32 sequentialArrayIndex = mParameterSetLayout->GetSequentialResourceIndex(slot, arrayIndex);
	if(sequentialArrayIndex == ~0u)
		return emptySurface;

	return mStorageTextureData[sequentialArrayIndex].Surface;
}

template <bool IsRenderProxy>
bool TGpuParameterSet<IsRenderProxy>::SetSampledTexture(u32 slot, const TextureType& texture, const TextureSurface& surface, u32 arrayIndex)
{
	const u32 sequentialArrayIndex = mParameterSetLayout->GetSequentialResourceIndex(slot, arrayIndex);
	if (sequentialArrayIndex == ~0u)
	{
		B3D_LOG(Warning, LogRenderBackend, "Unable to assign parameter. Cannot find sampled texture parameter with the set/slot combination: {0}/{1}", mSet, slot);
		return false;
	}

	mSampledTextureData[sequentialArrayIndex].Texture = texture;
	mSampledTextureData[sequentialArrayIndex].Surface = surface;

	MarkResourcesDirtyInternal();
	MarkRenderProxyDataDirtyInternal();

	return true;
}

template <bool IsRenderProxy>
bool TGpuParameterSet<IsRenderProxy>::SetStorageTexture(u32 slot, const TextureType& texture, const TextureSurface& surface, u32 arrayIndex)
{
	const u32 sequentialArrayIndex = mParameterSetLayout->GetSequentialResourceIndex(slot, arrayIndex);
	if (sequentialArrayIndex == ~0u)
	{
		B3D_LOG(Warning, LogRenderBackend, "Unable to assign parameter. Cannot find storage texture parameter with the set/slot combination: {0}/{1}", mSet, slot);
		return false;
	}

	mStorageTextureData[sequentialArrayIndex].Texture = texture;
	mStorageTextureData[sequentialArrayIndex].Surface = surface;

	MarkResourcesDirtyInternal();
	MarkRenderProxyDataDirtyInternal();

	return true;
}

template <bool IsRenderProxy>
bool TGpuParameterSet<IsRenderProxy>::SetStorageBuffer(u32 slot, const BufferType& buffer, u32 arrayIndex, GpuBufferViewInformation view)
{
	const u32 sequentialArrayIndex = mParameterSetLayout->GetSequentialResourceIndex(slot, arrayIndex);
	if (sequentialArrayIndex == ~0u)
	{
		B3D_LOG(Warning, LogRenderBackend, "Unable to assign parameter. Cannot find buffer parameter with the set/slot combination: {0}/{1}", mSet, slot);
		return false;
	}

	mStorageBufferData[sequentialArrayIndex].Buffer = buffer;
	mStorageBufferData[sequentialArrayIndex].View = view;

	MarkResourcesDirtyInternal();
	MarkRenderProxyDataDirtyInternal();

	return true;
}

template <bool IsRenderProxy>
bool TGpuParameterSet<IsRenderProxy>::SetSamplerState(u32 slot, const SPtr<SamplerState>& sampler, u32 arrayIndex)
{
	const u32 sequentialArrayIndex = mParameterSetLayout->GetSequentialResourceIndex(slot, arrayIndex);
	if (sequentialArrayIndex == ~0u)
	{
		B3D_LOG(Warning, LogRenderBackend, "Unable to assign parameter. Cannot find sampler parameter with the set/slot combination: {0}/{1}", mSet, slot);
		return false;
	}

	mSamplerStates[sequentialArrayIndex] = sampler;

	MarkResourcesDirtyInternal();
	MarkRenderProxyDataDirtyInternal();

	return true;
}

template class TGpuParameterSet<false>;
template class TGpuParameterSet<true>;

template B3D_EXPORT void TGpuParameterSet<false>::GetParameter<float>(const StringView&, TGpuParameterPrimitive<float, false>&) const;
template B3D_EXPORT void TGpuParameterSet<false>::GetParameter<int>(const StringView&, TGpuParameterPrimitive<int, false>&) const;
template B3D_EXPORT void TGpuParameterSet<false>::GetParameter<Color>(const StringView&, TGpuParameterPrimitive<Color, false>&) const;
template B3D_EXPORT void TGpuParameterSet<false>::GetParameter<Vector2>(const StringView&, TGpuParameterPrimitive<Vector2, false>&) const;
template B3D_EXPORT void TGpuParameterSet<false>::GetParameter<Vector3>(const StringView&, TGpuParameterPrimitive<Vector3, false>&) const;
template B3D_EXPORT void TGpuParameterSet<false>::GetParameter<Vector4>( const StringView&, TGpuParameterPrimitive<Vector4, false>&) const;
template B3D_EXPORT void TGpuParameterSet<false>::GetParameter<Vector2I>(const StringView&, TGpuParameterPrimitive<Vector2I, false>&) const;
template B3D_EXPORT void TGpuParameterSet<false>::GetParameter<Vector3I>(const StringView&, TGpuParameterPrimitive<Vector3I, false>&) const;
template B3D_EXPORT void TGpuParameterSet<false>::GetParameter<Vector4I>(const StringView&, TGpuParameterPrimitive<Vector4I, false>&) const;
template B3D_EXPORT void TGpuParameterSet<false>::GetParameter<Matrix2>(const StringView&, TGpuParameterPrimitive<Matrix2, false>&) const;
template B3D_EXPORT void TGpuParameterSet<false>::GetParameter<Matrix2x3>(const StringView&, TGpuParameterPrimitive<Matrix2x3, false>&) const;
template B3D_EXPORT void TGpuParameterSet<false>::GetParameter<Matrix2x4>(const StringView&, TGpuParameterPrimitive<Matrix2x4, false>&) const;
template B3D_EXPORT void TGpuParameterSet<false>::GetParameter<Matrix3>(const StringView&, TGpuParameterPrimitive<Matrix3, false>&) const;
template B3D_EXPORT void TGpuParameterSet<false>::GetParameter<Matrix3x2>(const StringView&, TGpuParameterPrimitive<Matrix3x2, false>&) const;
template B3D_EXPORT void TGpuParameterSet<false>::GetParameter<Matrix3x4>(const StringView&, TGpuParameterPrimitive<Matrix3x4, false>&) const;
template B3D_EXPORT void TGpuParameterSet<false>::GetParameter<Matrix4>(const StringView&, TGpuParameterPrimitive<Matrix4, false>&) const;
template B3D_EXPORT void TGpuParameterSet<false>::GetParameter<Matrix4x2>(const StringView&, TGpuParameterPrimitive<Matrix4x2, false>&) const;
template B3D_EXPORT void TGpuParameterSet<false>::GetParameter<Matrix4x3>(const StringView&, TGpuParameterPrimitive<Matrix4x3, false>&) const;

template B3D_EXPORT void TGpuParameterSet<true>::GetParameter<float>(const StringView&, TGpuParameterPrimitive<float, true>&) const;
template B3D_EXPORT void TGpuParameterSet<true>::GetParameter<int>(const StringView&, TGpuParameterPrimitive<int, true>&) const;
template B3D_EXPORT void TGpuParameterSet<true>::GetParameter<Color>(const StringView&, TGpuParameterPrimitive<Color, true>&) const;
template B3D_EXPORT void TGpuParameterSet<true>::GetParameter<Vector2>(const StringView&, TGpuParameterPrimitive<Vector2, true>&) const;
template B3D_EXPORT void TGpuParameterSet<true>::GetParameter<Vector3>(const StringView&, TGpuParameterPrimitive<Vector3, true>&) const;
template B3D_EXPORT void TGpuParameterSet<true>::GetParameter<Vector4>(const StringView&, TGpuParameterPrimitive<Vector4, true>&) const;
template B3D_EXPORT void TGpuParameterSet<true>::GetParameter<Vector2I>(const StringView&, TGpuParameterPrimitive<Vector2I, true>&) const;
template B3D_EXPORT void TGpuParameterSet<true>::GetParameter<Vector3I>(const StringView&, TGpuParameterPrimitive<Vector3I, true>&) const;
template B3D_EXPORT void TGpuParameterSet<true>::GetParameter<Vector4I>(const StringView&, TGpuParameterPrimitive<Vector4I, true>&) const;
template B3D_EXPORT void TGpuParameterSet<true>::GetParameter<Matrix2>(const StringView&, TGpuParameterPrimitive<Matrix2, true>&) const;
template B3D_EXPORT void TGpuParameterSet<true>::GetParameter<Matrix2x3>(const StringView&, TGpuParameterPrimitive<Matrix2x3, true>&) const;
template B3D_EXPORT void TGpuParameterSet<true>::GetParameter<Matrix2x4>(const StringView&, TGpuParameterPrimitive<Matrix2x4, true>&) const;
template B3D_EXPORT void TGpuParameterSet<true>::GetParameter<Matrix3>(const StringView&, TGpuParameterPrimitive<Matrix3, true>&) const;
template B3D_EXPORT void TGpuParameterSet<true>::GetParameter<Matrix3x2>(const StringView&, TGpuParameterPrimitive<Matrix3x2, true>&) const;
template B3D_EXPORT void TGpuParameterSet<true>::GetParameter<Matrix3x4>(const StringView&, TGpuParameterPrimitive<Matrix3x4, true>&) const;
template B3D_EXPORT void TGpuParameterSet<true>::GetParameter<Matrix4>(const StringView&, TGpuParameterPrimitive<Matrix4, true>&) const;
template B3D_EXPORT void TGpuParameterSet<true>::GetParameter<Matrix4x2>(const StringView&, TGpuParameterPrimitive<Matrix4x2, true>&) const;
template B3D_EXPORT void TGpuParameterSet<true>::GetParameter<Matrix4x3>(const StringView&, TGpuParameterPrimitive<Matrix4x3, true>&) const;

namespace b3d
{
	B3D_SYNC_BLOCK_BEGIN(GpuParameterSet, SyncPacket)
		B3D_SYNC_BLOCK_ENTRY_CUSTOM(Vector<HTexture>, SampledTextures)
		B3D_SYNC_BLOCK_ENTRY_CUSTOM(Vector<TextureSurface>, SampledTextureSurfaces)
		B3D_SYNC_BLOCK_ENTRY_CUSTOM(Vector<HTexture>, StorageTextures)
		B3D_SYNC_BLOCK_ENTRY_CUSTOM(Vector<TextureSurface>, StorageTextureSurfaces)
		B3D_SYNC_BLOCK_ENTRY_CUSTOM(Vector<SPtr<GpuBuffer>>, UniformBuffers)
		B3D_SYNC_BLOCK_ENTRY_CUSTOM(Vector<u32>, UniformBufferOffsets)
		B3D_SYNC_BLOCK_ENTRY_CUSTOM(Vector<SPtr<GpuBuffer>>, StorageBuffers)
		B3D_SYNC_BLOCK_ENTRY_CUSTOM(Vector<GpuBufferViewInformation>, StorageBufferViews)
		B3D_SYNC_BLOCK_ENTRY_CUSTOM(Vector<SPtr<SamplerState>>, SamplerStates)
	B3D_SYNC_BLOCK_END
}
const GpuDataParameterTypeInformationLookup GpuParameterSet::kParamSizes;

GpuParameterSet::GpuParameterSet(const SPtr<GpuPipelineParameterSetLayout>& parameterSetLayout, u32 setIndex)
	: TGpuParameterSet(parameterSetLayout, setIndex)
{
}

SPtr<GpuParameterSet> GpuParameterSet::GetSelf() const
{
	return std::static_pointer_cast<GpuParameterSet>(GetShared());
}

SPtr<render::RenderProxy> GpuParameterSet::CreateRenderProxy() const
{
	GpuParameterSetPool& pool = render::GetRenderer()->GetParameterSetPool();
	return pool.Create(mParameterSetLayout, mSet, true);
}

void GpuParameterSet::MarkRenderProxyDataDirtyInternal()
{
	MarkRenderProxyDataDirty();
}

void GpuParameterSet::MarkResourcesDirtyInternal()
{
	MarkListenerResourcesDirty();
}

SPtr<GpuParameterSet> GpuParameterSet::Create(const SPtr<GpuPipelineParameterSetLayout>& parameterSetLayout, u32 setIndex)
{
	GpuParameterSet* const output = new(B3DAllocate<GpuParameterSet>()) GpuParameterSet(parameterSetLayout, setIndex);
	SPtr<GpuParameterSet> shared = B3DMakeSharedFromExisting<GpuParameterSet>(output);
	shared->SetShared(shared);
	shared->Initialize();

	return shared;
}

RenderProxySyncPacket* GpuParameterSet::CreateRenderProxySyncPacket(FrameAllocator& allocator, u32 flags)
{
	SyncPacket* const syncPacket = allocator.Construct<SyncPacket>(*this, allocator, flags);

	const u32 uniformBufferCount = mParameterSetLayout->GetResourceCount(GpuParameterType::UniformBuffer);
	syncPacket->UniformBufferOffsets.reserve(uniformBufferCount);
	syncPacket->UniformBufferOffsets.reserve(uniformBufferCount);

	for(u32 i = 0; i < uniformBufferCount; i++)
	{
		syncPacket->UniformBufferOffsets.push_back(mUniformBufferData->Offset);
		syncPacket->UniformBuffers.push_back(B3DGetRenderProxy(mUniformBufferData[i].Buffer));
	}

	const u32 sampledTextureCount = mParameterSetLayout->GetResourceCount(GpuParameterType::SampledTexture);
	syncPacket->SampledTextureSurfaces.reserve(sampledTextureCount);
	syncPacket->SampledTextures.reserve(sampledTextureCount);

	for(u32 i = 0; i < sampledTextureCount; i++)
	{
		syncPacket->SampledTextureSurfaces.push_back(mSampledTextureData[i].Surface);
		syncPacket->SampledTextures.push_back(B3DGetRenderProxy(mSampledTextureData[i].Texture));
	}

	const u32 storageTextureCount = mParameterSetLayout->GetResourceCount(GpuParameterType::StorageTexture);
	syncPacket->StorageTextureSurfaces.reserve(storageTextureCount);
	syncPacket->StorageTextures.reserve(storageTextureCount);

	for(u32 i = 0; i < storageTextureCount; i++)
	{
		syncPacket->StorageTextureSurfaces.push_back(mStorageTextureData[i].Surface);
		syncPacket->StorageTextures.push_back(B3DGetRenderProxy(mStorageTextureData[i].Texture));
	}

	const u32 storageBufferCount = mParameterSetLayout->GetResourceCount(GpuParameterType::StorageBuffer);
	syncPacket->StorageBufferViews.reserve(storageBufferCount);
	syncPacket->StorageBuffers.reserve(storageBufferCount);

	for(u32 i = 0; i < storageBufferCount; i++)
	{
		syncPacket->StorageBufferViews.push_back(mStorageBufferData[i].View);
		syncPacket->StorageBuffers.push_back(B3DGetRenderProxy(mStorageBufferData[i].Buffer));
	}

	const u32 samplerCount = mParameterSetLayout->GetResourceCount(GpuParameterType::Sampler);
	syncPacket->SamplerStates.reserve(samplerCount);

	for(u32 i = 0; i < samplerCount; i++)
		syncPacket->SamplerStates.push_back(mSamplerStates[i]);

	return syncPacket;
}

void GpuParameterSet::GetListenerResources(Vector<HResource>& resources)
{
	u32 sampledTextureCount = mParameterSetLayout->GetResourceCount(GpuParameterType::SampledTexture);
	u32 storageTextureCount = mParameterSetLayout->GetResourceCount(GpuParameterType::StorageTexture);

	for(u32 i = 0; i < sampledTextureCount; i++)
	{
		if(mSampledTextureData[i].Texture != nullptr)
			resources.push_back(mSampledTextureData[i].Texture);
	}

	for(u32 i = 0; i < storageTextureCount; i++)
	{
		if(mStorageTextureData[i].Texture != nullptr)
			resources.push_back(mStorageTextureData[i].Texture);
	}
}

namespace b3d { namespace render
{
GpuParameterSet::GpuParameterSet(const SPtr<GpuPipelineParameterSetLayout>& parameterSetLayout, u32 setIndex)
	: TGpuParameterSet(parameterSetLayout, setIndex)
{
}

SPtr<GpuParameterSet> GpuParameterSet::GetSelf() const
{
	return std::static_pointer_cast<GpuParameterSet>(GetShared());
}

void GpuParameterSet::SyncFromCoreObject(const CoreSyncData& data, FrameAllocator& allocator)
{
	auto* const syncPacket = data.GetSyncPacket<b3d::GpuParameterSet::SyncPacket>();
	if(!syncPacket)
		return;

	const u32 uniformBufferCount = mParameterSetLayout->GetResourceCount(GpuParameterType::UniformBuffer);
	const u32 sampledTextureCount = mParameterSetLayout->GetResourceCount(GpuParameterType::SampledTexture);
	const u32 storageTextureCount = mParameterSetLayout->GetResourceCount(GpuParameterType::StorageTexture);
	const u32 storageBufferCount = mParameterSetLayout->GetResourceCount(GpuParameterType::StorageBuffer);
	const u32 samplerCount = mParameterSetLayout->GetResourceCount(GpuParameterType::Sampler);

	if(B3D_ENSURE(syncPacket->UniformBuffers.size() == uniformBufferCount && syncPacket->UniformBufferOffsets.size() == uniformBufferCount))
	{
		for(u32 i = 0; i < uniformBufferCount; i++)
		{
			mUniformBufferData[i].Buffer = syncPacket->UniformBuffers[i];
			mUniformBufferData[i].Offset = syncPacket->UniformBufferOffsets[i];
		}
	}

	if(B3D_ENSURE(syncPacket->SampledTextures.size() == sampledTextureCount && syncPacket->SampledTextureSurfaces.size() == sampledTextureCount))
	{
		for(u32 i = 0; i < sampledTextureCount; i++)
		{
			mSampledTextureData[i].Texture = syncPacket->SampledTextures[i];
			mSampledTextureData[i].Surface = syncPacket->SampledTextureSurfaces[i];
		}
	}

	if(B3D_ENSURE(syncPacket->StorageTextures.size() == storageTextureCount && syncPacket->StorageTextureSurfaces.size() == storageTextureCount))
	{
		for(u32 i = 0; i < storageTextureCount; i++)
		{
			mStorageTextureData[i].Texture = syncPacket->StorageTextures[i];
			mStorageTextureData[i].Surface = syncPacket->StorageTextureSurfaces[i];
		}
	}

	if(B3D_ENSURE(syncPacket->StorageBuffers.size() == storageBufferCount && syncPacket->StorageBufferViews.size() == storageBufferCount))
	{
		for(u32 i = 0; i < storageBufferCount; i++)
		{
			mStorageBufferData[i].Buffer = syncPacket->StorageBuffers[i];
			mStorageBufferData[i].View = syncPacket->StorageBufferViews[i];
		}
	}

	if(B3D_ENSURE(syncPacket->SamplerStates.size() == samplerCount))
	{
		for(u32 i = 0; i < samplerCount; i++)
		{
			mSamplerStates[i] = syncPacket->SamplerStates[i];
		}
	}
}
}}
