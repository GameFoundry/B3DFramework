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

SPtr<GpuParamDesc> GpuParamsBase::GetParamDesc(GpuProgramType type) const
{
	return mParamInfo->GetParamDesc(type);
}

u32 GpuParamsBase::GetDataParamSize(GpuProgramType type, const String& name) const
{
	GpuParamDataDesc* desc = GetParamDesc(type, name);
	if(desc != nullptr)
		return desc->ElementSize * 4;

	return 0;
}

bool GpuParamsBase::HasParam(GpuProgramType type, const String& name) const
{
	return GetParamDesc(type, name) != nullptr;
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

bool GpuParamsBase::HasLoadStoreTexture(GpuProgramType type, const String& name) const
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
	const SPtr<GpuParamDesc>& paramDesc = mParamInfo->GetParamDesc(type);
	if(paramDesc == nullptr)
		return false;

	auto paramBlockIter = paramDesc->ParamBlocks.find(name);
	if(paramBlockIter != paramDesc->ParamBlocks.end())
		return true;

	return false;
}

GpuParamDataDesc* GpuParamsBase::GetParamDesc(GpuProgramType type, const String& name) const
{
	const SPtr<GpuParamDesc>& paramDesc = mParamInfo->GetParamDesc(type);
	if(paramDesc == nullptr)
		return nullptr;

	auto paramIter = paramDesc->Params.find(name);
	if(paramIter != paramDesc->Params.end())
		return &paramIter->second;

	return nullptr;
}

GpuParamBlockDesc* GpuParamsBase::GetParamBlockDesc(GpuProgramType type, const String& name) const
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
	u32 numParamBlocks = mParamInfo->GetNumElements(GpuPipelineParamInfo::ParamType::ParamBlock);
	u32 numTextures = mParamInfo->GetNumElements(GpuPipelineParamInfo::ParamType::Texture);
	u32 numStorageTextures = mParamInfo->GetNumElements(GpuPipelineParamInfo::ParamType::LoadStoreTexture);
	u32 numBuffers = mParamInfo->GetNumElements(GpuPipelineParamInfo::ParamType::Buffer);
	u32 numSamplers = mParamInfo->GetNumElements(GpuPipelineParamInfo::ParamType::SamplerState);

	u32 paramBlocksSize = sizeof(ParamsBufferType) * numParamBlocks;
	u32 texturesSize = (sizeof(TextureType) + sizeof(TextureSurface)) * numTextures;
	u32 loadStoreTexturesSize = (sizeof(TextureType) + sizeof(TextureSurface)) * numStorageTextures;
	u32 buffersSize = sizeof(BufferType) * numBuffers;
	u32 samplerStatesSize = sizeof(SamplerType) * numSamplers;

	u32 totalSize = paramBlocksSize + texturesSize + loadStoreTexturesSize + buffersSize + samplerStatesSize;

	u8* data = (u8*)bs_alloc(totalSize);
	mParamBlockBuffers = (ParamsBufferType*)data;
	for(u32 i = 0; i < numParamBlocks; i++)
		new(&mParamBlockBuffers[i]) ParamsBufferType();

	data += paramBlocksSize;
	mSampledTextureData = (TextureData*)data;
	for(u32 i = 0; i < numTextures; i++)
	{
		new(&mSampledTextureData[i].Texture) TextureType();
		new(&mSampledTextureData[i].Surface) TextureSurface(0, 0, 0, 0);
	}

	data += texturesSize;
	mLoadStoreTextureData = (TextureData*)data;
	for(u32 i = 0; i < numStorageTextures; i++)
	{
		new(&mLoadStoreTextureData[i].Texture) TextureType();
		new(&mLoadStoreTextureData[i].Surface) TextureSurface(0, 0, 0, 0);
	}

	data += loadStoreTexturesSize;
	mBuffers = (BufferType*)data;
	for(u32 i = 0; i < numBuffers; i++)
		new(&mBuffers[i]) BufferType();

	data += buffersSize;
	mSamplerStates = (SamplerType*)data;
	for(u32 i = 0; i < numSamplers; i++)
		new(&mSamplerStates[i]) SamplerType();

	data += samplerStatesSize;
}

template <bool Core>
TGpuParams<Core>::~TGpuParams()
{
	u32 numParamBlocks = mParamInfo->GetNumElements(GpuPipelineParamInfo::ParamType::ParamBlock);
	u32 numTextures = mParamInfo->GetNumElements(GpuPipelineParamInfo::ParamType::Texture);
	u32 numStorageTextures = mParamInfo->GetNumElements(GpuPipelineParamInfo::ParamType::LoadStoreTexture);
	u32 numBuffers = mParamInfo->GetNumElements(GpuPipelineParamInfo::ParamType::Buffer);
	u32 numSamplers = mParamInfo->GetNumElements(GpuPipelineParamInfo::ParamType::SamplerState);

	for(u32 i = 0; i < numParamBlocks; i++)
		mParamBlockBuffers[i].~ParamsBufferType();

	for(u32 i = 0; i < numTextures; i++)
	{
		mSampledTextureData[i].Texture.~TextureType();
		mSampledTextureData[i].Surface.~TextureSurface();
	}

	for(u32 i = 0; i < numStorageTextures; i++)
	{
		mLoadStoreTextureData[i].Texture.~TextureType();
		mLoadStoreTextureData[i].Surface.~TextureSurface();
	}

	for(u32 i = 0; i < numBuffers; i++)
		mBuffers[i].~BufferType();

	for(u32 i = 0; i < numSamplers; i++)
		mSamplerStates[i].~SamplerType();

	// Everything is allocated in a single block, so it's enough to free the first element
	bs_free(mParamBlockBuffers);
}

template <bool Core>
void TGpuParams<Core>::SetParamBlockBuffer(u32 set, u32 slot, const ParamsBufferType& paramBlockBuffer)
{
	u32 globalSlot = mParamInfo->GetSequentialSlot(GpuPipelineParamInfo::ParamType::ParamBlock, set, slot);
	if(globalSlot == (u32)-1)
		return;

	mParamBlockBuffers[globalSlot] = paramBlockBuffer;

	MarkCoreDirtyInternal();
}

template <bool Core>
void TGpuParams<Core>::SetParamBlockBuffer(GpuProgramType type, const String& name, const ParamsBufferType& paramBlockBuffer)
{
	const SPtr<GpuParamDesc>& paramDescs = mParamInfo->GetParamDesc(type);
	if(paramDescs == nullptr)
	{
		BS_LOG(Warning, RenderBackend, "Cannot find parameter block with the name: '{0}'", name);
		return;
	}

	auto iterFind = paramDescs->ParamBlocks.find(name);
	if(iterFind == paramDescs->ParamBlocks.end())
	{
		BS_LOG(Warning, RenderBackend, "Cannot find parameter block with the name: '{0}'", name);
		return;
	}

	SetParamBlockBuffer(iterFind->second.Set, iterFind->second.Slot, paramBlockBuffer);
}

template <bool Core>
void TGpuParams<Core>::SetParamBlockBuffer(const String& name, const ParamsBufferType& paramBlockBuffer)
{
	for(u32 i = 0; i < 6; i++)
	{
		const SPtr<GpuParamDesc>& paramDescs = mParamInfo->GetParamDesc((GpuProgramType)i);
		if(paramDescs == nullptr)
			continue;

		auto iterFind = paramDescs->ParamBlocks.find(name);
		if(iterFind == paramDescs->ParamBlocks.end())
			continue;

		SetParamBlockBuffer(iterFind->second.Set, iterFind->second.Slot, paramBlockBuffer);
	}
}

template <bool Core>
template <class T>
void TGpuParams<Core>::GetParam(GpuProgramType type, const String& name, TGpuDataParam<T, Core>& output) const
{
	const SPtr<GpuParamDesc>& paramDescs = mParamInfo->GetParamDesc(type);
	if(paramDescs == nullptr)
	{
		output = TGpuDataParam<T, Core>(nullptr, nullptr);
		BS_LOG(Warning, RenderBackend, "Cannot find parameter with the name: '{0}'", name);
		return;
	}

	auto iterFind = paramDescs->Params.find(name);
	if(iterFind == paramDescs->Params.end())
	{
		output = TGpuDataParam<T, Core>(nullptr, nullptr);
		BS_LOG(Warning, RenderBackend, "Cannot find parameter with the name: '{0}'", name);
	}
	else
		output = TGpuDataParam<T, Core>(&iterFind->second, GetThisPtrInternal());
}

template <bool Core>
void TGpuParams<Core>::GetStructParam(GpuProgramType type, const String& name, TGpuParamStruct<Core>& output) const
{
	const SPtr<GpuParamDesc>& paramDescs = mParamInfo->GetParamDesc(type);
	if(paramDescs == nullptr)
	{
		output = TGpuParamStruct<Core>(nullptr, nullptr);
		BS_LOG(Warning, RenderBackend, "Cannot find struct parameter with the name: '{0}'", name);
		return;
	}

	auto iterFind = paramDescs->Params.find(name);
	if(iterFind == paramDescs->Params.end() || iterFind->second.Type != GPDT_STRUCT)
	{
		output = TGpuParamStruct<Core>(nullptr, nullptr);
		BS_LOG(Warning, RenderBackend, "Cannot find struct parameter with the name: '{0}'", name);
	}
	else
		output = TGpuParamStruct<Core>(&iterFind->second, GetThisPtrInternal());
}

template <bool Core>
void TGpuParams<Core>::GetTextureParam(GpuProgramType type, const String& name, TGpuParamTexture<Core>& output) const
{
	const SPtr<GpuParamDesc>& paramDescs = mParamInfo->GetParamDesc(type);
	if(paramDescs == nullptr)
	{
		output = TGpuParamTexture<Core>(nullptr, nullptr);
		BS_LOG(Warning, RenderBackend, "Cannot find texture parameter with the name: '{0}'", name);
		return;
	}

	auto iterFind = paramDescs->Textures.find(name);
	if(iterFind == paramDescs->Textures.end())
	{
		output = TGpuParamTexture<Core>(nullptr, nullptr);
		BS_LOG(Warning, RenderBackend, "Cannot find texture parameter with the name: '{0}'", name);
	}
	else
		output = TGpuParamTexture<Core>(&iterFind->second, GetThisPtrInternal());
}

template <bool Core>
void TGpuParams<Core>::GetLoadStoreTextureParam(GpuProgramType type, const String& name, TGpuParamLoadStoreTexture<Core>& output) const
{
	const SPtr<GpuParamDesc>& paramDescs = mParamInfo->GetParamDesc(type);
	if(paramDescs == nullptr)
	{
		output = TGpuParamLoadStoreTexture<Core>(nullptr, nullptr);
		BS_LOG(Warning, RenderBackend, "Cannot find load-store parameter with the name: '{0}'", name);
		return;
	}

	auto iterFind = paramDescs->LoadStoreTextures.find(name);
	if(iterFind == paramDescs->LoadStoreTextures.end())
	{
		output = TGpuParamLoadStoreTexture<Core>(nullptr, nullptr);
		BS_LOG(Warning, RenderBackend, "Cannot find load-store parameter with the name: '{0}'", name);
	}
	else
		output = TGpuParamLoadStoreTexture<Core>(&iterFind->second, GetThisPtrInternal());
}

template <bool Core>
void TGpuParams<Core>::GetBufferParam(GpuProgramType type, const String& name, TGpuParamBuffer<Core>& output) const
{
	const SPtr<GpuParamDesc>& paramDescs = mParamInfo->GetParamDesc(type);
	if(paramDescs == nullptr)
	{
		output = TGpuParamBuffer<Core>(nullptr, nullptr);
		BS_LOG(Warning, RenderBackend, "Cannot find buffer parameter with the name: '{0}'", name);
		return;
	}

	auto iterFind = paramDescs->Buffers.find(name);
	if(iterFind == paramDescs->Buffers.end())
	{
		output = TGpuParamBuffer<Core>(nullptr, nullptr);
		BS_LOG(Warning, RenderBackend, "Cannot find buffer parameter with the name: '{0}'", name);
	}
	else
		output = TGpuParamBuffer<Core>(&iterFind->second, GetThisPtrInternal());
}

template <bool Core>
void TGpuParams<Core>::GetSamplerStateParam(GpuProgramType type, const String& name, TGpuParamSampState<Core>& output) const
{
	const SPtr<GpuParamDesc>& paramDescs = mParamInfo->GetParamDesc(type);
	if(paramDescs == nullptr)
	{
		output = TGpuParamSampState<Core>(nullptr, nullptr);
		BS_LOG(Warning, RenderBackend, "Cannot find sampler state parameter with the name: '{0}'", name);
		return;
	}

	auto iterFind = paramDescs->Samplers.find(name);
	if(iterFind == paramDescs->Samplers.end())
	{
		output = TGpuParamSampState<Core>(nullptr, nullptr);
		BS_LOG(Warning, RenderBackend, "Cannot find sampler state parameter with the name: '{0}'", name);
	}
	else
		output = TGpuParamSampState<Core>(&iterFind->second, GetThisPtrInternal());
}

template <bool Core>
typename TGpuParams<Core>::ParamsBufferType TGpuParams<Core>::GetParamBlockBuffer(u32 set, u32 slot) const
{
	u32 globalSlot = mParamInfo->GetSequentialSlot(GpuPipelineParamInfo::ParamType::ParamBlock, set, slot);
	if(globalSlot == (u32)-1)
		return nullptr;

	return mParamBlockBuffers[globalSlot];
}

template <bool Core>
typename TGpuParams<Core>::TextureType TGpuParams<Core>::GetTexture(u32 set, u32 slot) const
{
	u32 globalSlot = mParamInfo->GetSequentialSlot(GpuPipelineParamInfo::ParamType::Texture, set, slot);
	if(globalSlot == (u32)-1)
		return TGpuParams<Core>::TextureType();

	return mSampledTextureData[globalSlot].Texture;
}

template <bool Core>
typename TGpuParams<Core>::TextureType TGpuParams<Core>::GetLoadStoreTexture(u32 set, u32 slot) const
{
	u32 globalSlot = mParamInfo->GetSequentialSlot(GpuPipelineParamInfo::ParamType::LoadStoreTexture, set, slot);
	if(globalSlot == (u32)-1)
		return TGpuParams<Core>::TextureType();

	return mLoadStoreTextureData[globalSlot].Texture;
}

template <bool Core>
typename TGpuParams<Core>::BufferType TGpuParams<Core>::GetBuffer(u32 set, u32 slot) const
{
	u32 globalSlot = mParamInfo->GetSequentialSlot(GpuPipelineParamInfo::ParamType::Buffer, set, slot);
	if(globalSlot == (u32)-1)
		return nullptr;

	return mBuffers[globalSlot];
}

template <bool Core>
typename TGpuParams<Core>::SamplerType TGpuParams<Core>::GetSamplerState(u32 set, u32 slot) const
{
	u32 globalSlot = mParamInfo->GetSequentialSlot(GpuPipelineParamInfo::ParamType::SamplerState, set, slot);
	if(globalSlot == (u32)-1)
		return nullptr;

	return mSamplerStates[globalSlot];
}

template <bool Core>
const TextureSurface& TGpuParams<Core>::GetTextureSurface(u32 set, u32 slot) const
{
	static TextureSurface emptySurface;

	u32 globalSlot = mParamInfo->GetSequentialSlot(GpuPipelineParamInfo::ParamType::Texture, set, slot);
	if(globalSlot == (u32)-1)
		return emptySurface;

	return mSampledTextureData[globalSlot].Surface;
}

template <bool Core>
const TextureSurface& TGpuParams<Core>::GetLoadStoreSurface(u32 set, u32 slot) const
{
	static TextureSurface emptySurface;

	u32 globalSlot = mParamInfo->GetSequentialSlot(GpuPipelineParamInfo::ParamType::LoadStoreTexture, set, slot);
	if(globalSlot == (u32)-1)
		return emptySurface;

	return mLoadStoreTextureData[globalSlot].Surface;
}

template <bool Core>
void TGpuParams<Core>::SetTexture(u32 set, u32 slot, const TextureType& texture, const TextureSurface& surface)
{
	u32 globalSlot = mParamInfo->GetSequentialSlot(GpuPipelineParamInfo::ParamType::Texture, set, slot);
	if(globalSlot == (u32)-1)
		return;

	mSampledTextureData[globalSlot].Texture = texture;
	mSampledTextureData[globalSlot].Surface = surface;

	MarkResourcesDirtyInternal();
	MarkCoreDirtyInternal();
}

template <bool Core>
void TGpuParams<Core>::SetLoadStoreTexture(u32 set, u32 slot, const TextureType& texture, const TextureSurface& surface)
{
	u32 globalSlot = mParamInfo->GetSequentialSlot(GpuPipelineParamInfo::ParamType::LoadStoreTexture, set, slot);
	if(globalSlot == (u32)-1)
		return;

	mLoadStoreTextureData[globalSlot].Texture = texture;
	mLoadStoreTextureData[globalSlot].Surface = surface;

	MarkResourcesDirtyInternal();
	MarkCoreDirtyInternal();
}

template <bool Core>
void TGpuParams<Core>::SetBuffer(u32 set, u32 slot, const BufferType& buffer)
{
	u32 globalSlot = mParamInfo->GetSequentialSlot(GpuPipelineParamInfo::ParamType::Buffer, set, slot);
	if(globalSlot == (u32)-1)
		return;

	mBuffers[globalSlot] = buffer;

	MarkResourcesDirtyInternal();
	MarkCoreDirtyInternal();
}

template <bool Core>
void TGpuParams<Core>::SetSamplerState(u32 set, u32 slot, const SamplerType& sampler)
{
	u32 globalSlot = mParamInfo->GetSequentialSlot(GpuPipelineParamInfo::ParamType::SamplerState, set, slot);
	if(globalSlot == (u32)-1)
		return;

	mSamplerStates[globalSlot] = sampler;

	MarkResourcesDirtyInternal();
	MarkCoreDirtyInternal();
}

template class TGpuParams<false>;
template class TGpuParams<true>;

template BS_CORE_EXPORT void TGpuParams<false>::GetParam<float>(GpuProgramType type, const String&, TGpuDataParam<float, false>&) const;
template BS_CORE_EXPORT void TGpuParams<false>::GetParam<int>(GpuProgramType type, const String&, TGpuDataParam<int, false>&) const;
template BS_CORE_EXPORT void TGpuParams<false>::GetParam<Color>(GpuProgramType type, const String&, TGpuDataParam<Color, false>&) const;
template BS_CORE_EXPORT void TGpuParams<false>::GetParam<Vector2>(GpuProgramType type, const String&, TGpuDataParam<Vector2, false>&) const;
template BS_CORE_EXPORT void TGpuParams<false>::GetParam<Vector3>(GpuProgramType type, const String&, TGpuDataParam<Vector3, false>&) const;
template BS_CORE_EXPORT void TGpuParams<false>::GetParam<Vector4>(GpuProgramType type, const String&, TGpuDataParam<Vector4, false>&) const;
template BS_CORE_EXPORT void TGpuParams<false>::GetParam<Vector2I>(GpuProgramType type, const String&, TGpuDataParam<Vector2I, false>&) const;
template BS_CORE_EXPORT void TGpuParams<false>::GetParam<Vector3I>(GpuProgramType type, const String&, TGpuDataParam<Vector3I, false>&) const;
template BS_CORE_EXPORT void TGpuParams<false>::GetParam<Vector4I>(GpuProgramType type, const String&, TGpuDataParam<Vector4I, false>&) const;
template BS_CORE_EXPORT void TGpuParams<false>::GetParam<Matrix2>(GpuProgramType type, const String&, TGpuDataParam<Matrix2, false>&) const;
template BS_CORE_EXPORT void TGpuParams<false>::GetParam<Matrix2x3>(GpuProgramType type, const String&, TGpuDataParam<Matrix2x3, false>&) const;
template BS_CORE_EXPORT void TGpuParams<false>::GetParam<Matrix2x4>(GpuProgramType type, const String&, TGpuDataParam<Matrix2x4, false>&) const;
template BS_CORE_EXPORT void TGpuParams<false>::GetParam<Matrix3>(GpuProgramType type, const String&, TGpuDataParam<Matrix3, false>&) const;
template BS_CORE_EXPORT void TGpuParams<false>::GetParam<Matrix3x2>(GpuProgramType type, const String&, TGpuDataParam<Matrix3x2, false>&) const;
template BS_CORE_EXPORT void TGpuParams<false>::GetParam<Matrix3x4>(GpuProgramType type, const String&, TGpuDataParam<Matrix3x4, false>&) const;
template BS_CORE_EXPORT void TGpuParams<false>::GetParam<Matrix4>(GpuProgramType type, const String&, TGpuDataParam<Matrix4, false>&) const;
template BS_CORE_EXPORT void TGpuParams<false>::GetParam<Matrix4x2>(GpuProgramType type, const String&, TGpuDataParam<Matrix4x2, false>&) const;
template BS_CORE_EXPORT void TGpuParams<false>::GetParam<Matrix4x3>(GpuProgramType type, const String&, TGpuDataParam<Matrix4x3, false>&) const;

template BS_CORE_EXPORT void TGpuParams<true>::GetParam<float>(GpuProgramType type, const String&, TGpuDataParam<float, true>&) const;
template BS_CORE_EXPORT void TGpuParams<true>::GetParam<int>(GpuProgramType type, const String&, TGpuDataParam<int, true>&) const;
template BS_CORE_EXPORT void TGpuParams<true>::GetParam<Color>(GpuProgramType type, const String&, TGpuDataParam<Color, true>&) const;
template BS_CORE_EXPORT void TGpuParams<true>::GetParam<Vector2>(GpuProgramType type, const String&, TGpuDataParam<Vector2, true>&) const;
template BS_CORE_EXPORT void TGpuParams<true>::GetParam<Vector3>(GpuProgramType type, const String&, TGpuDataParam<Vector3, true>&) const;
template BS_CORE_EXPORT void TGpuParams<true>::GetParam<Vector4>(GpuProgramType type, const String&, TGpuDataParam<Vector4, true>&) const;
template BS_CORE_EXPORT void TGpuParams<true>::GetParam<Vector2I>(GpuProgramType type, const String&, TGpuDataParam<Vector2I, true>&) const;
template BS_CORE_EXPORT void TGpuParams<true>::GetParam<Vector3I>(GpuProgramType type, const String&, TGpuDataParam<Vector3I, true>&) const;
template BS_CORE_EXPORT void TGpuParams<true>::GetParam<Vector4I>(GpuProgramType type, const String&, TGpuDataParam<Vector4I, true>&) const;
template BS_CORE_EXPORT void TGpuParams<true>::GetParam<Matrix2>(GpuProgramType type, const String&, TGpuDataParam<Matrix2, true>&) const;
template BS_CORE_EXPORT void TGpuParams<true>::GetParam<Matrix2x3>(GpuProgramType type, const String&, TGpuDataParam<Matrix2x3, true>&) const;
template BS_CORE_EXPORT void TGpuParams<true>::GetParam<Matrix2x4>(GpuProgramType type, const String&, TGpuDataParam<Matrix2x4, true>&) const;
template BS_CORE_EXPORT void TGpuParams<true>::GetParam<Matrix3>(GpuProgramType type, const String&, TGpuDataParam<Matrix3, true>&) const;
template BS_CORE_EXPORT void TGpuParams<true>::GetParam<Matrix3x2>(GpuProgramType type, const String&, TGpuDataParam<Matrix3x2, true>&) const;
template BS_CORE_EXPORT void TGpuParams<true>::GetParam<Matrix3x4>(GpuProgramType type, const String&, TGpuDataParam<Matrix3x4, true>&) const;
template BS_CORE_EXPORT void TGpuParams<true>::GetParam<Matrix4>(GpuProgramType type, const String&, TGpuDataParam<Matrix4, true>&) const;
template BS_CORE_EXPORT void TGpuParams<true>::GetParam<Matrix4x2>(GpuProgramType type, const String&, TGpuDataParam<Matrix4x2, true>&) const;
template BS_CORE_EXPORT void TGpuParams<true>::GetParam<Matrix4x3>(GpuProgramType type, const String&, TGpuDataParam<Matrix4x3, true>&) const;

const GpuDataParamInfos GpuParams::kParamSizes;

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
	u32 numParamBlocks = mParamInfo->GetNumElements(GpuPipelineParamInfo::ParamType::ParamBlock);
	u32 numTextures = mParamInfo->GetNumElements(GpuPipelineParamInfo::ParamType::Texture);
	u32 numStorageTextures = mParamInfo->GetNumElements(GpuPipelineParamInfo::ParamType::LoadStoreTexture);
	u32 numBuffers = mParamInfo->GetNumElements(GpuPipelineParamInfo::ParamType::Buffer);
	u32 numSamplers = mParamInfo->GetNumElements(GpuPipelineParamInfo::ParamType::SamplerState);

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
		loadStoreSurfaces[i] = mLoadStoreTextureData[i].Surface;

		new(&loadStoreTextures[i]) SPtr<ct::Texture>();

		if(mLoadStoreTextureData[i].Texture.IsLoaded())
			loadStoreTextures[i] = mLoadStoreTextureData[i].Texture->GetCore();
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
	u32 numTextures = mParamInfo->GetNumElements(GpuPipelineParamInfo::ParamType::Texture);
	u32 numStorageTextures = mParamInfo->GetNumElements(GpuPipelineParamInfo::ParamType::LoadStoreTexture);

	for(u32 i = 0; i < numTextures; i++)
	{
		if(mSampledTextureData[i].Texture != nullptr)
			resources.push_back(mSampledTextureData[i].Texture);
	}

	for(u32 i = 0; i < numStorageTextures; i++)
	{
		if(mLoadStoreTextureData[i].Texture != nullptr)
			resources.push_back(mLoadStoreTextureData[i].Texture);
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
	u32 numParamBlocks = mParamInfo->GetNumElements(GpuPipelineParamInfo::ParamType::ParamBlock);
	u32 numTextures = mParamInfo->GetNumElements(GpuPipelineParamInfo::ParamType::Texture);
	u32 numStorageTextures = mParamInfo->GetNumElements(GpuPipelineParamInfo::ParamType::LoadStoreTexture);
	u32 numBuffers = mParamInfo->GetNumElements(GpuPipelineParamInfo::ParamType::Buffer);
	u32 numSamplers = mParamInfo->GetNumElements(GpuPipelineParamInfo::ParamType::SamplerState);

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

	assert(data.GetBufferSize() == totalSize);

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
		mLoadStoreTextureData[i].Surface = loadStoreSurfaces[i];
		loadStoreSurfaces[i].~TextureSurface();

		mLoadStoreTextureData[i].Texture = loadStoreTextures[i];
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
