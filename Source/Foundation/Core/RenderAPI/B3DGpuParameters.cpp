//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "RenderAPI/B3DGpuParameters.h"

#include "B3DApplication.h"
#include "B3DGpuDevice.h"
#include "RenderAPI/B3DGpuProgramParameterDescription.h"
#include "RenderAPI/B3DGpuBuffer.h"
#include "RenderAPI/B3DGpuPipelineParameterLayout.h"
#include "RenderAPI/B3DGpuPipelineState.h"
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

GpuParametersBase::GpuParametersBase(const SPtr<GpuPipelineParameterLayout>& parameterLayout)
	: mParameterLayout(parameterLayout)
{}

bool GpuParametersBase::HasParameter(const StringView& name) const
{
	return mParameterLayout->HasUniformBufferMember(name);
}

bool GpuParametersBase::HasSampledTexture(const StringView& name) const
{
	return mParameterLayout->HasUniformOfType(name, GpuParameterType::SampledTexture);
}

bool GpuParametersBase::HasStorageBuffer(const StringView& name) const
{
	return mParameterLayout->HasUniformOfType(name, GpuParameterType::StorageBuffer);
}

bool GpuParametersBase::HasStorageTexture(const StringView& name) const
{
	return mParameterLayout->HasUniformOfType(name, GpuParameterType::StorageTexture);
}

bool GpuParametersBase::HasSamplerState(const StringView& name) const
{
	return mParameterLayout->HasUniformOfType(name, GpuParameterType::Sampler);
}

bool GpuParametersBase::HasUniformBuffer(const StringView& name) const
{
	return mParameterLayout->HasUniformOfType(name, GpuParameterType::UniformBuffer);
}

template <bool IsRenderProxy>
TGpuParameters<IsRenderProxy>::TGpuParameters(const SPtr<GpuPipelineParameterLayout>& parameterLayout)
	: GpuParametersBase(parameterLayout)
{
	const u32 uniformBufferCount = mParameterLayout->GetResourceCount(GpuParameterType::UniformBuffer);
	const u32 sampledTextureCount = mParameterLayout->GetResourceCount(GpuParameterType::SampledTexture);
	const u32 storageTextureCount = mParameterLayout->GetResourceCount(GpuParameterType::StorageTexture);
	const u32 storageBufferCount = mParameterLayout->GetResourceCount(GpuParameterType::StorageBuffer);
	const u32 samplerCount = mParameterLayout->GetResourceCount(GpuParameterType::Sampler);

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
TGpuParameters<IsRenderProxy>::~TGpuParameters()
{
	const u32 uniformBufferCount = mParameterLayout->GetResourceCount(GpuParameterType::UniformBuffer);
	const u32 sampledTextureCount = mParameterLayout->GetResourceCount(GpuParameterType::SampledTexture);
	const u32 storageTextureCount = mParameterLayout->GetResourceCount(GpuParameterType::StorageTexture);
	const u32 storageBufferCount = mParameterLayout->GetResourceCount(GpuParameterType::StorageBuffer);
	const u32 samplerCount = mParameterLayout->GetResourceCount(GpuParameterType::Sampler);

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
bool TGpuParameters<IsRenderProxy>::SetUniformBuffer(u32 set, u32 slot, const UniformBufferType& paramBlockBuffer, u32 arrayIndex, u32 offset)
{
	const u32 sequentialResourceIndex = mParameterLayout->GetSequentialResourceIndex(set, slot, arrayIndex);
	if (sequentialResourceIndex == ~0u)
	{
		B3D_LOG(Warning, RenderBackend, "Unable to assign parameter. Cannot find parameter block with the set/slot combination: {0}/{1}", set, slot);
		return false;
	}

	mUniformBufferData[sequentialResourceIndex].Buffer = paramBlockBuffer;
	mUniformBufferData[sequentialResourceIndex].Offset = offset;

	MarkRenderProxyDataDirtyInternal();
	return true;
}

template <bool IsRenderProxy>
bool TGpuParameters<IsRenderProxy>::SetUniformBuffer(const StringView& name, const UniformBufferType& paramBlockBuffer, u32 arrayIndex, u32 offset)
{
	const GpuParameterBinding binding = mParameterLayout->GetBinding(name);
	if(!binding.IsValid())
	{
		B3D_LOG(Warning, RenderBackend, "Unable to assign parameter. Cannot find parameter block with name: {0}", name);
		return false;
	}

	return SetUniformBuffer(binding.Set, binding.Slot, paramBlockBuffer, arrayIndex, offset);
}

template<bool IsRenderProxy>
bool TGpuParameters<IsRenderProxy>::TrySetUniformBuffer(const StringView& name, const UniformBufferType& parameterBlockBuffer, u32 arrayIndex, u32 offset)
{
	if (!HasUniformBuffer(name))
		return false;

	return SetUniformBuffer(name, parameterBlockBuffer, arrayIndex, offset);
}

template <bool IsRenderProxy>
template <class T>
void TGpuParameters<IsRenderProxy>::GetParameter(const StringView& name, TGpuParameterPrimitive<T, IsRenderProxy>& output) const
{
	if(!TryGetParameter(name, output))
		B3D_LOG(Warning, RenderBackend, "Cannot find parameter with the name: '{0}'", name);
}

template <bool IsRenderProxy>
void TGpuParameters<IsRenderProxy>::GetStructParameter(const StringView& name, TGpuParameterStruct<IsRenderProxy>& output) const
{
	if(!TryGetStructParameter(name, output))
		B3D_LOG(Warning, RenderBackend, "Cannot find struct parameter with the name: '{0}'", name);
}

template <bool IsRenderProxy>
void TGpuParameters<IsRenderProxy>::GetSampledTextureParameter(const StringView& name, TGpuParameterSampledTexture<IsRenderProxy>& output) const
{
	if(!TryGetSampledTextureParameter(name, output))
		B3D_LOG(Warning, RenderBackend, "Cannot find texture parameter with the name: '{0}'", name);
}

template <bool IsRenderProxy>
void TGpuParameters<IsRenderProxy>::GetStorageTextureParameter(const StringView& name, TGpuParameterStorageTexture<IsRenderProxy>& output) const
{
	if(!TryGetStorageTextureParameter(name, output))
		B3D_LOG(Warning, RenderBackend, "Cannot find storage texture parameter with the name: '{0}'", name);
}

template <bool IsRenderProxy>
void TGpuParameters<IsRenderProxy>::GetStorageBufferParameter(const StringView& name, TGpuParameterStorageBuffer<IsRenderProxy>& output) const
{
	if(!TryGetStorageBufferParameter(name, output))
		B3D_LOG(Warning, RenderBackend, "Cannot find storage buffer parameter with the name: '{0}'", name);
}

template <bool IsRenderProxy>
void TGpuParameters<IsRenderProxy>::GetUniformBufferParameter(const StringView& name, TGpuParameterUniformBuffer<IsRenderProxy>& output) const
{
	if(!TryGetUniformBufferParameter(name, output))
		B3D_LOG(Warning, RenderBackend, "Cannot find uniform buffer parameter with the name: '{0}'", name);
}

template <bool IsRenderProxy>
void TGpuParameters<IsRenderProxy>::GetSamplerStateParameter(const StringView& name, TGpuParameterSampler<IsRenderProxy>& output) const
{
	if(!TryGetSamplerStateParameter(name, output))
		B3D_LOG(Warning, RenderBackend, "Cannot find sampler parameter with the name: '{0}'", name);
}

template <bool IsRenderProxy>
template <class T>
bool TGpuParameters<IsRenderProxy>::TryGetParameter(const StringView& name, TGpuParameterPrimitive<T, IsRenderProxy>& output) const
{
	const GpuUniformBufferMemberInformation* parameterInformation = mParameterLayout->TryGetUniformBufferMemberInformation(name);
	if(parameterInformation == nullptr)
	{
		output = TGpuParameterPrimitive<T, IsRenderProxy>(nullptr, nullptr);
		return false;
	}

	output = TGpuParameterPrimitive<T, IsRenderProxy>(parameterInformation, GetSelf());
	return true;
}

template <bool IsRenderProxy>
bool TGpuParameters<IsRenderProxy>::TryGetStructParameter(const StringView& name, TGpuParameterStruct<IsRenderProxy>& output) const
{
	const GpuUniformBufferMemberInformation* parameterInformation = mParameterLayout->TryGetUniformBufferMemberInformation(name);
	if (parameterInformation == nullptr)
	{
		output = TGpuParameterStruct<IsRenderProxy>(nullptr, nullptr);
		return false;
	}

	output = TGpuParameterStruct<IsRenderProxy>(parameterInformation, GetSelf());
	return true;
}

template <bool IsRenderProxy>
bool TGpuParameters<IsRenderProxy>::TryGetSampledTextureParameter(const StringView& name, TGpuParameterSampledTexture<IsRenderProxy>& output) const
{
	const GpuParameterBinding binding = mParameterLayout->GetBinding(name);
	if (!binding.IsValid())
	{
		output = TGpuParameterSampledTexture<IsRenderProxy>(GpuParameterBinding(), nullptr);
		return false;
	}

	output = TGpuParameterSampledTexture<IsRenderProxy>(binding, GetSelf());
	return true;
}

template <bool IsRenderProxy>
bool TGpuParameters<IsRenderProxy>::TryGetStorageTextureParameter(const StringView& name, TGpuParameterStorageTexture<IsRenderProxy>& output) const
{
	const GpuParameterBinding binding = mParameterLayout->GetBinding(name);
	if (!binding.IsValid())
	{
		output = TGpuParameterStorageTexture<IsRenderProxy>(GpuParameterBinding(), nullptr);
		return false;
	}

	output = TGpuParameterStorageTexture<IsRenderProxy>(binding, GetSelf());
	return true;
}

template <bool IsRenderProxy>
bool TGpuParameters<IsRenderProxy>::TryGetStorageBufferParameter(const StringView& name, TGpuParameterStorageBuffer<IsRenderProxy>& output) const
{
	const GpuParameterBinding binding = mParameterLayout->GetBinding(name);
	if (!binding.IsValid())
	{
		output = TGpuParameterStorageBuffer<IsRenderProxy>(GpuParameterBinding(), nullptr);
		return false;
	}

	output = TGpuParameterStorageBuffer<IsRenderProxy>(binding, GetSelf());
	return true;
}

template <bool IsRenderProxy>
bool TGpuParameters<IsRenderProxy>::TryGetUniformBufferParameter(const StringView& name, TGpuParameterUniformBuffer<IsRenderProxy>& output) const
{
	const GpuParameterBinding binding = mParameterLayout->GetBinding(name);
	if (!binding.IsValid())
	{
		output = TGpuParameterUniformBuffer<IsRenderProxy>(GpuParameterBinding(), nullptr);
		return false;
	}

	output = TGpuParameterUniformBuffer<IsRenderProxy>(binding, GetSelf());
	return true;
}

template <bool IsRenderProxy>
bool TGpuParameters<IsRenderProxy>::TryGetSamplerStateParameter(const StringView& name, TGpuParameterSampler<IsRenderProxy>& output) const
{
	const GpuParameterBinding binding = mParameterLayout->GetBinding(name);
	if (!binding.IsValid())
	{
		output = TGpuParameterSampler<IsRenderProxy>(GpuParameterBinding(), nullptr);
		return false;
	}

	output = TGpuParameterSampler<IsRenderProxy>(binding, GetSelf());
	return true;
}

template <bool IsRenderProxy>
typename TGpuParameters<IsRenderProxy>::UniformBufferType TGpuParameters<IsRenderProxy>::GetUniformBuffer(u32 set, u32 slot, u32 arrayIndex) const
{
	const u32 sequentialResourceIndex = mParameterLayout->GetSequentialResourceIndex(set, slot, arrayIndex);
	if(sequentialResourceIndex == ~0u)
		return nullptr;

	return mUniformBufferData[sequentialResourceIndex].Buffer;
}

template <bool IsRenderProxy>
typename TGpuParameters<IsRenderProxy>::TextureType TGpuParameters<IsRenderProxy>::GetSampledTexture(u32 set, u32 slot, u32 arrayIndex) const
{
	const u32 sequentialResourceIndex = mParameterLayout->GetSequentialResourceIndex(set, slot, arrayIndex);
	if(sequentialResourceIndex == ~0u)
		return TGpuParameters<IsRenderProxy>::TextureType();

	return mSampledTextureData[sequentialResourceIndex].Texture;
}

template <bool IsRenderProxy>
typename TGpuParameters<IsRenderProxy>::TextureType TGpuParameters<IsRenderProxy>::GetStorageTexture(u32 set, u32 slot, u32 arrayIndex) const
{
	const u32 sequentialResourceIndex = mParameterLayout->GetSequentialResourceIndex(set, slot, arrayIndex);
	if(sequentialResourceIndex == ~0u)
		return TGpuParameters<IsRenderProxy>::TextureType();

	return mStorageTextureData[sequentialResourceIndex].Texture;
}

template <bool IsRenderProxy>
typename TGpuParameters<IsRenderProxy>::BufferType TGpuParameters<IsRenderProxy>::GetStorageBuffer(u32 set, u32 slot, u32 arrayIndex) const
{
	const u32 sequentialResourceIndex = mParameterLayout->GetSequentialResourceIndex(set, slot, arrayIndex);
	if(sequentialResourceIndex == ~0u)
		return nullptr;

	return mStorageBufferData[sequentialResourceIndex].Buffer;
}

template <bool IsRenderProxy>
SPtr<SamplerState> TGpuParameters<IsRenderProxy>::GetSamplerState(u32 set, u32 slot, u32 arrayIndex) const
{
	const u32 sequentialArrayIndex = mParameterLayout->GetSequentialResourceIndex(set, slot, arrayIndex);
	if(sequentialArrayIndex == ~0u)
		return nullptr;

	return mSamplerStates[sequentialArrayIndex];
}

template <bool IsRenderProxy>
const TextureSurface& TGpuParameters<IsRenderProxy>::GetTextureSurface(u32 set, u32 slot, u32 arrayIndex) const
{
	static TextureSurface emptySurface;

	const u32 sequentialArrayIndex = mParameterLayout->GetSequentialResourceIndex(set, slot, arrayIndex);
	if(sequentialArrayIndex == ~0u)
		return emptySurface;

	return mSampledTextureData[sequentialArrayIndex].Surface;
}

template <bool IsRenderProxy>
const TextureSurface& TGpuParameters<IsRenderProxy>::GetStorageTextureSurface(u32 set, u32 slot, u32 arrayIndex) const
{
	static TextureSurface emptySurface;

	const u32 sequentialArrayIndex = mParameterLayout->GetSequentialResourceIndex(set, slot, arrayIndex);
	if(sequentialArrayIndex == ~0u)
		return emptySurface;

	return mStorageTextureData[sequentialArrayIndex].Surface;
}

template <bool IsRenderProxy>
bool TGpuParameters<IsRenderProxy>::SetSampledTexture(u32 set, u32 slot, const TextureType& texture, const TextureSurface& surface, u32 arrayIndex)
{
	const u32 sequentialArrayIndex = mParameterLayout->GetSequentialResourceIndex(set, slot, arrayIndex);
	if (sequentialArrayIndex == ~0u)
	{
		B3D_LOG(Warning, RenderBackend, "Unable to assign parameter. Cannot find sampled texture parameter with the set/slot combination: {0}/{1}", set, slot);
		return false;
	}

	mSampledTextureData[sequentialArrayIndex].Texture = texture;
	mSampledTextureData[sequentialArrayIndex].Surface = surface;

	MarkResourcesDirtyInternal();
	MarkRenderProxyDataDirtyInternal();

	return true;
}

template <bool IsRenderProxy>
bool TGpuParameters<IsRenderProxy>::SetStorageTexture(u32 set, u32 slot, const TextureType& texture, const TextureSurface& surface, u32 arrayIndex)
{
	const u32 sequentialArrayIndex = mParameterLayout->GetSequentialResourceIndex(set, slot, arrayIndex);
	if (sequentialArrayIndex == ~0u)
	{
		B3D_LOG(Warning, RenderBackend, "Unable to assign parameter. Cannot find storage texture parameter with the set/slot combination: {0}/{1}", set, slot);
		return false;
	}

	mStorageTextureData[sequentialArrayIndex].Texture = texture;
	mStorageTextureData[sequentialArrayIndex].Surface = surface;

	MarkResourcesDirtyInternal();
	MarkRenderProxyDataDirtyInternal();

	return true;
}

template <bool IsRenderProxy>
bool TGpuParameters<IsRenderProxy>::SetStorageBuffer(u32 set, u32 slot, const BufferType& buffer, u32 arrayIndex, GpuBufferViewInformation view)
{
	const u32 sequentialArrayIndex = mParameterLayout->GetSequentialResourceIndex(set, slot, arrayIndex);
	if (sequentialArrayIndex == ~0u)
	{
		B3D_LOG(Warning, RenderBackend, "Unable to assign parameter. Cannot find buffer parameter with the set/slot combination: {0}/{1}", set, slot);
		return false;
	}

	mStorageBufferData[sequentialArrayIndex].Buffer = buffer;
	mStorageBufferData[sequentialArrayIndex].View = view;

	MarkResourcesDirtyInternal();
	MarkRenderProxyDataDirtyInternal();

	return true;
}

template <bool IsRenderProxy>
bool TGpuParameters<IsRenderProxy>::SetSamplerState(u32 set, u32 slot, const SPtr<SamplerState>& sampler, u32 arrayIndex)
{
	const u32 sequentialArrayIndex = mParameterLayout->GetSequentialResourceIndex(set, slot, arrayIndex);
	if (sequentialArrayIndex == ~0u)
	{
		B3D_LOG(Warning, RenderBackend, "Unable to assign parameter. Cannot find sampler parameter with the set/slot combination: {0}/{1}", set, slot);
		return false;
	}

	mSamplerStates[sequentialArrayIndex] = sampler;

	MarkResourcesDirtyInternal();
	MarkRenderProxyDataDirtyInternal();

	return true;
}

template class TGpuParameters<false>;
template class TGpuParameters<true>;

template B3D_EXPORT void TGpuParameters<false>::GetParameter<float>(const StringView&, TGpuParameterPrimitive<float, false>&) const;
template B3D_EXPORT void TGpuParameters<false>::GetParameter<int>(const StringView&, TGpuParameterPrimitive<int, false>&) const;
template B3D_EXPORT void TGpuParameters<false>::GetParameter<Color>(const StringView&, TGpuParameterPrimitive<Color, false>&) const;
template B3D_EXPORT void TGpuParameters<false>::GetParameter<Vector2>(const StringView&, TGpuParameterPrimitive<Vector2, false>&) const;
template B3D_EXPORT void TGpuParameters<false>::GetParameter<Vector3>(const StringView&, TGpuParameterPrimitive<Vector3, false>&) const;
template B3D_EXPORT void TGpuParameters<false>::GetParameter<Vector4>( const StringView&, TGpuParameterPrimitive<Vector4, false>&) const;
template B3D_EXPORT void TGpuParameters<false>::GetParameter<Vector2I>(const StringView&, TGpuParameterPrimitive<Vector2I, false>&) const;
template B3D_EXPORT void TGpuParameters<false>::GetParameter<Vector3I>(const StringView&, TGpuParameterPrimitive<Vector3I, false>&) const;
template B3D_EXPORT void TGpuParameters<false>::GetParameter<Vector4I>(const StringView&, TGpuParameterPrimitive<Vector4I, false>&) const;
template B3D_EXPORT void TGpuParameters<false>::GetParameter<Matrix2>(const StringView&, TGpuParameterPrimitive<Matrix2, false>&) const;
template B3D_EXPORT void TGpuParameters<false>::GetParameter<Matrix2x3>(const StringView&, TGpuParameterPrimitive<Matrix2x3, false>&) const;
template B3D_EXPORT void TGpuParameters<false>::GetParameter<Matrix2x4>(const StringView&, TGpuParameterPrimitive<Matrix2x4, false>&) const;
template B3D_EXPORT void TGpuParameters<false>::GetParameter<Matrix3>(const StringView&, TGpuParameterPrimitive<Matrix3, false>&) const;
template B3D_EXPORT void TGpuParameters<false>::GetParameter<Matrix3x2>(const StringView&, TGpuParameterPrimitive<Matrix3x2, false>&) const;
template B3D_EXPORT void TGpuParameters<false>::GetParameter<Matrix3x4>(const StringView&, TGpuParameterPrimitive<Matrix3x4, false>&) const;
template B3D_EXPORT void TGpuParameters<false>::GetParameter<Matrix4>(const StringView&, TGpuParameterPrimitive<Matrix4, false>&) const;
template B3D_EXPORT void TGpuParameters<false>::GetParameter<Matrix4x2>(const StringView&, TGpuParameterPrimitive<Matrix4x2, false>&) const;
template B3D_EXPORT void TGpuParameters<false>::GetParameter<Matrix4x3>(const StringView&, TGpuParameterPrimitive<Matrix4x3, false>&) const;

template B3D_EXPORT void TGpuParameters<true>::GetParameter<float>(const StringView&, TGpuParameterPrimitive<float, true>&) const;
template B3D_EXPORT void TGpuParameters<true>::GetParameter<int>(const StringView&, TGpuParameterPrimitive<int, true>&) const;
template B3D_EXPORT void TGpuParameters<true>::GetParameter<Color>(const StringView&, TGpuParameterPrimitive<Color, true>&) const;
template B3D_EXPORT void TGpuParameters<true>::GetParameter<Vector2>(const StringView&, TGpuParameterPrimitive<Vector2, true>&) const;
template B3D_EXPORT void TGpuParameters<true>::GetParameter<Vector3>(const StringView&, TGpuParameterPrimitive<Vector3, true>&) const;
template B3D_EXPORT void TGpuParameters<true>::GetParameter<Vector4>(const StringView&, TGpuParameterPrimitive<Vector4, true>&) const;
template B3D_EXPORT void TGpuParameters<true>::GetParameter<Vector2I>(const StringView&, TGpuParameterPrimitive<Vector2I, true>&) const;
template B3D_EXPORT void TGpuParameters<true>::GetParameter<Vector3I>(const StringView&, TGpuParameterPrimitive<Vector3I, true>&) const;
template B3D_EXPORT void TGpuParameters<true>::GetParameter<Vector4I>(const StringView&, TGpuParameterPrimitive<Vector4I, true>&) const;
template B3D_EXPORT void TGpuParameters<true>::GetParameter<Matrix2>(const StringView&, TGpuParameterPrimitive<Matrix2, true>&) const;
template B3D_EXPORT void TGpuParameters<true>::GetParameter<Matrix2x3>(const StringView&, TGpuParameterPrimitive<Matrix2x3, true>&) const;
template B3D_EXPORT void TGpuParameters<true>::GetParameter<Matrix2x4>(const StringView&, TGpuParameterPrimitive<Matrix2x4, true>&) const;
template B3D_EXPORT void TGpuParameters<true>::GetParameter<Matrix3>(const StringView&, TGpuParameterPrimitive<Matrix3, true>&) const;
template B3D_EXPORT void TGpuParameters<true>::GetParameter<Matrix3x2>(const StringView&, TGpuParameterPrimitive<Matrix3x2, true>&) const;
template B3D_EXPORT void TGpuParameters<true>::GetParameter<Matrix3x4>(const StringView&, TGpuParameterPrimitive<Matrix3x4, true>&) const;
template B3D_EXPORT void TGpuParameters<true>::GetParameter<Matrix4>(const StringView&, TGpuParameterPrimitive<Matrix4, true>&) const;
template B3D_EXPORT void TGpuParameters<true>::GetParameter<Matrix4x2>(const StringView&, TGpuParameterPrimitive<Matrix4x2, true>&) const;
template B3D_EXPORT void TGpuParameters<true>::GetParameter<Matrix4x3>(const StringView&, TGpuParameterPrimitive<Matrix4x3, true>&) const;

namespace b3d
{
	B3D_SYNC_BLOCK_BEGIN(GpuParameters, SyncPacket)
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
const GpuDataParameterTypeInformationLookup GpuParameters::kParamSizes;

GpuParameters::GpuParameters(const SPtr<GpuPipelineParameterLayout>& parameterLayout)
	: TGpuParameters(parameterLayout)
{
}

SPtr<GpuParameters> GpuParameters::GetSelf() const
{
	return std::static_pointer_cast<GpuParameters>(GetShared());
}

SPtr<render::RenderProxy> GpuParameters::CreateRenderProxy() const
{
	const SPtr<GpuDevice>& gpuDevice = GetApplication().GetPrimaryGpuDevice();
	if(!gpuDevice)
		return nullptr;

	SPtr<GpuPipelineParameterLayout> parameterLayout = std::static_pointer_cast<GpuPipelineParameterLayout>(mParameterLayout);
	return gpuDevice->CreateGpuParameters(parameterLayout, true);
}

void GpuParameters::MarkRenderProxyDataDirtyInternal()
{
	MarkRenderProxyDataDirty();
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
	SPtr<GpuParameters> shared = B3DMakeSharedFromExisting<GpuParameters>(output);
	shared->SetShared(shared);
	shared->Initialize();

	return shared;
}

RenderProxySyncPacket* GpuParameters::CreateRenderProxySyncPacket(FrameAllocator& allocator, u32 flags)
{
	SyncPacket* const syncPacket = allocator.Construct<SyncPacket>(*this, allocator, flags);

	const u32 uniformBufferCount = mParameterLayout->GetResourceCount(GpuParameterType::UniformBuffer);
	syncPacket->UniformBufferOffsets.reserve(uniformBufferCount);
	syncPacket->UniformBufferOffsets.reserve(uniformBufferCount);

	for(u32 i = 0; i < uniformBufferCount; i++)
	{
		syncPacket->UniformBufferOffsets.push_back(mUniformBufferData->Offset);
		syncPacket->UniformBuffers.push_back(B3DGetRenderProxy(mUniformBufferData[i].Buffer));
	}

	const u32 sampledTextureCount = mParameterLayout->GetResourceCount(GpuParameterType::SampledTexture);
	syncPacket->SampledTextureSurfaces.reserve(sampledTextureCount);
	syncPacket->SampledTextures.reserve(sampledTextureCount);

	for(u32 i = 0; i < sampledTextureCount; i++)
	{
		syncPacket->SampledTextureSurfaces.push_back(mSampledTextureData[i].Surface);
		syncPacket->SampledTextures.push_back(B3DGetRenderProxy(mSampledTextureData[i].Texture));
	}

	const u32 storageTextureCount = mParameterLayout->GetResourceCount(GpuParameterType::StorageTexture);
	syncPacket->StorageTextureSurfaces.reserve(storageTextureCount);
	syncPacket->StorageTextures.reserve(storageTextureCount);

	for(u32 i = 0; i < storageTextureCount; i++)
	{
		syncPacket->StorageTextureSurfaces.push_back(mStorageTextureData[i].Surface);
		syncPacket->StorageTextures.push_back(B3DGetRenderProxy(mStorageTextureData[i].Texture));
	}

	const u32 storageBufferCount = mParameterLayout->GetResourceCount(GpuParameterType::StorageBuffer);
	syncPacket->StorageBufferViews.reserve(storageBufferCount);
	syncPacket->StorageBuffers.reserve(storageBufferCount);

	for(u32 i = 0; i < storageBufferCount; i++)
	{
		syncPacket->StorageBufferViews.push_back(mStorageBufferData[i].View);
		syncPacket->StorageBuffers.push_back(B3DGetRenderProxy(mStorageBufferData[i].Buffer));
	}

	const u32 samplerCount = mParameterLayout->GetResourceCount(GpuParameterType::Sampler);
	syncPacket->SamplerStates.reserve(samplerCount);

	for(u32 i = 0; i < samplerCount; i++)
		syncPacket->SamplerStates.push_back(mSamplerStates[i]);

	return syncPacket;
}

void GpuParameters::GetListenerResources(Vector<HResource>& resources)
{
	u32 numTextures = mParameterLayout->GetResourceCount(GpuParameterType::SampledTexture);
	u32 numStorageTextures = mParameterLayout->GetResourceCount(GpuParameterType::StorageTexture);

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

namespace b3d { namespace render
{
GpuParameters::GpuParameters(const SPtr<GpuPipelineParameterLayout>& parameterLayout)
	: TGpuParameters(parameterLayout)
{
}

SPtr<GpuParameters> GpuParameters::GetSelf() const
{
	return std::static_pointer_cast<GpuParameters>(GetShared());
}

void GpuParameters::SyncFromCoreObject(const CoreSyncData& data, FrameAllocator& allocator)
{
	auto* const syncPacket = data.GetSyncPacket<b3d::GpuParameters::SyncPacket>();
	if(!syncPacket)
		return;

	const u32 uniformBufferCount = mParameterLayout->GetResourceCount(GpuParameterType::UniformBuffer);
	const u32 sampledTextureCount = mParameterLayout->GetResourceCount(GpuParameterType::SampledTexture);
	const u32 storageTextureCount = mParameterLayout->GetResourceCount(GpuParameterType::StorageTexture);
	const u32 storageBufferCount = mParameterLayout->GetResourceCount(GpuParameterType::StorageBuffer);
	const u32 samplerCount = mParameterLayout->GetResourceCount(GpuParameterType::Sampler);

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
