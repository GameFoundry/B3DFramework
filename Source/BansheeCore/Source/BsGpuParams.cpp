//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsGpuParams.h"
#include "BsGpuParamDesc.h"
#include "BsGpuParamBlockBuffer.h"
#include "BsVector2.h"
#include "BsTexture.h"
#include "BsGpuBuffer.h"
#include "BsSamplerState.h"
#include "BsFrameAlloc.h"
#include "BsDebug.h"
#include "BsException.h"
#include "BsVectorNI.h"
#include "BsMatrixNxM.h"
#include "BsHardwareBufferManager.h"

namespace BansheeEngine
{
	GpuParamsBase::GpuParamsBase(const GPU_PARAMS_DESC& desc)
	{
		mParamDescs[GPT_FRAGMENT_PROGRAM] = desc.fragmentParams;
		mParamDescs[GPT_VERTEX_PROGRAM] = desc.vertexParams;
		mParamDescs[GPT_GEOMETRY_PROGRAM] = desc.geometryParams;
		mParamDescs[GPT_HULL_PROGRAM] = desc.hullParams;
		mParamDescs[GPT_DOMAIN_PROGRAM] = desc.domainParams;
		mParamDescs[GPT_COMPUTE_PROGRAM] = desc.computeParams;
	}

	GpuParamsBase::~GpuParamsBase()
	{ }

UINT32 GpuParamsBase::getDataParamSize(GpuProgramType type, const String& name) const
	{
		GpuParamDataDesc* desc = getParamDesc(type, name);
		if(desc != nullptr)
			return desc->elementSize * 4;

		return 0;
	}

	bool GpuParamsBase::hasParam(GpuProgramType type, const String& name) const
	{
		return getParamDesc(type, name) != nullptr;
	}

	bool GpuParamsBase::hasTexture(GpuProgramType type, const String& name) const
	{
		const SPtr<GpuParamDesc>& paramDesc = mParamDescs[(int)type];
		if (paramDesc == nullptr)
			return false;

		auto paramIter = paramDesc->textures.find(name);
		if(paramIter != paramDesc->textures.end())
			return true;

		return false;
	}

	bool GpuParamsBase::hasBuffer(GpuProgramType type, const String& name) const
	{
		const SPtr<GpuParamDesc>& paramDesc = mParamDescs[(int)type];
		if (paramDesc == nullptr)
			return false;

		auto paramIter = paramDesc->buffers.find(name);
		if (paramIter != paramDesc->buffers.end())
			return true;

		return false;
	}

	bool GpuParamsBase::hasLoadStoreTexture(GpuProgramType type, const String& name) const
	{
		const SPtr<GpuParamDesc>& paramDesc = mParamDescs[(int)type];
		if (paramDesc == nullptr)
			return false;

		auto paramIter = paramDesc->loadStoreTextures.find(name);
		if (paramIter != paramDesc->loadStoreTextures.end())
			return true;

		return false;
	}

	bool GpuParamsBase::hasSamplerState(GpuProgramType type, const String& name) const
	{
		const SPtr<GpuParamDesc>& paramDesc = mParamDescs[(int)type];
		if (paramDesc == nullptr)
			return false;

		auto paramIter = paramDesc->samplers.find(name);
		if(paramIter != paramDesc->samplers.end())
			return true;

		return false;
	}

	bool GpuParamsBase::hasParamBlock(GpuProgramType type, const String& name) const
	{
		const SPtr<GpuParamDesc>& paramDesc = mParamDescs[(int)type];
		if (paramDesc == nullptr)
			return false;

		auto paramBlockIter = paramDesc->paramBlocks.find(name);
		if(paramBlockIter != paramDesc->paramBlocks.end())
			return true;

		return false;
	}

	GpuParamDataDesc* GpuParamsBase::getParamDesc(GpuProgramType type, const String& name) const
	{
		const SPtr<GpuParamDesc>& paramDesc = mParamDescs[(int)type];
		if (paramDesc == nullptr)
			return nullptr;

		auto paramIter = paramDesc->params.find(name);
		if (paramIter != paramDesc->params.end())
			return &paramIter->second;

		return nullptr;
	}

	GpuParamBlockDesc* GpuParamsBase::getParamBlockDesc(GpuProgramType type, const String& name) const
	{
		const SPtr<GpuParamDesc>& paramDesc = mParamDescs[(int)type];
		if (paramDesc == nullptr)
			return nullptr;

		auto paramBlockIter = paramDesc->paramBlocks.find(name);
		if (paramBlockIter != paramDesc->paramBlocks.end())
			return &paramBlockIter->second;

		return nullptr;
	}

	template<bool Core>
	TGpuParams<Core>::TGpuParams(const GPU_PARAMS_DESC& desc)
		: GpuParamsBase(desc)
	{
		for(UINT32 i = 0; i < (UINT32)ElementType::Count; i++)
		{
			mNumSets[i] = 0;
			mNumElements[i] = 0;
			mOffsets[i] = nullptr;
		}

		UINT32 numParamDescs = sizeof(mParamDescs) / sizeof(mParamDescs[0]);
		for (UINT32 i = 0; i < numParamDescs; i++)
		{
			const SPtr<GpuParamDesc>& paramDesc = mParamDescs[i];
			if (paramDesc == nullptr)
				continue;

			for (auto& paramBlock : paramDesc->paramBlocks)
			{
				if ((paramBlock.second.set + 1) > mNumSets[(int)ElementType::ParamBlock])
					mNumSets[(int)ElementType::ParamBlock] = paramBlock.second.set + 1;
			}

			for (auto& texture : paramDesc->textures)
			{
				if ((texture.second.set + 1) > mNumSets[(int)ElementType::Texture])
					mNumSets[(int)ElementType::Texture] = texture.second.set + 1;
			}

			for (auto& texture : paramDesc->loadStoreTextures)
			{
				if ((texture.second.set + 1) > mNumSets[(int)ElementType::LoadStoreTexture])
					mNumSets[(int)ElementType::LoadStoreTexture] = texture.second.set + 1;
			}

			for (auto& buffer : paramDesc->buffers)
			{
				if ((buffer.second.set + 1) > mNumSets[(int)ElementType::Buffer])
					mNumSets[(int)ElementType::Buffer] = buffer.second.set + 1;
			}

			for (auto& sampler : paramDesc->samplers)
			{
				if ((sampler.second.set + 1) > mNumSets[(int)ElementType::SamplerState])
					mNumSets[(int)ElementType::SamplerState] = sampler.second.set + 1;
			}
		}

		UINT32 totalNumSets = 0;
		for (UINT32 i = 0; i < (UINT32)ElementType::Count; i++)
			totalNumSets += mNumSets[i];

		UINT32* slotsPerSetData = bs_stack_alloc<UINT32>(totalNumSets);
		memset(slotsPerSetData, 0, sizeof(UINT32) * totalNumSets);

		UINT32* slotsPerSet[(UINT32)ElementType::Count];
		for (UINT32 i = 0; i < (UINT32)ElementType::Count; i++)
		{
			if (i == 0)
				slotsPerSet[i] = slotsPerSetData;
			else
				slotsPerSet[i] = slotsPerSet[i - 1] + mNumSets[i - 1];
		}

		for (UINT32 i = 0; i < numParamDescs; i++)
		{
			const SPtr<GpuParamDesc>& paramDesc = mParamDescs[i];
			if (paramDesc == nullptr)
				continue;
			
			for (auto& paramBlock : paramDesc->paramBlocks)
			{
				UINT32* slots = slotsPerSet[(int)ElementType::ParamBlock];
				slots[paramBlock.second.set] = std::max(slots[paramBlock.second.set], paramBlock.second.slot + 1);
			}

			for (auto& texture : paramDesc->textures)
			{
				UINT32* slots = slotsPerSet[(int)ElementType::Texture];
				slots[texture.second.set] = std::max(slots[texture.second.set], texture.second.slot + 1);
			}

			for (auto& texture : paramDesc->loadStoreTextures)
			{
				UINT32* slots = slotsPerSet[(int)ElementType::LoadStoreTexture];
				slots[texture.second.set] = std::max(slots[texture.second.set], texture.second.slot + 1);
			}

			for (auto& buffer : paramDesc->buffers)
			{
				UINT32* slots = slotsPerSet[(int)ElementType::Buffer];
				slots[buffer.second.set] = std::max(slots[buffer.second.set], buffer.second.slot + 1);
			}

			for (auto& sampler : paramDesc->samplers)
			{
				UINT32* slots = slotsPerSet[(int)ElementType::SamplerState];
				slots[sampler.second.set] = std::max(slots[sampler.second.set], sampler.second.slot + 1);
			}
		}

		for (UINT32 i = 0; i < (UINT32)ElementType::Count; i++)
		{
			for (UINT32 j = 0; j < mNumSets[i]; j++)
				mNumElements[i] += slotsPerSet[i][j];
		}

		UINT32 paramBlocksSize = sizeof(ParamsBufferType) * mNumElements[(int)ElementType::ParamBlock];
		UINT32 texturesSize = sizeof(TextureType) * mNumElements[(int)ElementType::Texture];
		UINT32 loadStoreTexturesSize = sizeof(TextureType) * mNumElements[(int)ElementType::LoadStoreTexture];
		UINT32 loadStoreSurfacesSize = sizeof(TextureSurface) * mNumElements[(int)ElementType::LoadStoreTexture];
		UINT32 buffersSize = sizeof(BufferType) * mNumElements[(int)ElementType::Buffer];
		UINT32 samplerStatesSize = sizeof(SamplerType) * mNumElements[(int)ElementType::SamplerState];
		UINT32 setOffsetsSize = sizeof(UINT32) * totalNumSets;

		UINT32 totalSize = paramBlocksSize + texturesSize + loadStoreTexturesSize + loadStoreSurfacesSize +
			buffersSize + samplerStatesSize + setOffsetsSize;

		UINT8* data = (UINT8*)bs_alloc(totalSize);
		mParamBlockBuffers = (ParamsBufferType*)data;
		for (UINT32 i = 0; i < mNumElements[(int)ElementType::ParamBlock]; i++)
			new (&mParamBlockBuffers[i]) ParamsBufferType();

		data += sizeof(ParamsBufferType) * mNumElements[(int)ElementType::ParamBlock];
		mTextures = (TextureType*)data;
		for (UINT32 i = 0; i < mNumElements[(int)ElementType::Texture]; i++)
			new (&mTextures[i]) TextureType();

		data += sizeof(TextureType) * mNumElements[(int)ElementType::Texture];
		mLoadStoreTextures = (TextureType*)data;
		for (UINT32 i = 0; i < mNumElements[(int)ElementType::LoadStoreTexture]; i++)
			new (&mLoadStoreTextures[i]) TextureType();

		data += sizeof(TextureType) * mNumElements[(int)ElementType::LoadStoreTexture];
		mLoadStoreSurfaces = (TextureSurface*)data;
		for (UINT32 i = 0; i < mNumElements[(int)ElementType::LoadStoreTexture]; i++)
			new (&mLoadStoreSurfaces[i]) TextureSurface();

		data += sizeof(TextureSurface) * mNumElements[(int)ElementType::LoadStoreTexture];
		mBuffers = (BufferType*)data;
		for (UINT32 i = 0; i < mNumElements[(int)ElementType::Buffer]; i++)
			new (&mBuffers[i]) BufferType();

		data += sizeof(BufferType) * mNumElements[(int)ElementType::Buffer];
		mSamplerStates = (SamplerType*)data;
		for (UINT32 i = 0; i < mNumElements[(int)ElementType::SamplerState]; i++)
			new (&mSamplerStates[i]) SamplerType();

		data += sizeof(SamplerType) * mNumElements[(int)ElementType::SamplerState];

		for (UINT32 i = 0; i < (UINT32)ElementType::Count; i++)
		{
			mOffsets[i] = (UINT32*)data;
			data += sizeof(UINT32) * mNumSets[i];

			if (mNumSets[i] == 0)
				continue;

			mOffsets[i][0] = 0;

			for (UINT32 j = 0; j < mNumSets[i] - 1; j++)
				mOffsets[i][j + 1] = mOffsets[i][j] + slotsPerSet[i][j];
		}

		bs_stack_free(slotsPerSetData);
	}

	template<bool Core>
	TGpuParams<Core>::~TGpuParams()
	{
		for (UINT32 i = 0; i < mNumElements[(int)ElementType::ParamBlock]; i++)
			mParamBlockBuffers[i].~ParamsBufferType();

		for (UINT32 i = 0; i <  mNumElements[(int)ElementType::Texture]; i++)
			mTextures[i].~TextureType();

		for (UINT32 i = 0; i <  mNumElements[(int)ElementType::LoadStoreTexture]; i++)
		{
			mLoadStoreTextures[i].~TextureType();
			mLoadStoreSurfaces[i].~TextureSurface();
		}

		for (UINT32 i = 0; i < mNumElements[(int)ElementType::Buffer]; i++)
			mBuffers[i].~BufferType();

		for (UINT32 i = 0; i < mNumElements[(int)ElementType::SamplerState]; i++)
			mSamplerStates[i].~SamplerType();

		// Everything is allocated in a single block, so it's enough to free the first element
		bs_free(mParamBlockBuffers);
	}

	template<bool Core>
	UINT32 TGpuParams<Core>::getGlobalSlot(ElementType type, UINT32 set, UINT32 slot) const
	{
#if BS_DEBUG_MODE
		if (set >= mNumSets[(int)type])
		{
			LOGERR("Set index out of range: Valid range: 0 .. " +
				toString(mNumSets[(int)type] - 1) + ". Requested: " + toString(set));
			return (UINT32)-1;
		}
#endif

		UINT32 globalSlot = mOffsets[(int)type][set] + slot;

#if BS_DEBUG_MODE
		if (globalSlot >= mNumElements[(int)type])
		{
			UINT32 maxSlot;
			if (set < (mNumSets[(int)type] - 1))
				maxSlot = mOffsets[(int)type][set + 1];
			else
				maxSlot = mNumElements[(int)type];

			maxSlot -= mOffsets[(int)type][set];

			LOGERR("Slot index out of range: Valid range: 0 .. " +
				toString(maxSlot - 1) + ". Requested: " + toString(slot));
			return (UINT32)-1;
		}
#endif

		return globalSlot;
	}

	template<bool Core>
	void TGpuParams<Core>::setParamBlockBuffer(UINT32 set, UINT32 slot, const ParamsBufferType& paramBlockBuffer)
	{
		UINT32 globalSlot = getGlobalSlot(ElementType::ParamBlock, set, slot);
		if (globalSlot == (UINT32)-1)
			return;

		mParamBlockBuffers[globalSlot] = paramBlockBuffer;

		_markCoreDirty();
	}

	template<bool Core>
	void TGpuParams<Core>::setParamBlockBuffer(GpuProgramType type, const String& name, const ParamsBufferType& paramBlockBuffer)
	{
		const SPtr<GpuParamDesc>& paramDescs = mParamDescs[(int)type];
		if(paramDescs == nullptr)
		{
			LOGWRN("Cannot find parameter block with the name: '" + name + "'");
			return;
		}

		auto iterFind = paramDescs->paramBlocks.find(name);
		if (iterFind == paramDescs->paramBlocks.end())
		{
			LOGWRN("Cannot find parameter block with the name: '" + name + "'");
			return;
		}

		setParamBlockBuffer(iterFind->second.set, iterFind->second.slot, paramBlockBuffer);
	}

	template<bool Core>
	template<class T> 
	void TGpuParams<Core>::getParam(GpuProgramType type, const String& name, TGpuDataParam<T, Core>& output) const
	{
		const SPtr<GpuParamDesc>& paramDescs = mParamDescs[(int)type];
		if (paramDescs == nullptr)
		{
			output = TGpuDataParam<T, Core>(nullptr, nullptr);
			LOGWRN("Cannot find parameter block with the name: '" + name + "'");
			return;
		}

		auto iterFind = paramDescs->params.find(name);
		if (iterFind == paramDescs->params.end())
		{
			output = TGpuDataParam<T, Core>(nullptr, nullptr);
			LOGWRN("Cannot find parameter with the name '" + name + "'");
		}
		else
			output = TGpuDataParam<T, Core>(&iterFind->second, _getThisPtr());
	}

	template<bool Core>
	void TGpuParams<Core>::getStructParam(GpuProgramType type, const String& name, TGpuParamStruct<Core>& output) const
	{
		const SPtr<GpuParamDesc>& paramDescs = mParamDescs[(int)type];
		if (paramDescs == nullptr)
		{
			output = TGpuParamStruct<Core>(nullptr, nullptr);
			LOGWRN("Cannot find struct parameter with the name: '" + name + "'");
			return;
		}

		auto iterFind = paramDescs->params.find(name);
		if (iterFind == paramDescs->params.end() || iterFind->second.type != GPDT_STRUCT)
		{
			output = TGpuParamStruct<Core>(nullptr, nullptr);
			LOGWRN("Cannot find struct parameter with the name '" + name + "'");
		}
		else
			output = TGpuParamStruct<Core>(&iterFind->second, _getThisPtr());
	}

	template<bool Core>
	void TGpuParams<Core>::getTextureParam(GpuProgramType type, const String& name, TGpuParamTexture<Core>& output) const
	{
		const SPtr<GpuParamDesc>& paramDescs = mParamDescs[(int)type];
		if (paramDescs == nullptr)
		{
			output = TGpuParamTexture<Core>(nullptr, nullptr);
			LOGWRN("Cannot find texture parameter with the name: '" + name + "'");
			return;
		}

		auto iterFind = paramDescs->textures.find(name);
		if (iterFind == paramDescs->textures.end())
		{
			output = TGpuParamTexture<Core>(nullptr, nullptr);
			LOGWRN("Cannot find texture parameter with the name '" + name + "'");
		}
		else
			output = TGpuParamTexture<Core>(&iterFind->second, _getThisPtr());
	}

	template<bool Core>
	void TGpuParams<Core>::getLoadStoreTextureParam(GpuProgramType type, const String& name, TGpuParamLoadStoreTexture<Core>& output) const
	{
		const SPtr<GpuParamDesc>& paramDescs = mParamDescs[(int)type];
		if (paramDescs == nullptr)
		{
			output = TGpuParamLoadStoreTexture<Core>(nullptr, nullptr);
			LOGWRN("Cannot find texture parameter with the name: '" + name + "'");
			return;
		}

		auto iterFind = paramDescs->loadStoreTextures.find(name);
		if (iterFind == paramDescs->loadStoreTextures.end())
		{
			output = TGpuParamLoadStoreTexture<Core>(nullptr, nullptr);
			LOGWRN("Cannot find texture parameter with the name '" + name + "'");
		}
		else
			output = TGpuParamLoadStoreTexture<Core>(&iterFind->second, _getThisPtr());
	}

	template<bool Core>
	void TGpuParams<Core>::getBufferParam(GpuProgramType type, const String& name, TGpuParamBuffer<Core>& output) const
	{
		const SPtr<GpuParamDesc>& paramDescs = mParamDescs[(int)type];
		if (paramDescs == nullptr)
		{
			output = TGpuParamBuffer<Core>(nullptr, nullptr);
			LOGWRN("Cannot find buffer parameter with the name: '" + name + "'");
			return;
		}

		auto iterFind = paramDescs->buffers.find(name);
		if (iterFind == paramDescs->buffers.end())
		{
			output = TGpuParamBuffer<Core>(nullptr, nullptr);
			LOGWRN("Cannot find buffer parameter with the name '" + name + "'");
		}
		else
			output = TGpuParamBuffer<Core>(&iterFind->second, _getThisPtr());
	}

	template<bool Core>
	void TGpuParams<Core>::getSamplerStateParam(GpuProgramType type, const String& name, TGpuParamSampState<Core>& output) const
	{
		const SPtr<GpuParamDesc>& paramDescs = mParamDescs[(int)type];
		if (paramDescs == nullptr)
		{
			output = TGpuParamSampState<Core>(nullptr, nullptr);
			LOGWRN("Cannot find sampler state parameter with the name: '" + name + "'");
			return;
		}

		auto iterFind = paramDescs->samplers.find(name);
		if (iterFind == paramDescs->samplers.end())
		{
			output = TGpuParamSampState<Core>(nullptr, nullptr);
			LOGWRN("Cannot find sampler state parameter with the name '" + name + "'");
		}
		else
			output = TGpuParamSampState<Core>(&iterFind->second, _getThisPtr());
	}

	template<bool Core>
	typename TGpuParams<Core>::ParamsBufferType TGpuParams<Core>::getParamBlockBuffer(UINT32 set, UINT32 slot) const
	{
		UINT32 globalSlot = getGlobalSlot(ElementType::ParamBlock, set, slot);
		if (globalSlot == (UINT32)-1)
			return nullptr;

		return mParamBlockBuffers[globalSlot];
	}

	template<bool Core>
	typename TGpuParams<Core>::TextureType TGpuParams<Core>::getTexture(UINT32 set, UINT32 slot) const
	{
		UINT32 globalSlot = getGlobalSlot(ElementType::Texture, set, slot);
		if (globalSlot == (UINT32)-1)
			return TGpuParams<Core>::TextureType();

		return mTextures[globalSlot];
	}

	template<bool Core>
	typename TGpuParams<Core>::TextureType TGpuParams<Core>::getLoadStoreTexture(UINT32 set, UINT32 slot) const
	{
		UINT32 globalSlot = getGlobalSlot(ElementType::LoadStoreTexture, set, slot);
		if (globalSlot == (UINT32)-1)
			return TGpuParams<Core>::TextureType();

		return mLoadStoreTextures[globalSlot];
	}

	template<bool Core>
	typename TGpuParams<Core>::BufferType TGpuParams<Core>::getBuffer(UINT32 set, UINT32 slot) const
	{
		UINT32 globalSlot = getGlobalSlot(ElementType::Buffer, set, slot);
		if (globalSlot == (UINT32)-1)
			return nullptr;

		return mBuffers[globalSlot];
	}

	template<bool Core>
	typename TGpuParams<Core>::SamplerType TGpuParams<Core>::getSamplerState(UINT32 set, UINT32 slot) const
	{
		UINT32 globalSlot = getGlobalSlot(ElementType::SamplerState, set, slot);
		if (globalSlot == (UINT32)-1)
			return nullptr;

		return mSamplerStates[globalSlot];
	}

	template<bool Core>
	const TextureSurface& TGpuParams<Core>::getLoadStoreSurface(UINT32 set, UINT32 slot) const
	{
		static TextureSurface emptySurface;

		UINT32 globalSlot = getGlobalSlot(ElementType::LoadStoreTexture, set, slot);
		if (globalSlot == (UINT32)-1)
			return emptySurface;

		return mLoadStoreSurfaces[globalSlot];
	}


	template<bool Core>
	void TGpuParams<Core>::setTexture(UINT32 set, UINT32 slot, const TextureType& texture)
	{
		UINT32 globalSlot = getGlobalSlot(ElementType::Texture, set, slot);
		if (globalSlot == (UINT32)-1)
			return;

		mTextures[globalSlot] = texture;

		_markResourcesDirty();
		_markCoreDirty();
	}

	template<bool Core>
	void TGpuParams<Core>::setLoadStoreTexture(UINT32 set, UINT32 slot, const TextureType& texture, const TextureSurface& surface)
	{
		UINT32 globalSlot = getGlobalSlot(ElementType::LoadStoreTexture, set, slot);
		if (globalSlot == (UINT32)-1)
			return;

		mLoadStoreTextures[globalSlot] = texture;

		_markResourcesDirty();
		_markCoreDirty();
	}

	template<bool Core>
	void TGpuParams<Core>::setBuffer(UINT32 set, UINT32 slot, const BufferType& buffer)
	{
		UINT32 globalSlot = getGlobalSlot(ElementType::Buffer, set, slot);
		if (globalSlot == (UINT32)-1)
			return;

		mBuffers[globalSlot] = buffer;

		_markResourcesDirty();
		_markCoreDirty();
	}

	template<bool Core>
	void TGpuParams<Core>::setSamplerState(UINT32 set, UINT32 slot, const SamplerType& sampler)
	{
		UINT32 globalSlot = getGlobalSlot(ElementType::SamplerState, set, slot);
		if (globalSlot == (UINT32)-1)
			return;

		mSamplerStates[globalSlot] = sampler;

		_markResourcesDirty();
		_markCoreDirty();
	}

	template<bool Core>
	void TGpuParams<Core>::setLoadStoreSurface(UINT32 set, UINT32 slot, const TextureSurface& surface)
	{
		UINT32 globalSlot = getGlobalSlot(ElementType::LoadStoreTexture, set, slot);
		if (globalSlot == (UINT32)-1)
			return;

		mLoadStoreSurfaces[globalSlot] = surface;
	}

	template class TGpuParams < false > ;
	template class TGpuParams < true > ;

	template BS_CORE_EXPORT void TGpuParams<false>::getParam<float>(GpuProgramType type, const String&, TGpuDataParam<float, false>&) const;
	template BS_CORE_EXPORT void TGpuParams<false>::getParam<int>(GpuProgramType type, const String&, TGpuDataParam<int, false>&) const;
	template BS_CORE_EXPORT void TGpuParams<false>::getParam<Color>(GpuProgramType type, const String&, TGpuDataParam<Color, false>&) const;
	template BS_CORE_EXPORT void TGpuParams<false>::getParam<Vector2>(GpuProgramType type, const String&, TGpuDataParam<Vector2, false>&) const;
	template BS_CORE_EXPORT void TGpuParams<false>::getParam<Vector3>(GpuProgramType type, const String&, TGpuDataParam<Vector3, false>&) const;
	template BS_CORE_EXPORT void TGpuParams<false>::getParam<Vector4>(GpuProgramType type, const String&, TGpuDataParam<Vector4, false>&) const;
	template BS_CORE_EXPORT void TGpuParams<false>::getParam<Vector2I>(GpuProgramType type, const String&, TGpuDataParam<Vector2I, false>&) const;
	template BS_CORE_EXPORT void TGpuParams<false>::getParam<Vector3I>(GpuProgramType type, const String&, TGpuDataParam<Vector3I, false>&) const;
	template BS_CORE_EXPORT void TGpuParams<false>::getParam<Vector4I>(GpuProgramType type, const String&, TGpuDataParam<Vector4I, false>&) const;
	template BS_CORE_EXPORT void TGpuParams<false>::getParam<Matrix2>(GpuProgramType type, const String&, TGpuDataParam<Matrix2, false>&) const;
	template BS_CORE_EXPORT void TGpuParams<false>::getParam<Matrix2x3>(GpuProgramType type, const String&, TGpuDataParam<Matrix2x3, false>&) const;
	template BS_CORE_EXPORT void TGpuParams<false>::getParam<Matrix2x4>(GpuProgramType type, const String&, TGpuDataParam<Matrix2x4, false>&) const;
	template BS_CORE_EXPORT void TGpuParams<false>::getParam<Matrix3>(GpuProgramType type, const String&, TGpuDataParam<Matrix3, false>&) const;
	template BS_CORE_EXPORT void TGpuParams<false>::getParam<Matrix3x2>(GpuProgramType type, const String&, TGpuDataParam<Matrix3x2, false>&) const;
	template BS_CORE_EXPORT void TGpuParams<false>::getParam<Matrix3x4>(GpuProgramType type, const String&, TGpuDataParam<Matrix3x4, false>&) const;
	template BS_CORE_EXPORT void TGpuParams<false>::getParam<Matrix4>(GpuProgramType type, const String&, TGpuDataParam<Matrix4, false>&) const;
	template BS_CORE_EXPORT void TGpuParams<false>::getParam<Matrix4x2>(GpuProgramType type, const String&, TGpuDataParam<Matrix4x2, false>&) const;
	template BS_CORE_EXPORT void TGpuParams<false>::getParam<Matrix4x3>(GpuProgramType type, const String&, TGpuDataParam<Matrix4x3, false>&) const;

	template BS_CORE_EXPORT void TGpuParams<true>::getParam<float>(GpuProgramType type, const String&, TGpuDataParam<float, true>&) const;
	template BS_CORE_EXPORT void TGpuParams<true>::getParam<int>(GpuProgramType type, const String&, TGpuDataParam<int, true>&) const;
	template BS_CORE_EXPORT void TGpuParams<true>::getParam<Color>(GpuProgramType type, const String&, TGpuDataParam<Color, true>&) const;
	template BS_CORE_EXPORT void TGpuParams<true>::getParam<Vector2>(GpuProgramType type, const String&, TGpuDataParam<Vector2, true>&) const;
	template BS_CORE_EXPORT void TGpuParams<true>::getParam<Vector3>(GpuProgramType type, const String&, TGpuDataParam<Vector3, true>&) const;
	template BS_CORE_EXPORT void TGpuParams<true>::getParam<Vector4>(GpuProgramType type, const String&, TGpuDataParam<Vector4, true>&) const;
	template BS_CORE_EXPORT void TGpuParams<true>::getParam<Vector2I>(GpuProgramType type, const String&, TGpuDataParam<Vector2I, true>&) const;
	template BS_CORE_EXPORT void TGpuParams<true>::getParam<Vector3I>(GpuProgramType type, const String&, TGpuDataParam<Vector3I, true>&) const;
	template BS_CORE_EXPORT void TGpuParams<true>::getParam<Vector4I>(GpuProgramType type, const String&, TGpuDataParam<Vector4I, true>&) const;
	template BS_CORE_EXPORT void TGpuParams<true>::getParam<Matrix2>(GpuProgramType type, const String&, TGpuDataParam<Matrix2, true>&) const;
	template BS_CORE_EXPORT void TGpuParams<true>::getParam<Matrix2x3>(GpuProgramType type, const String&, TGpuDataParam<Matrix2x3, true>&) const;
	template BS_CORE_EXPORT void TGpuParams<true>::getParam<Matrix2x4>(GpuProgramType type, const String&, TGpuDataParam<Matrix2x4, true>&) const;
	template BS_CORE_EXPORT void TGpuParams<true>::getParam<Matrix3>(GpuProgramType type, const String&, TGpuDataParam<Matrix3, true>&) const;
	template BS_CORE_EXPORT void TGpuParams<true>::getParam<Matrix3x2>(GpuProgramType type, const String&, TGpuDataParam<Matrix3x2, true>&) const;
	template BS_CORE_EXPORT void TGpuParams<true>::getParam<Matrix3x4>(GpuProgramType type, const String&, TGpuDataParam<Matrix3x4, true>&) const;
	template BS_CORE_EXPORT void TGpuParams<true>::getParam<Matrix4>(GpuProgramType type, const String&, TGpuDataParam<Matrix4, true>&) const;
	template BS_CORE_EXPORT void TGpuParams<true>::getParam<Matrix4x2>(GpuProgramType type, const String&, TGpuDataParam<Matrix4x2, true>&) const;
	template BS_CORE_EXPORT void TGpuParams<true>::getParam<Matrix4x3>(GpuProgramType type, const String&, TGpuDataParam<Matrix4x3, true>&) const;

	GpuParamsCore::GpuParamsCore(const GPU_PARAMS_DESC& desc)
		: TGpuParams(desc)
	{

	}

	SPtr<GpuParamsCore> GpuParamsCore::_getThisPtr() const
	{
		return std::static_pointer_cast<GpuParamsCore>(getThisPtr());
	}

	void GpuParamsCore::syncToCore(const CoreSyncData& data)
	{
		UINT32 loadStoreSurfacesSize = mNumElements[(int)ElementType::LoadStoreTexture] * sizeof(TextureSurface);
		UINT32 paramBufferSize = mNumElements[(int)ElementType::ParamBlock] * sizeof(SPtr<GpuParamBlockBufferCore>);
		UINT32 textureArraySize = mNumElements[(int)ElementType::Texture] * sizeof(SPtr<TextureCore>);
		UINT32 loadStoreTextureArraySize = mNumElements[(int)ElementType::LoadStoreTexture] * sizeof(SPtr<TextureCore>);
		UINT32 bufferArraySize = mNumElements[(int)ElementType::Buffer] * sizeof(SPtr<GpuBufferCore>);
		UINT32 samplerArraySize = mNumElements[(int)ElementType::SamplerState] * sizeof(SPtr<SamplerStateCore>);

		UINT32 totalSize = loadStoreSurfacesSize + paramBufferSize + textureArraySize + loadStoreTextureArraySize
			+ bufferArraySize + samplerArraySize;

		UINT32 textureInfoOffset = 0;
		UINT32 paramBufferOffset = textureInfoOffset + loadStoreSurfacesSize;
		UINT32 textureArrayOffset = paramBufferOffset + paramBufferSize;
		UINT32 loadStoreTextureArrayOffset = textureArrayOffset + textureArraySize;
		UINT32 bufferArrayOffset = loadStoreTextureArrayOffset + loadStoreTextureArraySize;
		UINT32 samplerArrayOffset = bufferArrayOffset + bufferArraySize;

		assert(data.getBufferSize() == totalSize);

		UINT8* dataPtr = data.getBuffer();

		TextureSurface* loadStoreSurfaces = (TextureSurface*)(dataPtr + textureInfoOffset);
		SPtr<GpuParamBlockBufferCore>* paramBuffers = (SPtr<GpuParamBlockBufferCore>*)(dataPtr + paramBufferOffset);
		SPtr<TextureCore>* textures = (SPtr<TextureCore>*)(dataPtr + textureArrayOffset);
		SPtr<TextureCore>* loadStoreTextures = (SPtr<TextureCore>*)(dataPtr + loadStoreTextureArrayOffset);
		SPtr<GpuBufferCore>* buffers = (SPtr<GpuBufferCore>*)(dataPtr + bufferArrayOffset);
		SPtr<SamplerStateCore>* samplers = (SPtr<SamplerStateCore>*)(dataPtr + samplerArrayOffset);

		// Copy & destruct
		for (UINT32 i = 0; i < mNumElements[(int)ElementType::ParamBlock]; i++)
		{
			mParamBlockBuffers[i] = paramBuffers[i];
			paramBuffers[i].~SPtr<GpuParamBlockBufferCore>();
		}

		for (UINT32 i = 0; i < mNumElements[(int)ElementType::Texture]; i++)
		{
			mTextures[i] = textures[i];
			textures[i].~SPtr<TextureCore>();
		}

		for (UINT32 i = 0; i < mNumElements[(int)ElementType::LoadStoreTexture]; i++)
		{
			mLoadStoreSurfaces[i] = loadStoreSurfaces[i];
			loadStoreSurfaces[i].~TextureSurface();

			mLoadStoreTextures[i] = loadStoreTextures[i];
			loadStoreTextures[i].~SPtr<TextureCore>();
		}

		for (UINT32 i = 0; i < mNumElements[(int)ElementType::Buffer]; i++)
		{
			mBuffers[i] = buffers[i];
			buffers[i].~SPtr<GpuBufferCore>();
		}

		for (UINT32 i = 0; i < mNumElements[(int)ElementType::SamplerState]; i++)
		{
			mSamplerStates[i] = samplers[i];
			samplers[i].~SPtr<SamplerStateCore>();
		}
	}

	SPtr<GpuParamsCore> GpuParamsCore::create(const GPU_PARAMS_DESC& desc)
	{
		return HardwareBufferCoreManager::instance().createGpuParams(desc);
	}

	const GpuDataParamInfos GpuParams::PARAM_SIZES;

	GpuParams::GpuParams(const GPU_PARAMS_DESC& desc)
		: TGpuParams(desc)
	{

	}

	SPtr<GpuParams> GpuParams::_getThisPtr() const
	{
		return std::static_pointer_cast<GpuParams>(getThisPtr());
	}

	SPtr<GpuParamsCore> GpuParams::getCore() const
	{
		return std::static_pointer_cast<GpuParamsCore>(mCoreSpecific);
	}

	SPtr<CoreObjectCore> GpuParams::createCore() const
	{
		GPU_PARAMS_DESC desc;
		desc.vertexParams = mParamDescs[GPT_VERTEX_PROGRAM];
		desc.fragmentParams = mParamDescs[GPT_FRAGMENT_PROGRAM];
		desc.geometryParams = mParamDescs[GPT_GEOMETRY_PROGRAM];
		desc.hullParams = mParamDescs[GPT_HULL_PROGRAM];
		desc.domainParams = mParamDescs[GPT_DOMAIN_PROGRAM];
		desc.computeParams = mParamDescs[GPT_COMPUTE_PROGRAM];

		GpuParamsCore* obj = new (bs_alloc<GpuParamsCore>()) GpuParamsCore(desc);

		SPtr<CoreObjectCore> coreObj = bs_shared_ptr<GpuParamsCore>(obj);
		coreObj->_setThisPtr(coreObj);

		return coreObj;
	}

	void GpuParams::_markCoreDirty()
	{
		markCoreDirty();
	}

	void GpuParams::_markResourcesDirty()
	{
		markListenerResourcesDirty();
	}

	SPtr<GpuParams> GpuParams::create(const GPU_PARAMS_DESC& desc)
	{
		return HardwareBufferManager::instance().createGpuParams(desc);
	}

	CoreSyncData GpuParams::syncToCore(FrameAlloc* allocator)
	{
		UINT32 loadStoreSurfacesSize = mNumElements[(int)ElementType::LoadStoreTexture] * sizeof(TextureSurface);
		UINT32 paramBufferSize = mNumElements[(int)ElementType::ParamBlock] * sizeof(SPtr<GpuParamBlockBufferCore>);
		UINT32 textureArraySize = mNumElements[(int)ElementType::Texture] * sizeof(SPtr<TextureCore>);
		UINT32 loadStoreTextureArraySize = mNumElements[(int)ElementType::LoadStoreTexture] * sizeof(SPtr<TextureCore>);
		UINT32 bufferArraySize = mNumElements[(int)ElementType::Buffer] * sizeof(SPtr<GpuBufferCore>);
		UINT32 samplerArraySize = mNumElements[(int)ElementType::SamplerState] * sizeof(SPtr<SamplerStateCore>);

		UINT32 totalSize = loadStoreSurfacesSize + paramBufferSize + textureArraySize + loadStoreTextureArraySize 
			+ bufferArraySize + samplerArraySize;

		UINT32 textureInfoOffset = 0;
		UINT32 paramBufferOffset = textureInfoOffset + loadStoreSurfacesSize;
		UINT32 textureArrayOffset = paramBufferOffset + paramBufferSize;
		UINT32 loadStoreTextureArrayOffset = textureArrayOffset + textureArraySize;
		UINT32 bufferArrayOffset = loadStoreTextureArrayOffset + loadStoreTextureArraySize;
		UINT32 samplerArrayOffset = bufferArrayOffset + bufferArraySize;

		UINT8* data = allocator->alloc(totalSize);

		TextureSurface* loadStoreSurfaces = (TextureSurface*)(data + textureInfoOffset);
		SPtr<GpuParamBlockBufferCore>* paramBuffers = (SPtr<GpuParamBlockBufferCore>*)(data + paramBufferOffset);
		SPtr<TextureCore>* textures = (SPtr<TextureCore>*)(data + textureArrayOffset);
		SPtr<TextureCore>* loadStoreTextures = (SPtr<TextureCore>*)(data + loadStoreTextureArrayOffset);
		SPtr<GpuBufferCore>* buffers = (SPtr<GpuBufferCore>*)(data + bufferArrayOffset);
		SPtr<SamplerStateCore>* samplers = (SPtr<SamplerStateCore>*)(data + samplerArrayOffset);

		// Construct & copy
		for (UINT32 i = 0; i < mNumElements[(int)ElementType::ParamBlock]; i++)
		{
			new (&paramBuffers[i]) SPtr<GpuParamBlockBufferCore>();

			if (mParamBlockBuffers[i] != nullptr)
				paramBuffers[i] = mParamBlockBuffers[i]->getCore();
		}

		for (UINT32 i = 0; i < mNumElements[(int)ElementType::Texture]; i++)
		{
			new (&textures[i]) SPtr<TextureCore>();

			if (mTextures[i].isLoaded())
				textures[i] = mTextures[i]->getCore();
			else
				textures[i] = nullptr;
		}

		for (UINT32 i = 0; i < mNumElements[(int)ElementType::LoadStoreTexture]; i++)
		{
			new (&loadStoreSurfaces[i]) TextureSurface();
			loadStoreSurfaces[i] = mLoadStoreSurfaces[i];

			new (&loadStoreTextures[i]) SPtr<TextureCore>();

			if (mLoadStoreTextures[i].isLoaded())
				loadStoreTextures[i] = mLoadStoreTextures[i]->getCore();
			else
				loadStoreTextures[i] = nullptr;
		}

		for (UINT32 i = 0; i < mNumElements[(int)ElementType::Buffer]; i++)
		{
			new (&buffers[i]) SPtr<GpuBufferCore>();

			if (mBuffers[i] != nullptr)
				buffers[i] = mBuffers[i]->getCore();
			else
				buffers[i] = nullptr;
		}

		for (UINT32 i = 0; i < mNumElements[(int)ElementType::SamplerState]; i++)
		{
			new (&samplers[i]) SPtr<SamplerStateCore>();

			if (mSamplerStates[i] != nullptr)
				samplers[i] = mSamplerStates[i]->getCore();
			else
				samplers[i] = nullptr;
		}

		return CoreSyncData(data, totalSize);
	}

	void GpuParams::getListenerResources(Vector<HResource>& resources)
	{
		for (UINT32 i = 0; i < mNumElements[(int)ElementType::Texture]; i++)
		{
			if (mTextures[i] != nullptr)
				resources.push_back(mTextures[i]);
		}

		for (UINT32 i = 0; i < mNumElements[(int)ElementType::LoadStoreTexture]; i++)
		{
			if (mLoadStoreTextures[i] != nullptr)
				resources.push_back(mLoadStoreTextures[i]);
		}
	}
}