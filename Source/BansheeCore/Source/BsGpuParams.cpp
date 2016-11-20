//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsGpuParams.h"
#include "BsGpuParamDesc.h"
#include "BsGpuParamBlockBuffer.h"
#include "BsGpuPipelineParamInfo.h"
#include "BsGpuPipelineState.h"
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

namespace bs
{
	GpuParamsBase::GpuParamsBase(const SPtr<GpuPipelineParamInfoBase>& paramInfo)
		:mParamInfo(paramInfo)
	{ }

	GpuParamsBase::~GpuParamsBase()
	{ }

	SPtr<GpuParamDesc> GpuParamsBase::getParamDesc(GpuProgramType type) const
	{
		return mParamInfo->getParamDesc(type);
	}

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
		const SPtr<GpuParamDesc>& paramDesc = mParamInfo->getParamDesc(type);
		if (paramDesc == nullptr)
			return false;

		auto paramIter = paramDesc->textures.find(name);
		if(paramIter != paramDesc->textures.end())
			return true;

		return false;
	}

	bool GpuParamsBase::hasBuffer(GpuProgramType type, const String& name) const
	{
		const SPtr<GpuParamDesc>& paramDesc = mParamInfo->getParamDesc(type);
		if (paramDesc == nullptr)
			return false;

		auto paramIter = paramDesc->buffers.find(name);
		if (paramIter != paramDesc->buffers.end())
			return true;

		return false;
	}

	bool GpuParamsBase::hasLoadStoreTexture(GpuProgramType type, const String& name) const
	{
		const SPtr<GpuParamDesc>& paramDesc = mParamInfo->getParamDesc(type);
		if (paramDesc == nullptr)
			return false;

		auto paramIter = paramDesc->loadStoreTextures.find(name);
		if (paramIter != paramDesc->loadStoreTextures.end())
			return true;

		return false;
	}

	bool GpuParamsBase::hasSamplerState(GpuProgramType type, const String& name) const
	{
		const SPtr<GpuParamDesc>& paramDesc = mParamInfo->getParamDesc(type);
		if (paramDesc == nullptr)
			return false;

		auto paramIter = paramDesc->samplers.find(name);
		if(paramIter != paramDesc->samplers.end())
			return true;

		return false;
	}

	bool GpuParamsBase::hasParamBlock(GpuProgramType type, const String& name) const
	{
		const SPtr<GpuParamDesc>& paramDesc = mParamInfo->getParamDesc(type);
		if (paramDesc == nullptr)
			return false;

		auto paramBlockIter = paramDesc->paramBlocks.find(name);
		if(paramBlockIter != paramDesc->paramBlocks.end())
			return true;

		return false;
	}

	GpuParamDataDesc* GpuParamsBase::getParamDesc(GpuProgramType type, const String& name) const
	{
		const SPtr<GpuParamDesc>& paramDesc = mParamInfo->getParamDesc(type);
		if (paramDesc == nullptr)
			return nullptr;

		auto paramIter = paramDesc->params.find(name);
		if (paramIter != paramDesc->params.end())
			return &paramIter->second;

		return nullptr;
	}

	GpuParamBlockDesc* GpuParamsBase::getParamBlockDesc(GpuProgramType type, const String& name) const
	{
		const SPtr<GpuParamDesc>& paramDesc = mParamInfo->getParamDesc(type);
		if (paramDesc == nullptr)
			return nullptr;

		auto paramBlockIter = paramDesc->paramBlocks.find(name);
		if (paramBlockIter != paramDesc->paramBlocks.end())
			return &paramBlockIter->second;

		return nullptr;
	}

	template<bool Core>
	TGpuParams<Core>::TGpuParams(const SPtr<GpuPipelineParamInfoBase>& paramInfo)
		: GpuParamsBase(paramInfo)
	{
		UINT32 numParamBlocks = mParamInfo->getNumElements(GpuPipelineParamInfo::ParamType::ParamBlock);
		UINT32 numTextures = mParamInfo->getNumElements(GpuPipelineParamInfo::ParamType::Texture);
		UINT32 numStorageTextures = mParamInfo->getNumElements(GpuPipelineParamInfo::ParamType::LoadStoreTexture);
		UINT32 numBuffers = mParamInfo->getNumElements(GpuPipelineParamInfo::ParamType::Buffer);
		UINT32 numSamplers = mParamInfo->getNumElements(GpuPipelineParamInfo::ParamType::SamplerState);
		
		UINT32 paramBlocksSize = sizeof(ParamsBufferType) * numParamBlocks;
		UINT32 texturesSize = sizeof(TextureType) * numTextures;
		UINT32 loadStoreTexturesSize = sizeof(TextureType) * numStorageTextures;
		UINT32 loadStoreSurfacesSize = sizeof(TextureSurface) * numStorageTextures;
		UINT32 buffersSize = sizeof(BufferType) * numBuffers;
		UINT32 samplerStatesSize = sizeof(SamplerType) * numSamplers;

		UINT32 totalSize = paramBlocksSize + texturesSize + loadStoreTexturesSize + loadStoreSurfacesSize +
			buffersSize + samplerStatesSize;

		UINT8* data = (UINT8*)bs_alloc(totalSize);
		mParamBlockBuffers = (ParamsBufferType*)data;
		for (UINT32 i = 0; i < numParamBlocks; i++)
			new (&mParamBlockBuffers[i]) ParamsBufferType();

		data += sizeof(ParamsBufferType) * numParamBlocks;
		mTextures = (TextureType*)data;
		for (UINT32 i = 0; i < numTextures; i++)
			new (&mTextures[i]) TextureType();

		data += sizeof(TextureType) * numTextures;
		mLoadStoreTextures = (TextureType*)data;
		for (UINT32 i = 0; i < numStorageTextures; i++)
			new (&mLoadStoreTextures[i]) TextureType();

		data += sizeof(TextureType) * numStorageTextures;
		mLoadStoreSurfaces = (TextureSurface*)data;
		for (UINT32 i = 0; i < numStorageTextures; i++)
			new (&mLoadStoreSurfaces[i]) TextureSurface();

		data += sizeof(TextureSurface) * numStorageTextures;
		mBuffers = (BufferType*)data;
		for (UINT32 i = 0; i < numBuffers; i++)
			new (&mBuffers[i]) BufferType();

		data += sizeof(BufferType) * numBuffers;
		mSamplerStates = (SamplerType*)data;
		for (UINT32 i = 0; i < numSamplers; i++)
			new (&mSamplerStates[i]) SamplerType();

		data += sizeof(SamplerType) * numSamplers;
	}

	template<bool Core>
	TGpuParams<Core>::~TGpuParams()
	{
		UINT32 numParamBlocks = mParamInfo->getNumElements(GpuPipelineParamInfo::ParamType::ParamBlock);
		UINT32 numTextures = mParamInfo->getNumElements(GpuPipelineParamInfo::ParamType::Texture);
		UINT32 numStorageTextures = mParamInfo->getNumElements(GpuPipelineParamInfo::ParamType::LoadStoreTexture);
		UINT32 numBuffers = mParamInfo->getNumElements(GpuPipelineParamInfo::ParamType::Buffer);
		UINT32 numSamplers = mParamInfo->getNumElements(GpuPipelineParamInfo::ParamType::SamplerState);

		for (UINT32 i = 0; i < numParamBlocks; i++)
			mParamBlockBuffers[i].~ParamsBufferType();

		for (UINT32 i = 0; i <  numTextures; i++)
			mTextures[i].~TextureType();

		for (UINT32 i = 0; i <  numStorageTextures; i++)
		{
			mLoadStoreTextures[i].~TextureType();
			mLoadStoreSurfaces[i].~TextureSurface();
		}

		for (UINT32 i = 0; i < numBuffers; i++)
			mBuffers[i].~BufferType();

		for (UINT32 i = 0; i < numSamplers; i++)
			mSamplerStates[i].~SamplerType();

		// Everything is allocated in a single block, so it's enough to free the first element
		bs_free(mParamBlockBuffers);
	}

	template<bool Core>
	void TGpuParams<Core>::setParamBlockBuffer(UINT32 set, UINT32 slot, const ParamsBufferType& paramBlockBuffer)
	{
		UINT32 globalSlot = mParamInfo->getSequentialSlot(GpuPipelineParamInfo::ParamType::ParamBlock, set, slot);
		if (globalSlot == (UINT32)-1)
			return;

		mParamBlockBuffers[globalSlot] = paramBlockBuffer;

		_markCoreDirty();
	}

	template<bool Core>
	void TGpuParams<Core>::setParamBlockBuffer(GpuProgramType type, const String& name, const ParamsBufferType& paramBlockBuffer)
	{
		const SPtr<GpuParamDesc>& paramDescs = mParamInfo->getParamDesc(type);
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
		const SPtr<GpuParamDesc>& paramDescs = mParamInfo->getParamDesc(type);
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
		const SPtr<GpuParamDesc>& paramDescs = mParamInfo->getParamDesc(type);
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
		const SPtr<GpuParamDesc>& paramDescs = mParamInfo->getParamDesc(type);
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
		const SPtr<GpuParamDesc>& paramDescs = mParamInfo->getParamDesc(type);
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
		const SPtr<GpuParamDesc>& paramDescs = mParamInfo->getParamDesc(type);
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
		const SPtr<GpuParamDesc>& paramDescs = mParamInfo->getParamDesc(type);
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
		UINT32 globalSlot = mParamInfo->getSequentialSlot(GpuPipelineParamInfo::ParamType::ParamBlock, set, slot);
		if (globalSlot == (UINT32)-1)
			return nullptr;

		return mParamBlockBuffers[globalSlot];
	}

	template<bool Core>
	typename TGpuParams<Core>::TextureType TGpuParams<Core>::getTexture(UINT32 set, UINT32 slot) const
	{
		UINT32 globalSlot = mParamInfo->getSequentialSlot(GpuPipelineParamInfo::ParamType::Texture, set, slot);
		if (globalSlot == (UINT32)-1)
			return TGpuParams<Core>::TextureType();

		return mTextures[globalSlot];
	}

	template<bool Core>
	typename TGpuParams<Core>::TextureType TGpuParams<Core>::getLoadStoreTexture(UINT32 set, UINT32 slot) const
	{
		UINT32 globalSlot = mParamInfo->getSequentialSlot(GpuPipelineParamInfo::ParamType::LoadStoreTexture, set, slot);
		if (globalSlot == (UINT32)-1)
			return TGpuParams<Core>::TextureType();

		return mLoadStoreTextures[globalSlot];
	}

	template<bool Core>
	typename TGpuParams<Core>::BufferType TGpuParams<Core>::getBuffer(UINT32 set, UINT32 slot) const
	{
		UINT32 globalSlot = mParamInfo->getSequentialSlot(GpuPipelineParamInfo::ParamType::Buffer, set, slot);
		if (globalSlot == (UINT32)-1)
			return nullptr;

		return mBuffers[globalSlot];
	}

	template<bool Core>
	typename TGpuParams<Core>::SamplerType TGpuParams<Core>::getSamplerState(UINT32 set, UINT32 slot) const
	{
		UINT32 globalSlot = mParamInfo->getSequentialSlot(GpuPipelineParamInfo::ParamType::SamplerState, set, slot);
		if (globalSlot == (UINT32)-1)
			return nullptr;

		return mSamplerStates[globalSlot];
	}

	template<bool Core>
	const TextureSurface& TGpuParams<Core>::getLoadStoreSurface(UINT32 set, UINT32 slot) const
	{
		static TextureSurface emptySurface;

		UINT32 globalSlot = mParamInfo->getSequentialSlot(GpuPipelineParamInfo::ParamType::LoadStoreTexture, set, slot);
		if (globalSlot == (UINT32)-1)
			return emptySurface;

		return mLoadStoreSurfaces[globalSlot];
	}


	template<bool Core>
	void TGpuParams<Core>::setTexture(UINT32 set, UINT32 slot, const TextureType& texture)
	{
		UINT32 globalSlot = mParamInfo->getSequentialSlot(GpuPipelineParamInfo::ParamType::Texture, set, slot);
		if (globalSlot == (UINT32)-1)
			return;

		mTextures[globalSlot] = texture;

		_markResourcesDirty();
		_markCoreDirty();
	}

	template<bool Core>
	void TGpuParams<Core>::setLoadStoreTexture(UINT32 set, UINT32 slot, const TextureType& texture, const TextureSurface& surface)
	{
		UINT32 globalSlot = mParamInfo->getSequentialSlot(GpuPipelineParamInfo::ParamType::LoadStoreTexture, set, slot);
		if (globalSlot == (UINT32)-1)
			return;

		mLoadStoreTextures[globalSlot] = texture;

		_markResourcesDirty();
		_markCoreDirty();
	}

	template<bool Core>
	void TGpuParams<Core>::setBuffer(UINT32 set, UINT32 slot, const BufferType& buffer)
	{
		UINT32 globalSlot = mParamInfo->getSequentialSlot(GpuPipelineParamInfo::ParamType::Buffer, set, slot);
		if (globalSlot == (UINT32)-1)
			return;

		mBuffers[globalSlot] = buffer;

		_markResourcesDirty();
		_markCoreDirty();
	}

	template<bool Core>
	void TGpuParams<Core>::setSamplerState(UINT32 set, UINT32 slot, const SamplerType& sampler)
	{
		UINT32 globalSlot = mParamInfo->getSequentialSlot(GpuPipelineParamInfo::ParamType::SamplerState, set, slot);
		if (globalSlot == (UINT32)-1)
			return;

		mSamplerStates[globalSlot] = sampler;

		_markResourcesDirty();
		_markCoreDirty();
	}

	template<bool Core>
	void TGpuParams<Core>::setLoadStoreSurface(UINT32 set, UINT32 slot, const TextureSurface& surface)
	{
		UINT32 globalSlot = mParamInfo->getSequentialSlot(GpuPipelineParamInfo::ParamType::LoadStoreTexture, set, slot);
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

	GpuParamsCore::GpuParamsCore(const SPtr<GpuPipelineParamInfoCore>& paramInfo, GpuDeviceFlags deviceMask)
		: TGpuParams(paramInfo)
	{

	}

	SPtr<GpuParamsCore> GpuParamsCore::_getThisPtr() const
	{
		return std::static_pointer_cast<GpuParamsCore>(getThisPtr());
	}

	void GpuParamsCore::syncToCore(const CoreSyncData& data)
	{
		UINT32 numParamBlocks = mParamInfo->getNumElements(GpuPipelineParamInfo::ParamType::ParamBlock);
		UINT32 numTextures = mParamInfo->getNumElements(GpuPipelineParamInfo::ParamType::Texture);
		UINT32 numStorageTextures = mParamInfo->getNumElements(GpuPipelineParamInfo::ParamType::LoadStoreTexture);
		UINT32 numBuffers = mParamInfo->getNumElements(GpuPipelineParamInfo::ParamType::Buffer);
		UINT32 numSamplers = mParamInfo->getNumElements(GpuPipelineParamInfo::ParamType::SamplerState);

		UINT32 loadStoreSurfacesSize = numStorageTextures * sizeof(TextureSurface);
		UINT32 paramBufferSize = numParamBlocks * sizeof(SPtr<GpuParamBlockBufferCore>);
		UINT32 textureArraySize = numTextures * sizeof(SPtr<TextureCore>);
		UINT32 loadStoreTextureArraySize = numStorageTextures * sizeof(SPtr<TextureCore>);
		UINT32 bufferArraySize = numBuffers * sizeof(SPtr<GpuBufferCore>);
		UINT32 samplerArraySize = numSamplers * sizeof(SPtr<SamplerStateCore>);

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
		for (UINT32 i = 0; i < numParamBlocks; i++)
		{
			mParamBlockBuffers[i] = paramBuffers[i];
			paramBuffers[i].~SPtr<GpuParamBlockBufferCore>();
		}

		for (UINT32 i = 0; i < numTextures; i++)
		{
			mTextures[i] = textures[i];
			textures[i].~SPtr<TextureCore>();
		}

		for (UINT32 i = 0; i < numStorageTextures; i++)
		{
			mLoadStoreSurfaces[i] = loadStoreSurfaces[i];
			loadStoreSurfaces[i].~TextureSurface();

			mLoadStoreTextures[i] = loadStoreTextures[i];
			loadStoreTextures[i].~SPtr<TextureCore>();
		}

		for (UINT32 i = 0; i < numBuffers; i++)
		{
			mBuffers[i] = buffers[i];
			buffers[i].~SPtr<GpuBufferCore>();
		}

		for (UINT32 i = 0; i < numSamplers; i++)
		{
			mSamplerStates[i] = samplers[i];
			samplers[i].~SPtr<SamplerStateCore>();
		}
	}

	SPtr<GpuParamsCore> GpuParamsCore::create(const SPtr<GraphicsPipelineStateCore>& pipelineState, GpuDeviceFlags deviceMask)
	{
		return HardwareBufferCoreManager::instance().createGpuParams(pipelineState->getParamInfo(), deviceMask);
	}

	SPtr<GpuParamsCore> GpuParamsCore::create(const SPtr<ComputePipelineStateCore>& pipelineState, GpuDeviceFlags deviceMask)
	{
		return HardwareBufferCoreManager::instance().createGpuParams(pipelineState->getParamInfo(), deviceMask);
	}

	SPtr<GpuParamsCore> GpuParamsCore::create(const SPtr<GpuPipelineParamInfoCore>& paramInfo, GpuDeviceFlags deviceMask)
	{
		return HardwareBufferCoreManager::instance().createGpuParams(paramInfo, deviceMask);
	}

	const GpuDataParamInfos GpuParams::PARAM_SIZES;

	GpuParams::GpuParams(const SPtr<GpuPipelineParamInfo>& paramInfo)
		: TGpuParams(paramInfo)
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
		SPtr<GpuPipelineParamInfo> paramInfo = std::static_pointer_cast<GpuPipelineParamInfo>(mParamInfo);

		return HardwareBufferCoreManager::instance().createGpuParams(paramInfo->getCore());
	}

	void GpuParams::_markCoreDirty()
	{
		markCoreDirty();
	}

	void GpuParams::_markResourcesDirty()
	{
		markListenerResourcesDirty();
	}

	SPtr<GpuParams> GpuParams::create(const SPtr<GraphicsPipelineState>& pipelineState)
	{
		return HardwareBufferManager::instance().createGpuParams(pipelineState->getParamInfo());
	}

	SPtr<GpuParams> GpuParams::create(const SPtr<ComputePipelineState>& pipelineState)
	{
		return HardwareBufferManager::instance().createGpuParams(pipelineState->getParamInfo());
	}

	SPtr<GpuParams> GpuParams::create(const SPtr<GpuPipelineParamInfo>& paramInfo)
	{
		return HardwareBufferManager::instance().createGpuParams(paramInfo);
	}

	CoreSyncData GpuParams::syncToCore(FrameAlloc* allocator)
	{
		UINT32 numParamBlocks = mParamInfo->getNumElements(GpuPipelineParamInfo::ParamType::ParamBlock);
		UINT32 numTextures = mParamInfo->getNumElements(GpuPipelineParamInfo::ParamType::Texture);
		UINT32 numStorageTextures = mParamInfo->getNumElements(GpuPipelineParamInfo::ParamType::LoadStoreTexture);
		UINT32 numBuffers = mParamInfo->getNumElements(GpuPipelineParamInfo::ParamType::Buffer);
		UINT32 numSamplers = mParamInfo->getNumElements(GpuPipelineParamInfo::ParamType::SamplerState);

		UINT32 loadStoreSurfacesSize = numStorageTextures * sizeof(TextureSurface);
		UINT32 paramBufferSize = numParamBlocks * sizeof(SPtr<GpuParamBlockBufferCore>);
		UINT32 textureArraySize = numTextures * sizeof(SPtr<TextureCore>);
		UINT32 loadStoreTextureArraySize = numStorageTextures * sizeof(SPtr<TextureCore>);
		UINT32 bufferArraySize = numBuffers * sizeof(SPtr<GpuBufferCore>);
		UINT32 samplerArraySize = numSamplers * sizeof(SPtr<SamplerStateCore>);

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
		for (UINT32 i = 0; i < numParamBlocks; i++)
		{
			new (&paramBuffers[i]) SPtr<GpuParamBlockBufferCore>();

			if (mParamBlockBuffers[i] != nullptr)
				paramBuffers[i] = mParamBlockBuffers[i]->getCore();
		}

		for (UINT32 i = 0; i < numTextures; i++)
		{
			new (&textures[i]) SPtr<TextureCore>();

			if (mTextures[i].isLoaded())
				textures[i] = mTextures[i]->getCore();
			else
				textures[i] = nullptr;
		}

		for (UINT32 i = 0; i < numStorageTextures; i++)
		{
			new (&loadStoreSurfaces[i]) TextureSurface();
			loadStoreSurfaces[i] = mLoadStoreSurfaces[i];

			new (&loadStoreTextures[i]) SPtr<TextureCore>();

			if (mLoadStoreTextures[i].isLoaded())
				loadStoreTextures[i] = mLoadStoreTextures[i]->getCore();
			else
				loadStoreTextures[i] = nullptr;
		}

		for (UINT32 i = 0; i < numBuffers; i++)
		{
			new (&buffers[i]) SPtr<GpuBufferCore>();

			if (mBuffers[i] != nullptr)
				buffers[i] = mBuffers[i]->getCore();
			else
				buffers[i] = nullptr;
		}

		for (UINT32 i = 0; i < numSamplers; i++)
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
		UINT32 numTextures = mParamInfo->getNumElements(GpuPipelineParamInfo::ParamType::Texture);
		UINT32 numStorageTextures = mParamInfo->getNumElements(GpuPipelineParamInfo::ParamType::LoadStoreTexture);

		for (UINT32 i = 0; i < numTextures; i++)
		{
			if (mTextures[i] != nullptr)
				resources.push_back(mTextures[i]);
		}

		for (UINT32 i = 0; i < numStorageTextures; i++)
		{
			if (mLoadStoreTextures[i] != nullptr)
				resources.push_back(mLoadStoreTextures[i]);
		}
	}
}