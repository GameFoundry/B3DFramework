//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsShader.h"
#include "BsTechnique.h"
#include "BsException.h"
#include "BsDebug.h"
#include "BsShaderRTTI.h"
#include "BsResources.h"
#include "BsGpuParams.h"
#include "BsFrameAlloc.h"
#include "BsPass.h"
#include "BsSamplerState.h"
#include "BsTexture.h"

namespace bs
{
	template<bool Core>
	TSHADER_DESC<Core>::TSHADER_DESC()
		:queueSortType(QueueSortType::None), queuePriority(0), separablePasses(false), flags(0)
	{

	}

	template<bool Core>
	void TSHADER_DESC<Core>::addParameter(const String& name, const String& gpuVariableName, GpuParamDataType type,
		StringID rendererSemantic, UINT32 arraySize, UINT32 elementSize, UINT8* defaultValue)
	{
		if(type == GPDT_STRUCT && elementSize <= 0)
			BS_EXCEPT(InvalidParametersException, "You need to provide a non-zero element size for a struct parameter.")

		SHADER_DATA_PARAM_DESC desc;
		desc.name = name;
		desc.gpuVariableName = gpuVariableName;
		desc.type = type;
		desc.arraySize = arraySize;
		desc.rendererSemantic = rendererSemantic;
		desc.elementSize = elementSize;

		if (defaultValue != nullptr)
		{
			desc.defaultValueIdx = (UINT32)dataDefaultValues.size();
			UINT32 defaultValueSize = Shader::getDataParamSize(type);

			dataDefaultValues.resize(desc.defaultValueIdx + defaultValueSize);
			memcpy(&dataDefaultValues[desc.defaultValueIdx], defaultValue, defaultValueSize);
		}
		else
			desc.defaultValueIdx = (UINT32)-1;

		dataParams[name] = desc;
	}

	template<bool Core>
	void TSHADER_DESC<Core>::addParameter(const String& name, const String& gpuVariableName, GpuParamObjectType type, StringID rendererSemantic)
	{
		UINT32 defaultValueIdx = (UINT32)-1;

		addParameterInternal(name, gpuVariableName, type, rendererSemantic, defaultValueIdx);
	}

	template<bool Core>
	void TSHADER_DESC<Core>::addParameter(const String& name, const String& gpuVariableName, GpuParamObjectType type, const SamplerStateType& defaultValue, StringID rendererSemantic)
	{
		UINT32 defaultValueIdx = (UINT32)-1;
		if (Shader::isSampler(type) && defaultValue != nullptr)
		{
			defaultValueIdx = (UINT32)samplerDefaultValues.size();
			samplerDefaultValues.push_back(defaultValue);
		}

		addParameterInternal(name, gpuVariableName, type, rendererSemantic, defaultValueIdx);
	}

	template<bool Core>
	void TSHADER_DESC<Core>::addParameter(const String& name, const String& gpuVariableName, GpuParamObjectType type, const TextureType& defaultValue, StringID rendererSemantic)
	{
		UINT32 defaultValueIdx = (UINT32)-1;
		if (Shader::isTexture(type) && defaultValue != nullptr)
		{
			defaultValueIdx = (UINT32)textureDefaultValues.size();
			textureDefaultValues.push_back(defaultValue);
		}

		addParameterInternal(name, gpuVariableName, type, rendererSemantic, defaultValueIdx);
	}

	template<bool Core>
	void TSHADER_DESC<Core>::addParameterInternal(const String& name, const String& gpuVariableName, GpuParamObjectType type, StringID rendererSemantic, UINT32 defaultValueIdx)
	{
		Map<String, SHADER_OBJECT_PARAM_DESC>* DEST_LOOKUP[] = { &textureParams, &bufferParams, &samplerParams };
		UINT32 destIdx = 0;
		if (Shader::isBuffer(type))
			destIdx = 1;
		else if (Shader::isSampler(type))
			destIdx = 2;

		Map<String, SHADER_OBJECT_PARAM_DESC>& paramsMap = *DEST_LOOKUP[destIdx];

		auto iterFind = paramsMap.find(name);
		if (iterFind == paramsMap.end())
		{
			SHADER_OBJECT_PARAM_DESC desc;
			desc.name = name;
			desc.type = type;
			desc.rendererSemantic = rendererSemantic;
			desc.gpuVariableNames.push_back(gpuVariableName);
			desc.defaultValueIdx = defaultValueIdx;

			paramsMap[name] = desc;
		}
		else
		{
			SHADER_OBJECT_PARAM_DESC& desc = iterFind->second;

			// If same name but different properties, we ignore this param
			if (desc.type != type || desc.rendererSemantic != rendererSemantic)
				return;

			Vector<String>& gpuVariableNames = desc.gpuVariableNames;
			bool found = false;
			for (UINT32 i = 0; i < (UINT32)gpuVariableNames.size(); i++)
			{
				if (gpuVariableNames[i] == gpuVariableName)
				{
					found = true;
					break;
				}
			}

			if (!found)
				gpuVariableNames.push_back(gpuVariableName);
		}
	}

	template<bool Core>
	void TSHADER_DESC<Core>::setParamBlockAttribs(const String& name, bool shared, GpuParamBlockUsage usage, StringID rendererSemantic)
	{
		SHADER_PARAM_BLOCK_DESC desc;
		desc.name = name;
		desc.shared = shared;
		desc.usage = usage;
		desc.rendererSemantic = rendererSemantic;

		paramBlocks[name] = desc;
	}

	template struct TSHADER_DESC<false>;
	template struct TSHADER_DESC<true>;

	template<bool Core>
	TShader<Core>::TShader(const String& name, const TSHADER_DESC<Core>& desc, const Vector<SPtr<TechniqueType>>& techniques, UINT32 id)
		:mName(name), mDesc(desc), mTechniques(techniques), mId(id)
	{ }

	template<bool Core>
	TShader<Core>::~TShader() 
	{ }

	template<bool Core>
	GpuParamType TShader<Core>::getParamType(const String& name) const
	{
		auto findIterData = mDesc.dataParams.find(name);
		if (findIterData != mDesc.dataParams.end())
			return GPT_DATA;

		auto findIterTexture = mDesc.textureParams.find(name);
		if (findIterTexture != mDesc.textureParams.end())
			return GPT_TEXTURE;

		auto findIterBuffer = mDesc.bufferParams.find(name);
		if (findIterBuffer != mDesc.bufferParams.end())
			return GPT_BUFFER;

		auto findIterSampler = mDesc.samplerParams.find(name);
		if (findIterSampler != mDesc.samplerParams.end())
			return GPT_SAMPLER;

		BS_EXCEPT(InternalErrorException, "Cannot find the parameter with the name: " + name);
		return GPT_DATA;
	}

	template<bool Core>
	const SHADER_DATA_PARAM_DESC& TShader<Core>::getDataParamDesc(const String& name) const
	{
		auto findIterData = mDesc.dataParams.find(name);
		if (findIterData != mDesc.dataParams.end())
			return findIterData->second;

		BS_EXCEPT(InternalErrorException, "Cannot find the parameter with the name: " + name);
		static SHADER_DATA_PARAM_DESC dummy;
		return dummy;
	}

	template<bool Core>
	const SHADER_OBJECT_PARAM_DESC& TShader<Core>::getTextureParamDesc(const String& name) const
	{
		auto findIterObject = mDesc.textureParams.find(name);
		if (findIterObject != mDesc.textureParams.end())
			return findIterObject->second;

		BS_EXCEPT(InternalErrorException, "Cannot find the parameter with the name: " + name);
		static SHADER_OBJECT_PARAM_DESC dummy;
		return dummy;
	}

	template<bool Core>
	const SHADER_OBJECT_PARAM_DESC& TShader<Core>::getSamplerParamDesc(const String& name) const
	{
		auto findIterObject = mDesc.samplerParams.find(name);
		if (findIterObject != mDesc.samplerParams.end())
			return findIterObject->second;

		BS_EXCEPT(InternalErrorException, "Cannot find the parameter with the name: " + name);
		static SHADER_OBJECT_PARAM_DESC dummy;
		return dummy;
	}

	template<bool Core>
	const SHADER_OBJECT_PARAM_DESC& TShader<Core>::getBufferParamDesc(const String& name) const
	{
		auto findIterObject = mDesc.bufferParams.find(name);
		if (findIterObject != mDesc.bufferParams.end())
			return findIterObject->second;

		BS_EXCEPT(InternalErrorException, "Cannot find the parameter with the name: " + name);
		static SHADER_OBJECT_PARAM_DESC dummy;
		return dummy;
	}

	template<bool Core>
	bool TShader<Core>::hasDataParam(const String& name) const
	{
		auto findIterData = mDesc.dataParams.find(name);
		if (findIterData != mDesc.dataParams.end())
			return true;

		return false;
	}

	template<bool Core>
	bool TShader<Core>::hasTextureParam(const String& name) const
	{
		auto findIterObject = mDesc.textureParams.find(name);
		if (findIterObject != mDesc.textureParams.end())
			return true;

		return false;
	}

	template<bool Core>
	bool TShader<Core>::hasSamplerParam(const String& name) const
	{
		auto findIterObject = mDesc.samplerParams.find(name);
		if (findIterObject != mDesc.samplerParams.end())
			return true;

		return false;
	}

	template<bool Core>
	bool TShader<Core>::hasBufferParam(const String& name) const
	{
		auto findIterObject = mDesc.bufferParams.find(name);
		if (findIterObject != mDesc.bufferParams.end())
			return true;

		return false;
	}

	template<bool Core>
	bool TShader<Core>::hasParamBlock(const String& name) const
	{
		auto findIterObject = mDesc.paramBlocks.find(name);
		if (findIterObject != mDesc.paramBlocks.end())
			return true;

		return false;
	}

	template<bool Core>
	typename TShader<Core>::TextureType TShader<Core>::getDefaultTexture(UINT32 index) const
	{
		if (index < (UINT32)mDesc.textureDefaultValues.size())
			return mDesc.textureDefaultValues[index];

		return TextureType();
	}

	template<bool Core>
	typename TShader<Core>::SamplerStateType TShader<Core>::getDefaultSampler(UINT32 index) const
	{
		if (index < (UINT32)mDesc.samplerDefaultValues.size())
			return mDesc.samplerDefaultValues[index];

		return SamplerStateType();
	}

	template<bool Core>
	UINT8* TShader<Core>::getDefaultValue(UINT32 index) const
	{
		if (index < (UINT32)mDesc.dataDefaultValues.size())
			return (UINT8*)&mDesc.dataDefaultValues[index];

		return nullptr;
	}

	template<bool Core>
	Vector<SPtr<typename TShader<Core>::TechniqueType>> TShader<Core>::getCompatibleTechniques() const
	{
		Vector<SPtr<TechniqueType>> output;
		for (auto& technique : mTechniques)
		{
			if (technique->isSupported())
				output.push_back(technique);
		}

		return output;
	}

	template class TShader < false > ;
	template class TShader < true >;

	Shader::Shader(const String& name, const SHADER_DESC& desc, const Vector<SPtr<Technique>>& techniques, UINT32 id)
		:TShader(name, desc, techniques, id)
	{
		mMetaData = bs_shared_ptr_new<ShaderMetaData>();
	}

	SPtr<ct::Shader> Shader::getCore() const
	{
		return std::static_pointer_cast<ct::Shader>(mCoreSpecific);
	}

	void Shader::setIncludeFiles(const Vector<String>& includes)
	{
		SPtr<ShaderMetaData> meta = std::static_pointer_cast<ShaderMetaData>(getMetaData());
		meta->includes = includes;
	}

	SPtr<ct::CoreObject> Shader::createCore() const
	{
		Vector<SPtr<ct::Technique>> techniques;
		for (auto& technique : mTechniques)
			techniques.push_back(technique->getCore());

		ct::Shader* shaderCore = new (bs_alloc<ct::Shader>()) ct::Shader(mName, convertDesc(mDesc), techniques, mId);
		SPtr<ct::Shader> shaderCorePtr = bs_shared_ptr<ct::Shader>(shaderCore);
		shaderCorePtr->_setThisPtr(shaderCorePtr);

		return shaderCorePtr;
	}

	ct::SHADER_DESC Shader::convertDesc(const SHADER_DESC& desc) const
	{
		ct::SHADER_DESC output;
		output.dataParams = desc.dataParams;
		output.textureParams = desc.textureParams;
		output.samplerParams = desc.samplerParams;
		output.bufferParams = desc.bufferParams;
		output.paramBlocks = desc.paramBlocks;

		output.dataDefaultValues = desc.dataDefaultValues;

		output.samplerDefaultValues.resize(desc.samplerDefaultValues.size());
		for (UINT32 i = 0; i < (UINT32)desc.samplerDefaultValues.size(); i++)
		{
			if (desc.samplerDefaultValues[i] != nullptr)
				output.samplerDefaultValues.push_back(desc.samplerDefaultValues[i]->getCore());
			else
				output.samplerDefaultValues.push_back(nullptr);
		}

		output.textureDefaultValues.resize(desc.textureDefaultValues.size());
		for (UINT32 i = 0; i < (UINT32)desc.textureDefaultValues.size(); i++)
		{
			if (desc.textureDefaultValues[i].isLoaded())
				output.textureDefaultValues.push_back(desc.textureDefaultValues[i]->getCore());
			else
				output.textureDefaultValues.push_back(nullptr);
		}

		output.queuePriority = desc.queuePriority;
		output.queueSortType = desc.queueSortType;
		output.separablePasses = desc.separablePasses;
		output.flags = desc.flags;
		
		// Ignoring default values as they are not needed for syncing since
		// they're initialized through the material.
		return output;
	}

	void Shader::getCoreDependencies(Vector<CoreObject*>& dependencies)
	{
		for (auto& technique : mTechniques)
			dependencies.push_back(technique.get());
	}

	bool Shader::isSampler(GpuParamObjectType type)
	{
		switch(type)
		{
			case GPOT_SAMPLER1D:
			case GPOT_SAMPLER2D:
			case GPOT_SAMPLER3D:
			case GPOT_SAMPLERCUBE:
			case GPOT_SAMPLER2DMS:
				return true;
			default:
				return false;
		}
	}

	bool Shader::isTexture(GpuParamObjectType type)
	{
		switch(type)
		{
		case GPOT_TEXTURE1D:
		case GPOT_TEXTURE2D:
		case GPOT_TEXTURE3D:
		case GPOT_TEXTURECUBE:
		case GPOT_TEXTURE2DMS:
		case GPOT_TEXTURE1DARRAY:
		case GPOT_TEXTURE2DARRAY:
		case GPOT_TEXTURE2DMSARRAY:
		case GPOT_TEXTURECUBEARRAY:
			return true;
		default:
			return false;
		}
	}

	bool Shader::isLoadStoreTexture(GpuParamObjectType type)
	{
		switch (type)
		{
		case GPOT_RWTEXTURE1D:
		case GPOT_RWTEXTURE2D:
		case GPOT_RWTEXTURE3D:
		case GPOT_RWTEXTURE2DMS:
		case GPOT_RWTEXTURE1DARRAY:
		case GPOT_RWTEXTURE2DARRAY:
		case GPOT_RWTEXTURE2DMSARRAY:
			return true;
		default:
			return false;
		}
	}

	bool Shader::isBuffer(GpuParamObjectType type)
	{
		switch(type)
		{
		case GPOT_BYTE_BUFFER:
		case GPOT_STRUCTURED_BUFFER:
		case GPOT_RWBYTE_BUFFER:
		case GPOT_RWAPPEND_BUFFER:
		case GPOT_RWCONSUME_BUFFER:
		case GPOT_RWSTRUCTURED_BUFFER:
		case GPOT_RWSTRUCTURED_BUFFER_WITH_COUNTER:
		case GPOT_RWTYPED_BUFFER:
			return true;
		default:
			return false;
		}
	}

	UINT32 Shader::getDataParamSize(GpuParamDataType type)
	{
		static const GpuDataParamInfos PARAM_SIZES;

		UINT32 idx = (UINT32)type;
		if (idx < sizeof(GpuParams::PARAM_SIZES.lookup))
			return GpuParams::PARAM_SIZES.lookup[idx].size;

		return 0;
	}

	HShader Shader::create(const String& name, const SHADER_DESC& desc, const Vector<SPtr<Technique>>& techniques)
	{
		SPtr<Shader> newShader = _createPtr(name, desc, techniques);

		return static_resource_cast<Shader>(gResources()._createResourceHandle(newShader));
	}

	SPtr<Shader> Shader::_createPtr(const String& name, const SHADER_DESC& desc, const Vector<SPtr<Technique>>& techniques)
	{
		UINT32 id = ct::Shader::mNextShaderId.fetch_add(1, std::memory_order_relaxed);
		assert(id < std::numeric_limits<UINT32>::max() && "Created too many shaders, reached maximum id.");

		SPtr<Shader> newShader = bs_core_ptr<Shader>(new (bs_alloc<Shader>()) Shader(name, desc, techniques, id));
		newShader->_setThisPtr(newShader);
		newShader->initialize();

		return newShader;
	}

	SPtr<Shader> Shader::createEmpty()
	{
		SPtr<Shader> newShader = bs_core_ptr<Shader>(new (bs_alloc<Shader>()) Shader());
		newShader->_setThisPtr(newShader);

		return newShader;
	}

	RTTITypeBase* Shader::getRTTIStatic()
	{
		return ShaderRTTI::instance();
	}

	RTTITypeBase* Shader::getRTTI() const
	{
		return Shader::getRTTIStatic();
	}

	RTTITypeBase* ShaderMetaData::getRTTIStatic()
	{
		return ShaderMetaDataRTTI::instance();
	}

	RTTITypeBase* ShaderMetaData::getRTTI() const
	{
		return ShaderMetaData::getRTTIStatic();
	}

	namespace ct
	{
	std::atomic<UINT32> Shader::mNextShaderId;

	Shader::Shader(const String& name, const SHADER_DESC& desc, const Vector<SPtr<Technique>>& techniques, 
		UINT32 id)
		:TShader(name, desc, techniques, id)
	{

	}

	SPtr<Shader> Shader::create(const String& name, const SHADER_DESC& desc, 
		const Vector<SPtr<Technique>>& techniques)
	{
		UINT32 id = mNextShaderId.fetch_add(1, std::memory_order_relaxed);
		assert(id < std::numeric_limits<UINT32>::max() && "Created too many shaders, reached maximum id.");

		Shader* shaderCore = new (bs_alloc<Shader>()) Shader(name, desc, techniques, id);
		SPtr<Shader> shaderCorePtr = bs_shared_ptr<Shader>(shaderCore);
		shaderCorePtr->_setThisPtr(shaderCorePtr);
		shaderCorePtr->initialize();

		return shaderCorePtr;
	}
	}
}