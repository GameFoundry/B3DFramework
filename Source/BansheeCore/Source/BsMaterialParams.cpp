//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsMaterialParams.h"
#include "BsMaterialParamsRTTI.h"
#include "BsShader.h"
#include "BsTexture.h"
#include "BsGpuBuffer.h"
#include "BsSamplerState.h"

namespace bs
{
	MaterialParamsBase::MaterialParamsBase(
		const Map<String, SHADER_DATA_PARAM_DESC>& dataParams,
		const Map<String, SHADER_OBJECT_PARAM_DESC>& textureParams,
		const Map<String, SHADER_OBJECT_PARAM_DESC>& bufferParams,
		const Map<String, SHADER_OBJECT_PARAM_DESC>& samplerParams)
	{
		mDataSize = 0;

		for (auto& param : dataParams)
		{
			UINT32 arraySize = param.second.arraySize > 1 ? param.second.arraySize : 1;
			const GpuParamDataTypeInfo& typeInfo = GpuParams::PARAM_SIZES.lookup[(int)param.second.type];
			UINT32 paramSize = typeInfo.numColumns * typeInfo.numRows * typeInfo.baseTypeSize;

			mDataSize += arraySize * paramSize;
		}

		mNumTextureParams = (UINT32)textureParams.size();
		mNumBufferParams = (UINT32)bufferParams.size();
		mNumSamplerParams = (UINT32)samplerParams.size();

		mDataParamsBuffer = mAlloc.alloc(mDataSize);
		memset(mDataParamsBuffer, 0, mDataSize);
		
		UINT32 dataBufferIdx = 0;
		UINT32 textureIdx = 0;
		UINT32 bufferIdx = 0;
		UINT32 samplerIdx = 0;

		for (auto& entry : dataParams)
		{
			UINT32 paramIdx = (UINT32)mParams.size();
			mParams.push_back(ParamData());
			mParamLookup[entry.first] = paramIdx;

			ParamData& dataParam = mParams.back();

			UINT32 arraySize = entry.second.arraySize > 1 ? entry.second.arraySize : 1;
			dataParam.arraySize = arraySize;
			dataParam.type = ParamType::Data;
			dataParam.dataType = entry.second.type;
			dataParam.version = 1;

			const GpuParamDataTypeInfo& typeInfo = GpuParams::PARAM_SIZES.lookup[(int)dataParam.dataType];
			UINT32 paramSize = typeInfo.numColumns * typeInfo.numRows * typeInfo.baseTypeSize;

			dataParam.index = dataBufferIdx;
			dataBufferIdx += arraySize * paramSize;
		}

		for (auto& entry : textureParams)
		{
			UINT32 paramIdx = (UINT32)mParams.size();
			mParams.push_back(ParamData());
			mParamLookup[entry.first] = paramIdx;

			ParamData& dataParam = mParams.back();

			dataParam.arraySize = 1;
			dataParam.type = ParamType::Texture;
			dataParam.dataType = GPDT_UNKNOWN;
			dataParam.index = textureIdx;
			dataParam.version = 1;

			textureIdx++;
		}

		for (auto& entry : bufferParams)
		{
			UINT32 paramIdx = (UINT32)mParams.size();
			mParams.push_back(ParamData());
			mParamLookup[entry.first] = paramIdx;

			ParamData& dataParam = mParams.back();

			dataParam.arraySize = 1;
			dataParam.type = ParamType::Buffer;
			dataParam.dataType = GPDT_UNKNOWN;
			dataParam.index = bufferIdx;
			dataParam.version = 1;

			bufferIdx++;
		}

		for (auto& entry : samplerParams)
		{
			UINT32 paramIdx = (UINT32)mParams.size();
			mParams.push_back(ParamData());
			mParamLookup[entry.first] = paramIdx;

			ParamData& dataParam = mParams.back();

			dataParam.arraySize = 1;
			dataParam.type = ParamType::Sampler;
			dataParam.dataType = GPDT_UNKNOWN;
			dataParam.index = samplerIdx;
			dataParam.version = 1;

			samplerIdx++;
		}
	}

	MaterialParamsBase::~MaterialParamsBase()
	{
		mAlloc.free(mDataParamsBuffer);
		
		mAlloc.clear();
	}

	UINT32 MaterialParamsBase::getParamIndex(const String& name) const
	{
		auto iterFind = mParamLookup.find(name);
		if (iterFind == mParamLookup.end())
			return (UINT32)-1;

		return iterFind->second;
	}

	MaterialParamsBase::GetParamResult MaterialParamsBase::getParamIndex(const String& name, ParamType type,
		GpuParamDataType dataType, UINT32 arrayIdx, UINT32& output) const
	{
		auto iterFind = mParamLookup.find(name);
		if (iterFind == mParamLookup.end())
			return GetParamResult::NotFound;

		UINT32 index = iterFind->second;
		const ParamData& param = mParams[index];
		
		if (param.type != type || (type == ParamType::Data && param.dataType != dataType))
			return GetParamResult::InvalidType;

		if (arrayIdx >= param.arraySize)
			return GetParamResult::IndexOutOfBounds;

		output = index;
		return GetParamResult::Success;
	}

	MaterialParamsBase::GetParamResult MaterialParamsBase::getParamData(const String& name, ParamType type, 
		GpuParamDataType dataType, UINT32 arrayIdx, const ParamData** output) const
	{
		auto iterFind = mParamLookup.find(name);
		if (iterFind == mParamLookup.end())
			return GetParamResult::NotFound;

		UINT32 index = iterFind->second;
		const ParamData& param = mParams[index];
		*output = &param;

		if (param.type != type || (type == ParamType::Data && param.dataType != dataType))
			return GetParamResult::InvalidType;

		if (arrayIdx >= param.arraySize)
			return GetParamResult::IndexOutOfBounds;

		return GetParamResult::Success;
	}

	void MaterialParamsBase::reportGetParamError(GetParamResult errorCode, const String& name, UINT32 arrayIdx) const
	{
		switch (errorCode)
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

	RTTITypeBase* MaterialParamStructData::getRTTIStatic()
	{
		return MaterialParamStructDataRTTI::instance();
	}

	RTTITypeBase* MaterialParamStructData::getRTTI() const
	{
		return getRTTIStatic();
	}

	RTTITypeBase* MaterialParamTextureData::getRTTIStatic()
	{
		return MaterialParamTextureDataRTTI::instance();
	}

	RTTITypeBase* MaterialParamTextureData::getRTTI() const
	{
		return getRTTIStatic();
	}

	template<bool Core>
	TMaterialParams<Core>::TMaterialParams(const ShaderType& shader)
		:MaterialParamsBase(
			shader->getDataParams(),
			shader->getTextureParams(),
			shader->getBufferParams(),
			shader->getSamplerParams())
	{
		auto& dataParams = shader->getDataParams();
		auto& textureParams = shader->getTextureParams();
		auto& bufferParams = shader->getBufferParams();
		auto& samplerParams = shader->getSamplerParams();

		mStructParams = mAlloc.construct<ParamStructDataType>(mNumStructParams);
		mTextureParams = mAlloc.construct<ParamTextureDataType>(mNumTextureParams);
		mBufferParams = mAlloc.construct<ParamBufferDataType>(mNumBufferParams);
		mSamplerStateParams = mAlloc.construct<ParamSamplerStateDataType>(mNumSamplerParams);
		mDefaultTextureParams = mAlloc.construct<TextureType>(mNumTextureParams);
		mDefaultSamplerStateParams = mAlloc.construct<SamplerType>(mNumSamplerParams);

		UINT32 structIdx = 0;
		UINT32 textureIdx = 0;
		UINT32 samplerIdx = 0;

		for (auto& entry : dataParams)
		{
			if(entry.second.type == GPDT_STRUCT)
			{
				UINT32 arraySize = entry.second.arraySize > 1 ? entry.second.arraySize : 1;
				for (UINT32 i = 0; i < arraySize; i++)
				{
					ParamStructDataType& param = mStructParams[structIdx];
					param.dataSize = entry.second.elementSize;
					param.data = mAlloc.alloc(param.dataSize);

					structIdx++;
				}
			}
		}

		for (auto& entry : textureParams)
		{
			ParamTextureDataType& param = mTextureParams[textureIdx];
			param.isLoadStore = false;

			if (entry.second.defaultValueIdx != (UINT32)-1)
				mDefaultTextureParams[textureIdx] = shader->getDefaultTexture(entry.second.defaultValueIdx);

			textureIdx++;
		}

		for (auto& entry : samplerParams)
		{
			if (entry.second.defaultValueIdx != (UINT32)-1)
				mDefaultSamplerStateParams[samplerIdx] = shader->getDefaultSampler(entry.second.defaultValueIdx);

			samplerIdx++;
		}
	}

	template<bool Core>
	TMaterialParams<Core>::~TMaterialParams()
	{
		if (mStructParams != nullptr)
		{
			for (UINT32 i = 0; mNumStructParams; i++)
				mAlloc.free(mStructParams[i].data);
		}

		mAlloc.destruct(mStructParams, mNumStructParams);
		mAlloc.destruct(mTextureParams, mNumTextureParams);
		mAlloc.destruct(mBufferParams, mNumBufferParams);
		mAlloc.destruct(mSamplerStateParams, mNumSamplerParams);

		if(mDefaultTextureParams != nullptr)
			mAlloc.destruct(mDefaultTextureParams, mNumTextureParams);

		if (mDefaultSamplerStateParams != nullptr)
			mAlloc.destruct(mDefaultSamplerStateParams, mNumSamplerParams);
	}

	template<bool Core>
	void TMaterialParams<Core>::getStructData(const String& name, void* value, UINT32 size, UINT32 arrayIdx) const
	{
		const ParamData* param = nullptr;
		GetParamResult result = getParamData(name, ParamType::Data, GPDT_STRUCT, arrayIdx, &param);
		if (result != GetParamResult::Success)
		{
			reportGetParamError(result, name, arrayIdx);
			return;
		}

		getStructData(*param,  value, size, arrayIdx);
	}

	template<bool Core>
	void TMaterialParams<Core>::setStructData(const String& name, const void* value, UINT32 size, UINT32 arrayIdx)
	{
		const ParamData* param = nullptr;
		GetParamResult result = getParamData(name, ParamType::Data, GPDT_STRUCT, arrayIdx, &param);
		if (result != GetParamResult::Success)
		{
			reportGetParamError(result, name, arrayIdx);
			return;
		}

		setStructData(*param, value, size, arrayIdx);
	}

	template<bool Core>
	void TMaterialParams<Core>::getTexture(const String& name, TextureType& value, TextureSurface& surface) const
	{
		const ParamData* param = nullptr;
		GetParamResult result = getParamData(name, ParamType::Texture, GPDT_UNKNOWN, 0, &param);
		if (result != GetParamResult::Success)
		{
			reportGetParamError(result, name, 0);
			return;
		}

		getTexture(*param, value, surface);
	}

	template<bool Core>
	void TMaterialParams<Core>::setTexture(const String& name, const TextureType& value, const TextureSurface& surface)
	{
		const ParamData* param = nullptr;
		GetParamResult result = getParamData(name, ParamType::Texture, GPDT_UNKNOWN, 0, &param);
		if (result != GetParamResult::Success)
		{
			reportGetParamError(result, name, 0);
			return;
		}

		setTexture(*param, value, surface);
	}

	template<bool Core>
	void TMaterialParams<Core>::getLoadStoreTexture(const String& name, TextureType& value, TextureSurface& surface) const
	{
		const ParamData* param = nullptr;
		GetParamResult result = getParamData(name, ParamType::Texture, GPDT_UNKNOWN, 0, &param);
		if (result != GetParamResult::Success)
		{
			reportGetParamError(result, name, 0);
			return;
		}

		getLoadStoreTexture(*param, value, surface);
	}

	template<bool Core>
	void TMaterialParams<Core>::setLoadStoreTexture(const String& name, const TextureType& value, const TextureSurface& surface)
	{
		const ParamData* param = nullptr;
		GetParamResult result = getParamData(name, ParamType::Texture, GPDT_UNKNOWN, 0, &param);
		if (result != GetParamResult::Success)
		{
			reportGetParamError(result, name, 0);
			return;
		}

		setLoadStoreTexture(*param, value, surface);
	}

	template<bool Core>
	void TMaterialParams<Core>::getBuffer(const String& name, BufferType& value) const
	{
		const ParamData* param = nullptr;
		GetParamResult result = getParamData(name, ParamType::Buffer, GPDT_UNKNOWN, 0, &param);
		if (result != GetParamResult::Success)
		{
			reportGetParamError(result, name, 0);
			return;
		}

		getBuffer(*param, value);
	}

	template<bool Core>
	void TMaterialParams<Core>::setBuffer(const String& name, const BufferType& value)
	{
		const ParamData* param = nullptr;
		GetParamResult result = getParamData(name, ParamType::Buffer, GPDT_UNKNOWN, 0, &param);
		if (result != GetParamResult::Success)
		{
			reportGetParamError(result, name, 0);
			return;
		}

		setBuffer(*param, value);
	}

	template<bool Core>
	void TMaterialParams<Core>::getSamplerState(const String& name, SamplerType& value) const
	{
		const ParamData* param = nullptr;
		GetParamResult result = getParamData(name, ParamType::Sampler, GPDT_UNKNOWN, 0, &param);
		if (result != GetParamResult::Success)
		{
			reportGetParamError(result, name, 0);
			return;
		}

		getSamplerState(*param, value);
	}

	template<bool Core>
	void TMaterialParams<Core>::setSamplerState(const String& name, const SamplerType& value)
	{
		const ParamData* param = nullptr;
		GetParamResult result = getParamData(name, ParamType::Sampler, GPDT_UNKNOWN, 0, &param);
		if(result != GetParamResult::Success)
		{
			reportGetParamError(result, name, 0);
			return;
		}

		setSamplerState(*param, value);
	}

	template<bool Core>
	void TMaterialParams<Core>::getStructData(const ParamData& param, void* value, UINT32 size, UINT32 arrayIdx) const
	{
		const ParamStructDataType& structParam = mStructParams[param.index + arrayIdx];
		if (structParam.dataSize != size)
		{
			LOGWRN("Size mismatch when writing to a struct. Provided size was " + toString(size) + " bytes but the "
				"struct size is" + toString(structParam.dataSize) + " bytes");
			return;
		}

		memcpy(value, structParam.data, structParam.dataSize);
	}

	template<bool Core>
	void TMaterialParams<Core>::setStructData(const ParamData& param, const void* value, UINT32 size, UINT32 arrayIdx)
	{
		const ParamStructDataType& structParam = mStructParams[param.index + arrayIdx];
		if (structParam.dataSize != size)
		{
			LOGWRN("Size mismatch when writing to a struct. Provided size was " + toString(size) + " bytes but the "
				"struct size is" + toString(structParam.dataSize) + " bytes");
			return;
		}

		memcpy(structParam.data, value, structParam.dataSize);
		param.version = ++mParamVersion;
	}

	template<bool Core>
	UINT32 TMaterialParams<Core>::getStructSize(const ParamData& param) const
	{
		const ParamStructDataType& structParam = mStructParams[param.index];
		return structParam.dataSize;
	}

	template<bool Core>
	void TMaterialParams<Core>::getTexture(const ParamData& param, TextureType& value, TextureSurface& surface) const
	{
		ParamTextureDataType& textureParam = mTextureParams[param.index];
		value = textureParam.value;
		surface = textureParam.surface;
	}
	
	template<bool Core>
	void TMaterialParams<Core>::setTexture(const ParamData& param, const TextureType& value, const TextureSurface& surface)
	{
		ParamTextureDataType& textureParam = mTextureParams[param.index];
		textureParam.value = value;
		textureParam.isLoadStore = false;
		textureParam.surface = surface;

		param.version = ++mParamVersion;
	}

	template<bool Core>
	void TMaterialParams<Core>::getBuffer(const ParamData& param, BufferType& value) const
	{
		value = mBufferParams[param.index].value;
	}

	template<bool Core>
	void TMaterialParams<Core>::setBuffer(const ParamData& param, const BufferType& value)
	{
		mBufferParams[param.index].value = value;

		param.version = ++mParamVersion;
	}

	template<bool Core>
	void TMaterialParams<Core>::getLoadStoreTexture(const ParamData& param, TextureType& value, TextureSurface& surface) const
	{
		ParamTextureDataType& textureParam = mTextureParams[param.index];
		value = textureParam.value;
		surface = textureParam.surface;
	}

	template<bool Core>
	void TMaterialParams<Core>::setLoadStoreTexture(const ParamData& param, const TextureType& value, const TextureSurface& surface)
	{
		ParamTextureDataType& textureParam = mTextureParams[param.index];
		textureParam.value = value;
		textureParam.isLoadStore = true;
		textureParam.surface = surface;

		param.version = ++mParamVersion;
	}

	template<bool Core>
	bool TMaterialParams<Core>::getIsTextureLoadStore(const ParamData& param) const
	{
		return mTextureParams[param.index].isLoadStore;
	}

	template<bool Core>
	void TMaterialParams<Core>::getSamplerState(const ParamData& param, SamplerType& value) const
	{
		value = mSamplerStateParams[param.index].value;
	}

	template<bool Core>
	void TMaterialParams<Core>::setSamplerState(const ParamData& param, const SamplerType& value)
	{
		mSamplerStateParams[param.index].value = value;

		param.version = ++mParamVersion;
	}

	template<bool Core>
	void TMaterialParams<Core>::getDefaultTexture(const ParamData& param, TextureType& value) const
	{
		value = mDefaultTextureParams[param.index];
	}

	template<bool Core>
	void TMaterialParams<Core>::getDefaultSamplerState(const ParamData& param, SamplerType& value) const
	{
		value = mDefaultSamplerStateParams[param.index];
	}

	template class TMaterialParams<true>;
	template class TMaterialParams<false>;

	MaterialParams::MaterialParams(const HShader& shader)
		:TMaterialParams(shader), mLastSyncVersion(1)
	{ }

	void MaterialParams::getSyncData(UINT8* buffer, UINT32& size, bool forceAll)
	{
		// Note: Not syncing struct data

		UINT32 numDirtyDataParams = 0;
		UINT32 numDirtyTextureParams = 0;
		UINT32 numDirtyBufferParams = 0;
		UINT32 numDirtySamplerParams = 0;

		UINT32 dataParamSize = 0;
		for(auto& param : mParams)
		{
			if (param.version <= mLastSyncVersion && !forceAll)
				continue;

			switch(param.type)
			{
			case ParamType::Data:
			{
				numDirtyDataParams++;

				UINT32 arraySize = param.arraySize > 1 ? param.arraySize : 1;
				const GpuParamDataTypeInfo& typeInfo = GpuParams::PARAM_SIZES.lookup[(int)param.dataType];
				UINT32 paramSize = typeInfo.numColumns * typeInfo.numRows * typeInfo.baseTypeSize;

				dataParamSize += arraySize * paramSize;
			}
				break;
			case ParamType::Texture:
				numDirtyTextureParams++;
				break;
			case ParamType::Buffer:
				numDirtyBufferParams++;
				break;
			case ParamType::Sampler:
				numDirtySamplerParams++;
				break;
			}
		}

		UINT32 textureEntrySize = sizeof(MaterialParamTextureDataCore) + sizeof(UINT32);
		UINT32 bufferEntrySize = sizeof(MaterialParamBufferDataCore) + sizeof(UINT32);
		UINT32 samplerStateEntrySize = sizeof(MaterialParamSamplerStateDataCore) + sizeof(UINT32);

		UINT32 dataParamsOffset = sizeof(UINT32) * 4;
		UINT32 textureParamsOffset = dataParamsOffset + dataParamSize + sizeof(UINT32) * numDirtyDataParams;
		UINT32 bufferParamsOffset = textureParamsOffset + textureEntrySize * numDirtyTextureParams;
		UINT32 samplerStateParamsOffset = bufferParamsOffset + bufferEntrySize * numDirtyBufferParams;

		UINT32 totalSize = samplerStateParamsOffset + samplerStateEntrySize * numDirtySamplerParams;

		if (buffer == nullptr)
		{
			size = totalSize;
			return;
		}

		if(size != totalSize)
		{
			LOGERR("Invalid buffer size provided, ignoring.");
			return;
		}

		char* writeDest = (char*)buffer;
		writeDest = rttiWriteElem(numDirtyDataParams, writeDest);
		writeDest = rttiWriteElem(numDirtyTextureParams, writeDest);
		writeDest = rttiWriteElem(numDirtyBufferParams, writeDest);
		writeDest = rttiWriteElem(numDirtySamplerParams, writeDest);

		UINT32 dirtyDataParamOffset = 0;
		UINT32 dirtyTextureParamIdx = 0;
		UINT32 dirtyBufferParamIdx = 0;
		UINT32 dirtySamplerParamIdx = 0;

		for(UINT32 i = 0; i < (UINT32)mParams.size(); i++)
		{
			ParamData& param = mParams[i];
			if (param.version <= mLastSyncVersion && !forceAll)
				continue;

			switch (param.type)
			{
			case ParamType::Data:
			{
				UINT32 arraySize = param.arraySize > 1 ? param.arraySize : 1;
				const GpuParamDataTypeInfo& typeInfo = GpuParams::PARAM_SIZES.lookup[(int)param.dataType];
				UINT32 paramSize = typeInfo.numColumns * typeInfo.numRows * typeInfo.baseTypeSize;

				UINT32 dataSize = arraySize * paramSize;

				writeDest = (char*)buffer + dataParamsOffset + dirtyDataParamOffset;
				writeDest = rttiWriteElem(i, writeDest);
				memcpy(writeDest, &mDataParamsBuffer[param.index], dataSize);
				writeDest += dataSize;

				dirtyDataParamOffset += dataSize + sizeof(UINT32);
			}
			break;
			case ParamType::Texture:
			{
				writeDest = (char*)buffer + textureParamsOffset + dirtyTextureParamIdx * textureEntrySize;
				writeDest = rttiWriteElem(i, writeDest);

				const MaterialParamTextureData& textureData = mTextureParams[param.index];
				MaterialParamTextureDataCore* coreTexData = (MaterialParamTextureDataCore*)writeDest;
				new (coreTexData) MaterialParamTextureDataCore();

				coreTexData->isLoadStore = textureData.isLoadStore;
				coreTexData->surface = textureData.surface;

				if (textureData.value.isLoaded())
					coreTexData->value = textureData.value->getCore();

				dirtyTextureParamIdx++;
			}
				break;
			case ParamType::Buffer:
			{
				writeDest = (char*)buffer + bufferParamsOffset + dirtyBufferParamIdx * bufferEntrySize;
				writeDest = rttiWriteElem(i, writeDest);

				const MaterialParamBufferData& bufferData = mBufferParams[param.index];
				MaterialParamBufferDataCore* coreBufferData = (MaterialParamBufferDataCore*)writeDest;
				new (coreBufferData) MaterialParamBufferDataCore();

				if(bufferData.value != nullptr)
					coreBufferData->value = bufferData.value->getCore();

				dirtyBufferParamIdx++;
			}
				break;
			case ParamType::Sampler:
			{
				writeDest = (char*)buffer + samplerStateParamsOffset + dirtySamplerParamIdx * samplerStateEntrySize;
				writeDest = rttiWriteElem(i, writeDest);

				const MaterialParamSamplerStateData& samplerData = mSamplerStateParams[param.index];
				MaterialParamSamplerStateDataCore* coreSamplerData = (MaterialParamSamplerStateDataCore*)writeDest;
				new (coreSamplerData) MaterialParamSamplerStateDataCore();

				if (samplerData.value != nullptr)
					coreSamplerData->value = samplerData.value->getCore();

				dirtySamplerParamIdx++;
			}
				break;
			}
		}

		mLastSyncVersion = mParamVersion;
	}

	void MaterialParams::getResourceDependencies(Vector<HResource>& resources)
	{
		for (UINT32 i = 0; i < (UINT32)mParams.size(); i++)
		{
			ParamData& param = mParams[i];
			if (param.type != ParamType::Texture)
				continue;

			const MaterialParamTextureData& textureData = mTextureParams[param.index];
			if (textureData.value != nullptr)
				resources.push_back(textureData.value);
		}
	}

	void MaterialParams::getCoreObjectDependencies(Vector<CoreObject*>& coreObjects)
	{
		for (UINT32 i = 0; i < (UINT32)mParams.size(); i++)
		{
			ParamData& param = mParams[i];

			switch (param.type)
			{
			case ParamType::Texture:
			{
				const MaterialParamTextureData& textureData = mTextureParams[param.index];

				if (textureData.value.isLoaded())
					coreObjects.push_back(textureData.value.get());
			}
			break;
			case ParamType::Buffer:
			{
				const MaterialParamBufferData& bufferData = mBufferParams[param.index];

				if (bufferData.value != nullptr)
					coreObjects.push_back(bufferData.value.get());
			}
			break;
			case ParamType::Sampler:
			{

				const MaterialParamSamplerStateData& samplerData = mSamplerStateParams[param.index];

				if (samplerData.value != nullptr)
					coreObjects.push_back(samplerData.value.get());
			}
			break;
			}
		}
	}

	RTTITypeBase* MaterialParams::getRTTIStatic()
	{
		return MaterialParamsRTTI::instance();
	}

	RTTITypeBase* MaterialParams::getRTTI() const
	{
		return MaterialParams::getRTTIStatic();
	}

	namespace ct
	{
	MaterialParams::MaterialParams(const SPtr<Shader>& shader)
		:TMaterialParams(shader)
	{ }

	MaterialParams::MaterialParams(const SPtr<Shader>& shader, const SPtr<bs::MaterialParams>& params)
		: TMaterialParams(shader)
	{
		memcpy(mDataParamsBuffer, params->mDataParamsBuffer, mDataSize);

		for (auto& param : mParams)
		{
			switch (param.type)
			{
			case ParamType::Texture:
			{
				HTexture texture = params->mTextureParams[param.index].value;
				SPtr<Texture> textureCore;
				if (texture.isLoaded())
					textureCore = texture->getCore();

				mTextureParams[param.index].value = textureCore;
				mTextureParams[param.index].isLoadStore = params->mTextureParams[param.index].isLoadStore;
				mTextureParams[param.index].surface = params->mTextureParams[param.index].surface;
			}
				break;
			case ParamType::Buffer:
			{
				SPtr<bs::GpuBuffer> buffer = params->mBufferParams[param.index].value;
				SPtr<GpuBuffer> bufferCore;
				if (buffer != nullptr)
					bufferCore = buffer->getCore();

				mBufferParams[param.index].value = bufferCore;
			}
				break;
			case ParamType::Sampler:
			{
				SPtr<bs::SamplerState> sampState = params->mSamplerStateParams[param.index].value;
				SPtr<SamplerState> sampStateCore;
				if (sampState != nullptr)
					sampStateCore = sampState->getCore();

				mSamplerStateParams[param.index].value = sampStateCore;
			}
				break;
			}
		}
	}

	void MaterialParams::setSyncData(UINT8* buffer, UINT32 size)
	{
		char* sourceData = (char*)buffer;

		UINT32 numDirtyDataParams = 0;
		UINT32 numDirtyTextureParams = 0;
		UINT32 numDirtyBufferParams = 0;
		UINT32 numDirtySamplerParams = 0;

		sourceData = rttiReadElem(numDirtyDataParams, sourceData);
		sourceData = rttiReadElem(numDirtyTextureParams, sourceData);
		sourceData = rttiReadElem(numDirtyBufferParams, sourceData);
		sourceData = rttiReadElem(numDirtySamplerParams, sourceData);

		mParamVersion++;

		for(UINT32 i = 0; i < numDirtyDataParams; i++)
		{
			UINT32 paramIdx = 0;
			sourceData = rttiReadElem(paramIdx, sourceData);

			ParamData& param = mParams[paramIdx];
			param.version = mParamVersion;

			UINT32 arraySize = param.arraySize > 1 ? param.arraySize : 1;
			const GpuParamDataTypeInfo& typeInfo = bs::GpuParams::PARAM_SIZES.lookup[(int)param.dataType];
			UINT32 paramSize = typeInfo.numColumns * typeInfo.numRows * typeInfo.baseTypeSize;

			UINT32 dataParamSize = arraySize * paramSize;
			memcpy(&mDataParamsBuffer[param.index], sourceData, dataParamSize);
			sourceData += dataParamSize;
		}

		for(UINT32 i = 0; i < numDirtyTextureParams; i++)
		{
			UINT32 paramIdx = 0;
			sourceData = rttiReadElem(paramIdx, sourceData);

			ParamData& param = mParams[paramIdx];
			param.version = mParamVersion;

			MaterialParamTextureDataCore* sourceTexData = (MaterialParamTextureDataCore*)sourceData;
			sourceData += sizeof(MaterialParamTextureDataCore);

			mTextureParams[param.index] = *sourceTexData;
			sourceTexData->~MaterialParamTextureDataCore();
		}

		for (UINT32 i = 0; i < numDirtyBufferParams; i++)
		{
			UINT32 paramIdx = 0;
			sourceData = rttiReadElem(paramIdx, sourceData);

			ParamData& param = mParams[paramIdx];
			param.version = mParamVersion;

			MaterialParamBufferDataCore* sourceBufferData = (MaterialParamBufferDataCore*)sourceData;
			sourceData += sizeof(MaterialParamBufferDataCore);

			mBufferParams[param.index] = *sourceBufferData;
			sourceBufferData->~MaterialParamBufferDataCore();
		}

		for (UINT32 i = 0; i < numDirtySamplerParams; i++)
		{
			UINT32 paramIdx = 0;
			sourceData = rttiReadElem(paramIdx, sourceData);

			ParamData& param = mParams[paramIdx];
			param.version = mParamVersion;

			MaterialParamSamplerStateDataCore* sourceSamplerStateData = (MaterialParamSamplerStateDataCore*)sourceData;
			sourceData += sizeof(MaterialParamSamplerStateDataCore);

			mSamplerStateParams[param.index] = *sourceSamplerStateData;
			sourceSamplerStateData->~MaterialParamSamplerStateDataCore();
		}
	}
	}
}