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

namespace bs
{
	const TextureSurface TextureSurface::COMPLETE = TextureSurface(0, 0, 0, 0);

	GpuParamsBase::GpuParamsBase(const SPtr<GpuPipelineParamInfoBase>& paramInfo)
		:mParamInfo(paramInfo)
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
		UINT32 texturesSize = (sizeof(TextureType) + sizeof(TextureSurface)) * numTextures;
		UINT32 loadStoreTexturesSize = (sizeof(TextureType) + sizeof(TextureSurface)) * numStorageTextures;
		UINT32 buffersSize = sizeof(BufferType) * numBuffers;
		UINT32 samplerStatesSize = sizeof(SamplerType) * numSamplers;

		UINT32 totalSize = paramBlocksSize + texturesSize + loadStoreTexturesSize + buffersSize + samplerStatesSize;

		UINT8* data = (UINT8*)bs_alloc(totalSize);
		mParamBlockBuffers = (ParamsBufferType*)data;
		for (UINT32 i = 0; i < numParamBlocks; i++)
			new (&mParamBlockBuffers[i]) ParamsBufferType();

		data += paramBlocksSize;
		mSampledTextureData = (TextureData*)data;
		for (UINT32 i = 0; i < numTextures; i++)
		{
			new (&mSampledTextureData[i].texture) TextureType();
			new (&mSampledTextureData[i].surface) TextureSurface(0, 0, 0, 0);
		}

		data += texturesSize;
		mLoadStoreTextureData = (TextureData*)data;
		for (UINT32 i = 0; i < numStorageTextures; i++)
		{
			new (&mLoadStoreTextureData[i].texture) TextureType();
			new (&mLoadStoreTextureData[i].surface) TextureSurface(0, 0, 0, 0);
		}

		data += loadStoreTexturesSize;
		mBuffers = (BufferType*)data;
		for (UINT32 i = 0; i < numBuffers; i++)
			new (&mBuffers[i]) BufferType();

		data += buffersSize;
		mSamplerStates = (SamplerType*)data;
		for (UINT32 i = 0; i < numSamplers; i++)
			new (&mSamplerStates[i]) SamplerType();

		data += samplerStatesSize;
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

		for (UINT32 i = 0; i < numTextures; i++)
		{
			mSampledTextureData[i].texture.~TextureType();
			mSampledTextureData[i].surface.~TextureSurface();
		}

		for (UINT32 i = 0; i <  numStorageTextures; i++)
		{
			mLoadStoreTextureData[i].texture.~TextureType();
			mLoadStoreTextureData[i].surface.~TextureSurface();
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
	void TGpuParams<Core>::setParamBlockBuffer(const String& name, const ParamsBufferType& paramBlockBuffer)
	{
		for (UINT32 i = 0; i < 6; i++)
		{
			const SPtr<GpuParamDesc>& paramDescs = mParamInfo->getParamDesc((GpuProgramType)i);
			if (paramDescs == nullptr)
				continue;

			auto iterFind = paramDescs->paramBlocks.find(name);
			if (iterFind == paramDescs->paramBlocks.end())
				continue;

			setParamBlockBuffer(iterFind->second.set, iterFind->second.slot, paramBlockBuffer);
		}
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

		return mSampledTextureData[globalSlot].texture;
	}

	template<bool Core>
	typename TGpuParams<Core>::TextureType TGpuParams<Core>::getLoadStoreTexture(UINT32 set, UINT32 slot) const
	{
		UINT32 globalSlot = mParamInfo->getSequentialSlot(GpuPipelineParamInfo::ParamType::LoadStoreTexture, set, slot);
		if (globalSlot == (UINT32)-1)
			return TGpuParams<Core>::TextureType();

		return mLoadStoreTextureData[globalSlot].texture;
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
	const TextureSurface& TGpuParams<Core>::getTextureSurface(UINT32 set, UINT32 slot) const
	{
		static TextureSurface emptySurface;

		UINT32 globalSlot = mParamInfo->getSequentialSlot(GpuPipelineParamInfo::ParamType::Texture, set, slot);
		if (globalSlot == (UINT32)-1)
			return emptySurface;

		return mSampledTextureData[globalSlot].surface;
	}

	template<bool Core>
	const TextureSurface& TGpuParams<Core>::getLoadStoreSurface(UINT32 set, UINT32 slot) const
	{
		static TextureSurface emptySurface;

		UINT32 globalSlot = mParamInfo->getSequentialSlot(GpuPipelineParamInfo::ParamType::LoadStoreTexture, set, slot);
		if (globalSlot == (UINT32)-1)
			return emptySurface;

		return mLoadStoreTextureData[globalSlot].surface;
	}


	template<bool Core>
	void TGpuParams<Core>::setTexture(UINT32 set, UINT32 slot, const TextureType& texture, const TextureSurface& surface)
	{
		UINT32 globalSlot = mParamInfo->getSequentialSlot(GpuPipelineParamInfo::ParamType::Texture, set, slot);
		if (globalSlot == (UINT32)-1)
			return;

		mSampledTextureData[globalSlot].texture = texture;
		mSampledTextureData[globalSlot].surface = surface;

		_markResourcesDirty();
		_markCoreDirty();
	}

	template<bool Core>
	void TGpuParams<Core>::setLoadStoreTexture(UINT32 set, UINT32 slot, const TextureType& texture, const TextureSurface& surface)
	{
		UINT32 globalSlot = mParamInfo->getSequentialSlot(GpuPipelineParamInfo::ParamType::LoadStoreTexture, set, slot);
		if (globalSlot == (UINT32)-1)
			return;

		mLoadStoreTextureData[globalSlot].texture = texture;
		mLoadStoreTextureData[globalSlot].surface = surface;

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

	const GpuDataParamInfos GpuParams::PARAM_SIZES;

	GpuParams::GpuParams(const SPtr<GpuPipelineParamInfo>& paramInfo)
		: TGpuParams(paramInfo)
	{

	}

	SPtr<GpuParams> GpuParams::_getThisPtr() const
	{
		return std::static_pointer_cast<GpuParams>(getThisPtr());
	}

	SPtr<ct::GpuParams> GpuParams::getCore() const
	{
		return std::static_pointer_cast<ct::GpuParams>(mCoreSpecific);
	}

	SPtr<ct::CoreObject> GpuParams::createCore() const
	{
		SPtr<GpuPipelineParamInfo> paramInfo = std::static_pointer_cast<GpuPipelineParamInfo>(mParamInfo);

		return ct::HardwareBufferManager::instance().createGpuParams(paramInfo->getCore());
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

		UINT32 sampledSurfacesSize = numTextures * sizeof(TextureSurface);
		UINT32 loadStoreSurfacesSize = numStorageTextures * sizeof(TextureSurface);
		UINT32 paramBufferSize = numParamBlocks * sizeof(SPtr<ct::GpuParamBlockBuffer>);
		UINT32 textureArraySize = numTextures * sizeof(SPtr<ct::Texture>);
		UINT32 loadStoreTextureArraySize = numStorageTextures * sizeof(SPtr<ct::Texture>);
		UINT32 bufferArraySize = numBuffers * sizeof(SPtr<ct::GpuBuffer>);
		UINT32 samplerArraySize = numSamplers * sizeof(SPtr<ct::SamplerState>);

		UINT32 totalSize = sampledSurfacesSize + loadStoreSurfacesSize + paramBufferSize + textureArraySize 
			+ loadStoreTextureArraySize + bufferArraySize + samplerArraySize;

		UINT32 sampledSurfaceOffset = 0;
		UINT32 loadStoreSurfaceOffset = sampledSurfaceOffset + sampledSurfacesSize;
		UINT32 paramBufferOffset = loadStoreSurfaceOffset + loadStoreSurfacesSize;
		UINT32 textureArrayOffset = paramBufferOffset + paramBufferSize;
		UINT32 loadStoreTextureArrayOffset = textureArrayOffset + textureArraySize;
		UINT32 bufferArrayOffset = loadStoreTextureArrayOffset + loadStoreTextureArraySize;
		UINT32 samplerArrayOffset = bufferArrayOffset + bufferArraySize;

		UINT8* data = allocator->alloc(totalSize);

		TextureSurface* sampledSurfaces = (TextureSurface*)(data + sampledSurfaceOffset);
		TextureSurface* loadStoreSurfaces = (TextureSurface*)(data + loadStoreSurfaceOffset);
		SPtr<ct::GpuParamBlockBuffer>* paramBuffers = (SPtr<ct::GpuParamBlockBuffer>*)(data + paramBufferOffset);
		SPtr<ct::Texture>* textures = (SPtr<ct::Texture>*)(data + textureArrayOffset);
		SPtr<ct::Texture>* loadStoreTextures = (SPtr<ct::Texture>*)(data + loadStoreTextureArrayOffset);
		SPtr<ct::GpuBuffer>* buffers = (SPtr<ct::GpuBuffer>*)(data + bufferArrayOffset);
		SPtr<ct::SamplerState>* samplers = (SPtr<ct::SamplerState>*)(data + samplerArrayOffset);

		// Construct & copy
		for (UINT32 i = 0; i < numParamBlocks; i++)
		{
			new (&paramBuffers[i]) SPtr<ct::GpuParamBlockBuffer>();

			if (mParamBlockBuffers[i] != nullptr)
				paramBuffers[i] = mParamBlockBuffers[i]->getCore();
		}

		for (UINT32 i = 0; i < numTextures; i++)
		{
			new (&sampledSurfaces[i]) TextureSurface();
			sampledSurfaces[i] = mSampledTextureData[i].surface;

			new (&textures[i]) SPtr<ct::Texture>();

			if (mSampledTextureData[i].texture.isLoaded())
				textures[i] = mSampledTextureData[i].texture->getCore();
			else
				textures[i] = nullptr;
		}

		for (UINT32 i = 0; i < numStorageTextures; i++)
		{
			new (&loadStoreSurfaces[i]) TextureSurface();
			loadStoreSurfaces[i] = mLoadStoreTextureData[i].surface;

			new (&loadStoreTextures[i]) SPtr<ct::Texture>();

			if (mLoadStoreTextureData[i].texture.isLoaded())
				loadStoreTextures[i] = mLoadStoreTextureData[i].texture->getCore();
			else
				loadStoreTextures[i] = nullptr;
		}

		for (UINT32 i = 0; i < numBuffers; i++)
		{
			new (&buffers[i]) SPtr<ct::GpuBuffer>();

			if (mBuffers[i] != nullptr)
				buffers[i] = mBuffers[i]->getCore();
			else
				buffers[i] = nullptr;
		}

		for (UINT32 i = 0; i < numSamplers; i++)
		{
			new (&samplers[i]) SPtr<ct::SamplerState>();

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
			if (mSampledTextureData[i].texture != nullptr)
				resources.push_back(mSampledTextureData[i].texture);
		}

		for (UINT32 i = 0; i < numStorageTextures; i++)
		{
			if (mLoadStoreTextureData[i].texture != nullptr)
				resources.push_back(mLoadStoreTextureData[i].texture);
		}
	}

	namespace ct
	{
	GpuParams::GpuParams(const SPtr<GpuPipelineParamInfo>& paramInfo, GpuDeviceFlags deviceMask)
		: TGpuParams(paramInfo)
	{

	}

	SPtr<GpuParams> GpuParams::_getThisPtr() const
	{
		return std::static_pointer_cast<GpuParams>(getThisPtr());
	}

	void GpuParams::syncToCore(const CoreSyncData& data)
	{
		UINT32 numParamBlocks = mParamInfo->getNumElements(GpuPipelineParamInfo::ParamType::ParamBlock);
		UINT32 numTextures = mParamInfo->getNumElements(GpuPipelineParamInfo::ParamType::Texture);
		UINT32 numStorageTextures = mParamInfo->getNumElements(GpuPipelineParamInfo::ParamType::LoadStoreTexture);
		UINT32 numBuffers = mParamInfo->getNumElements(GpuPipelineParamInfo::ParamType::Buffer);
		UINT32 numSamplers = mParamInfo->getNumElements(GpuPipelineParamInfo::ParamType::SamplerState);

		UINT32 sampledSurfacesSize = numTextures * sizeof(TextureSurface);
		UINT32 loadStoreSurfacesSize = numStorageTextures * sizeof(TextureSurface);
		UINT32 paramBufferSize = numParamBlocks * sizeof(SPtr<GpuParamBlockBuffer>);
		UINT32 textureArraySize = numTextures * sizeof(SPtr<Texture>);
		UINT32 loadStoreTextureArraySize = numStorageTextures * sizeof(SPtr<Texture>);
		UINT32 bufferArraySize = numBuffers * sizeof(SPtr<GpuBuffer>);
		UINT32 samplerArraySize = numSamplers * sizeof(SPtr<SamplerState>);

		UINT32 totalSize = sampledSurfacesSize + loadStoreSurfacesSize + paramBufferSize + textureArraySize 
			+ loadStoreTextureArraySize + bufferArraySize + samplerArraySize;

		UINT32 sampledSurfacesOffset = 0;
		UINT32 loadStoreSurfaceOffset = sampledSurfacesOffset + sampledSurfacesSize;
		UINT32 paramBufferOffset = loadStoreSurfaceOffset + loadStoreSurfacesSize;
		UINT32 textureArrayOffset = paramBufferOffset + paramBufferSize;
		UINT32 loadStoreTextureArrayOffset = textureArrayOffset + textureArraySize;
		UINT32 bufferArrayOffset = loadStoreTextureArrayOffset + loadStoreTextureArraySize;
		UINT32 samplerArrayOffset = bufferArrayOffset + bufferArraySize;

		assert(data.getBufferSize() == totalSize);

		UINT8* dataPtr = data.getBuffer();

		TextureSurface* sampledSurfaces = (TextureSurface*)(dataPtr + sampledSurfacesOffset);
		TextureSurface* loadStoreSurfaces = (TextureSurface*)(dataPtr + loadStoreSurfaceOffset);
		SPtr<GpuParamBlockBuffer>* paramBuffers = (SPtr<GpuParamBlockBuffer>*)(dataPtr + paramBufferOffset);
		SPtr<Texture>* textures = (SPtr<Texture>*)(dataPtr + textureArrayOffset);
		SPtr<Texture>* loadStoreTextures = (SPtr<Texture>*)(dataPtr + loadStoreTextureArrayOffset);
		SPtr<GpuBuffer>* buffers = (SPtr<GpuBuffer>*)(dataPtr + bufferArrayOffset);
		SPtr<SamplerState>* samplers = (SPtr<SamplerState>*)(dataPtr + samplerArrayOffset);

		// Copy & destruct
		for (UINT32 i = 0; i < numParamBlocks; i++)
		{
			mParamBlockBuffers[i] = paramBuffers[i];
			paramBuffers[i].~SPtr<GpuParamBlockBuffer>();
		}

		for (UINT32 i = 0; i < numTextures; i++)
		{
			mSampledTextureData[i].surface = sampledSurfaces[i];
			loadStoreSurfaces[i].~TextureSurface();

			mSampledTextureData[i].texture = textures[i];
			textures[i].~SPtr<Texture>();
		}

		for (UINT32 i = 0; i < numStorageTextures; i++)
		{
			mLoadStoreTextureData[i].surface = loadStoreSurfaces[i];
			loadStoreSurfaces[i].~TextureSurface();

			mLoadStoreTextureData[i].texture = loadStoreTextures[i];
			loadStoreTextures[i].~SPtr<Texture>();
		}

		for (UINT32 i = 0; i < numBuffers; i++)
		{
			mBuffers[i] = buffers[i];
			buffers[i].~SPtr<GpuBuffer>();
		}

		for (UINT32 i = 0; i < numSamplers; i++)
		{
			mSamplerStates[i] = samplers[i];
			samplers[i].~SPtr<SamplerState>();
		}
	}

	SPtr<GpuParams> GpuParams::create(const SPtr<GraphicsPipelineState>& pipelineState, GpuDeviceFlags deviceMask)
	{
		return HardwareBufferManager::instance().createGpuParams(pipelineState->getParamInfo(), deviceMask);
	}

	SPtr<GpuParams> GpuParams::create(const SPtr<ComputePipelineState>& pipelineState, GpuDeviceFlags deviceMask)
	{
		return HardwareBufferManager::instance().createGpuParams(pipelineState->getParamInfo(), deviceMask);
	}

	SPtr<GpuParams> GpuParams::create(const SPtr<GpuPipelineParamInfo>& paramInfo, GpuDeviceFlags deviceMask)
	{
		return HardwareBufferManager::instance().createGpuParams(paramInfo, deviceMask);
	}
	}
}