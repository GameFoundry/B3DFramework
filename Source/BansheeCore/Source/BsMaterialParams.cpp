//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsMaterialParams.h"
#include "BsMaterialParamsRTTI.h"
#include "BsShader.h"

namespace BansheeEngine
{
	MaterialParams::MaterialParams(const HShader& shader)
	{
		mDataSize = 0;

		auto& dataParams = shader->getDataParams();
		for (auto& param : dataParams)
		{
			UINT32 arraySize = param.second.arraySize > 1 ? param.second.arraySize : 1;
			const GpuParamDataTypeInfo& typeInfo = GpuParams::PARAM_SIZES.lookup[(int)param.second.type];
			UINT32 paramSize = typeInfo.numColumns * typeInfo.numRows * typeInfo.baseTypeSize;

			mDataSize += arraySize * paramSize;

		}

		auto& textureParams = shader->getTextureParams();
		auto& bufferParams = shader->getBufferParams();
		auto& samplerParams = shader->getSamplerParams();

		mNumTextureParams = (UINT32)textureParams.size();
		mNumBufferParams = (UINT32)bufferParams.size();
		mNumSamplerParams = (UINT32)samplerParams.size();

		mDataParamsBuffer = mAlloc.alloc(mDataSize);
		memset(mDataParamsBuffer, 0, mDataSize);

		mStructParams = mAlloc.construct<StructParamData>(mNumStructParams);
		mTextureParams = mAlloc.construct<TextureParamData>(mNumTextureParams);
		mBufferParams = mAlloc.construct<SPtr<GpuBuffer>>(mNumBufferParams);
		mSamplerStateParams = mAlloc.construct<SPtr<SamplerState>>(mNumSamplerParams);
		mDefaultTextureParams = mAlloc.construct<HTexture>(mNumTextureParams);
		mDefaultSamplerStateParams = mAlloc.construct<SPtr<SamplerState>>(mNumSamplerParams);

		UINT32 dataBufferIdx = 0;
		UINT32 structIdx = 0;
		UINT32 textureIdx = 0;
		UINT32 bufferIdx = 0;
		UINT32 samplerIdx = 0;

		for (auto& entry : dataParams)
		{
			ParamData& dataParam = mParams[entry.first];

			UINT32 arraySize = entry.second.arraySize > 1 ? entry.second.arraySize : 1;
			dataParam.arraySize = arraySize;
			dataParam.type = ParamType::Data;
			dataParam.dataType = entry.second.type;
			
			const GpuParamDataTypeInfo& typeInfo = GpuParams::PARAM_SIZES.lookup[(int)dataParam.dataType];
			UINT32 paramSize = typeInfo.numColumns * typeInfo.numRows * typeInfo.baseTypeSize;

			dataParam.index = dataBufferIdx;
			dataBufferIdx += arraySize * paramSize;

			if(entry.second.type == GPDT_STRUCT)
			{
				for (UINT32 i = 0; i < arraySize; i++)
				{
					StructParamData& param = mStructParams[structIdx];
					param.dataSize = entry.second.elementSize;
					param.data = mAlloc.alloc(param.dataSize);

					dataParam.index = structIdx;
					structIdx++;
				}
			}
		}

		for (auto& entry : textureParams)
		{
			ParamData& dataParam = mParams[entry.first];

			dataParam.arraySize = 1;
			dataParam.type = ParamType::Texture;
			dataParam.dataType = GPDT_UNKNOWN;
			dataParam.index = textureIdx;

			TextureParamData& param = mTextureParams[textureIdx];
			param.isLoadStore = false;

			if (entry.second.defaultValueIdx != (UINT32)-1)
				mDefaultTextureParams[textureIdx] = shader->getDefaultTexture(entry.second.defaultValueIdx);

			textureIdx++;
		}

		for (auto& entry : bufferParams)
		{
			ParamData& dataParam = mParams[entry.first];

			dataParam.arraySize = 1;
			dataParam.type = ParamType::Buffer;
			dataParam.dataType = GPDT_UNKNOWN;
			dataParam.index = bufferIdx;

			bufferIdx++;
		}

		for (auto& entry : samplerParams)
		{
			ParamData& dataParam = mParams[entry.first];

			dataParam.arraySize = 1;
			dataParam.type = ParamType::Sampler;
			dataParam.dataType = GPDT_UNKNOWN;
			dataParam.index = samplerIdx;

			if (entry.second.defaultValueIdx != (UINT32)-1)
				mDefaultSamplerStateParams[textureIdx] = shader->getDefaultSampler(entry.second.defaultValueIdx);

			samplerIdx++;
		}
	}

	MaterialParams::~MaterialParams()
	{
		if (mStructParams != nullptr)
		{
			for (UINT32 i = 0; mNumStructParams; i++)
				mAlloc.free(mStructParams[i].data);
		}

		mAlloc.free(mDataParamsBuffer);
		mAlloc.destruct(mStructParams, mNumStructParams);
		mAlloc.destruct(mTextureParams, mNumTextureParams);
		mAlloc.destruct(mBufferParams, mNumBufferParams);
		mAlloc.destruct(mSamplerStateParams, mNumSamplerParams);

		if(mDefaultTextureParams != nullptr)
			mAlloc.destruct(mDefaultTextureParams, mNumTextureParams);

		if (mDefaultSamplerStateParams != nullptr)
			mAlloc.destruct(mDefaultSamplerStateParams, mNumSamplerParams);

		mAlloc.clear();
	}

	void MaterialParams::getStructData(const String& name, void* value, UINT32 size, UINT32 arrayIdx) const
	{
		const ParamData* param = nullptr;
		GetParamResult result = getParamData(name, ParamType::Data, GPDT_STRUCT, arrayIdx, &param);
		if (result != GetParamResult::Success)
		{
			reportGetParamError(result, name, arrayIdx);
			return;
		}

		getStructData(param->index + arrayIdx, value, size);
	}

	void MaterialParams::setStructData(const String& name, const void* value, UINT32 size, UINT32 arrayIdx)
	{
		const ParamData* param = nullptr;
		GetParamResult result = getParamData(name, ParamType::Data, GPDT_STRUCT, arrayIdx, &param);
		if (result != GetParamResult::Success)
		{
			reportGetParamError(result, name, arrayIdx);
			return;
		}

		setStructData(param->index + arrayIdx, value, size);
	}

	void MaterialParams::getTexture(const String& name, HTexture& value) const
	{
		const ParamData* param = nullptr;
		GetParamResult result = getParamData(name, ParamType::Texture, GPDT_UNKNOWN, 0, &param);
		if (result != GetParamResult::Success)
		{
			reportGetParamError(result, name, 0);
			return;
		}

		getTexture(param->index, value);
	}

	void MaterialParams::setTexture(const String& name, const HTexture& value)
	{
		const ParamData* param = nullptr;
		GetParamResult result = getParamData(name, ParamType::Texture, GPDT_UNKNOWN, 0, &param);
		if (result != GetParamResult::Success)
		{
			reportGetParamError(result, name, 0);
			return;
		}

		setTexture(param->index, value);
	}

	void MaterialParams::getLoadStoreTexture(const String& name, HTexture& value, TextureSurface& surface) const
	{
		const ParamData* param = nullptr;
		GetParamResult result = getParamData(name, ParamType::Texture, GPDT_UNKNOWN, 0, &param);
		if (result != GetParamResult::Success)
		{
			reportGetParamError(result, name, 0);
			return;
		}

		getLoadStoreTexture(param->index, value, surface);
	}

	void MaterialParams::setLoadStoreTexture(const String& name, const HTexture& value, const TextureSurface& surface)
	{
		const ParamData* param = nullptr;
		GetParamResult result = getParamData(name, ParamType::Texture, GPDT_UNKNOWN, 0, &param);
		if (result != GetParamResult::Success)
		{
			reportGetParamError(result, name, 0);
			return;
		}

		setLoadStoreTexture(param->index, value, surface);
	}

	void MaterialParams::getBuffer(const String& name, SPtr<GpuBuffer>& value) const
	{
		const ParamData* param = nullptr;
		GetParamResult result = getParamData(name, ParamType::Buffer, GPDT_UNKNOWN, 0, &param);
		if (result != GetParamResult::Success)
		{
			reportGetParamError(result, name, 0);
			return;
		}

		getBuffer(param->index, value);
	}

	void MaterialParams::setBuffer(const String& name, const SPtr<GpuBuffer>& value)
	{
		const ParamData* param = nullptr;
		GetParamResult result = getParamData(name, ParamType::Buffer, GPDT_UNKNOWN, 0, &param);
		if (result != GetParamResult::Success)
		{
			reportGetParamError(result, name, 0);
			return;
		}

		setBuffer(param->index, value);
	}

	void MaterialParams::getSamplerState(const String& name, SPtr<SamplerState>& value) const
	{
		const ParamData* param = nullptr;
		GetParamResult result = getParamData(name, ParamType::Sampler, GPDT_UNKNOWN, 0, &param);
		if (result != GetParamResult::Success)
		{
			reportGetParamError(result, name, 0);
			return;
		}

		getSamplerState(param->index, value);
	}

	void MaterialParams::setSamplerState(const String& name, const SPtr<SamplerState>& value)
	{
		const ParamData* param = nullptr;
		GetParamResult result = getParamData(name, ParamType::Sampler, GPDT_UNKNOWN, 0, &param);
		if(result != GetParamResult::Success)
		{
			reportGetParamError(result, name, 0);
			return;
		}

		setSamplerState(param->index, value);
	}

	MaterialParams::GetParamResult MaterialParams::getParamData(const String& name, ParamType type, GpuParamDataType dataType,
		UINT32 arrayIdx, const ParamData** output) const
	{
		auto iterFind = mParams.find(name);
		if (iterFind == mParams.end())
			return GetParamResult::NotFound;

		const ParamData& param = iterFind->second;
		*output = &param;

		if (param.type != type || (type == ParamType::Data && param.dataType != dataType))
			return GetParamResult::InvalidType;

		if (arrayIdx >= param.arraySize)
			return GetParamResult::IndexOutOfBounds;

		return GetParamResult::Success;
	}

	void MaterialParams::reportGetParamError(GetParamResult errorCode, const String& name, UINT32 arrayIdx) const
	{
		switch(errorCode)
		{
		case GetParamResult::NotFound:
			LOGWRN("Material doesn't have a parameter named " + name + ".");
			break;
		case GetParamResult::InvalidType:
			LOGWRN("Parameter \"" + name + "\" is not of the requested type.");
			break;
		case GetParamResult::IndexOutOfBounds:
			LOGWRN("Parameter \"" + name + "\" array index " + toString(arrayIdx) + " out of range.");
			break;
		default:
			break;
		}
	}

	void MaterialParams::getStructData(UINT32 index, void* value, UINT32 size) const
	{
		const StructParamData& structParam = mStructParams[index];
		if (structParam.dataSize != size)
		{
			LOGWRN("Size mismatch when writing to a struct. Provided size was " + toString(size) + " bytes but the "
				"struct size is" + toString(structParam.dataSize) + " bytes");
			return;
		}

		memcpy(value, structParam.data, structParam.dataSize);
	}

	void MaterialParams::setStructData(UINT32 index, const void* value, UINT32 size)
	{
		const StructParamData& structParam = mStructParams[index];
		if (structParam.dataSize != size)
		{
			LOGWRN("Size mismatch when writing to a struct. Provided size was " + toString(size) + " bytes but the "
				"struct size is" + toString(structParam.dataSize) + " bytes");
			return;
		}

		memcpy(structParam.data, value, structParam.dataSize);
	}

	UINT32 MaterialParams::getStructSize(UINT32 index) const
	{
		const StructParamData& structParam = mStructParams[index];
		return structParam.dataSize;
	}

	void MaterialParams::getTexture(UINT32 index, HTexture& value) const
	{
		TextureParamData& textureParam = mTextureParams[index];
		value = textureParam.value;
	}

	void MaterialParams::setTexture(UINT32 index, const HTexture& value)
	{
		TextureParamData& textureParam = mTextureParams[index];
		textureParam.value = value;
		textureParam.isLoadStore = false;
	}

	void MaterialParams::getBuffer(UINT32 index, SPtr<GpuBuffer>& value) const
	{
		value = mBufferParams[index];
	}

	void MaterialParams::setBuffer(UINT32 index, const SPtr<GpuBuffer>& value)
	{
		mBufferParams[index] = value;
	}

	void MaterialParams::getLoadStoreTexture(UINT32 index, HTexture& value, TextureSurface& surface) const
	{
		TextureParamData& textureParam = mTextureParams[index];
		value = textureParam.value;
		surface = textureParam.surface;
	}

	void MaterialParams::setLoadStoreTexture(UINT32 index, const HTexture& value, const TextureSurface& surface)
	{
		TextureParamData& textureParam = mTextureParams[index];
		textureParam.value = value;
		textureParam.isLoadStore = true;
		textureParam.surface = surface;
	}

	bool MaterialParams::getIsTextureLoadStore(UINT32 index) const
	{
		return mTextureParams[index].isLoadStore;
	}

	void MaterialParams::getSamplerState(UINT32 index, SPtr<SamplerState>& value) const
	{
		value = mSamplerStateParams[index];
	}

	void MaterialParams::setSamplerState(UINT32 index, const SPtr<SamplerState>& value)
	{
		mSamplerStateParams[index] = value;
	}

	void MaterialParams::getDefaultTexture(UINT32 index, HTexture& value) const
	{
		value = mDefaultTextureParams[index];
	}

	void MaterialParams::getDefaultSamplerState(UINT32 index, SPtr<SamplerState>& value) const
	{
		value = mDefaultSamplerStateParams[index];
	}

	RTTITypeBase* MaterialParams::TextureParamData::getRTTIStatic()
	{
		return TextureParamDataRTTI::instance();
	}

	RTTITypeBase* MaterialParams::TextureParamData::getRTTI() const
	{
		return getRTTIStatic();
	}

	RTTITypeBase* MaterialParams::StructParamData::getRTTIStatic()
	{
		return StructParamDataRTTI::instance();
	}

	RTTITypeBase* MaterialParams::StructParamData::getRTTI() const
	{
		return getRTTIStatic();
	}

	RTTITypeBase* MaterialParams::getRTTIStatic()
	{
		return MaterialParamsRTTI::instance();
	}

	RTTITypeBase* MaterialParams::getRTTI() const
	{
		return MaterialParams::getRTTIStatic();
	}
}