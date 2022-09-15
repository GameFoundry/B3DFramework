//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Material/BsShader.h"
#include "Material/BsTechnique.h"
#include "Error/BsException.h"
#include "Debug/BsDebug.h"
#include "Private/RTTI/BsShaderRTTI.h"
#include "Resources/BsResources.h"
#include "RenderAPI/BsGpuParams.h"
#include "Material/BsPass.h"
#include "RenderAPI/BsSamplerState.h"
#include "Image/BsTexture.h"

namespace bs
{
	RTTITypeBase* SubShader::GetRttiStatic()
	{
		return SubShaderRTTI::Instance();
	}

	RTTITypeBase* SubShader::GetRtti() const
	{
		return SubShader::GetRttiStatic();
	}

	template<bool Core>
	TSHADER_DESC<Core>::TSHADER_DESC()
		:queueSortType(QueueSortType::None), queuePriority(0), separablePasses(false), flags(0)
	{

	}

	template<bool Core>
	void TSHADER_DESC<Core>::AddParameter(SHADER_DATA_PARAM_DESC paramDesc, UINT8* defaultValue)
	{
		if(paramDesc.type == GPDT_STRUCT && paramDesc.elementSize <= 0)
		{
			BS_LOG(Error, Material, "You need to provide a non-zero element size for a struct parameter.");
			return;
		}

		const auto iterFind = dataParams.find(paramDesc.name);
		if(iterFind != dataParams.end())
			return;

		if (defaultValue != nullptr)
		{
			paramDesc.defaultValueIdx = (UINT32)dataDefaultValues.size();
			UINT32 defaultValueSize = Shader::GetDataParamSize(paramDesc.type);

			dataDefaultValues.resize(paramDesc.defaultValueIdx + defaultValueSize);
			memcpy(&dataDefaultValues[paramDesc.defaultValueIdx], defaultValue, defaultValueSize);
		}
		else
			paramDesc.defaultValueIdx = (UINT32)-1;

		dataParams[paramDesc.name] = paramDesc;
	}

	template<bool Core>
	void TSHADER_DESC<Core>::AddParameter(SHADER_OBJECT_PARAM_DESC paramDesc)
	{
		UINT32 defaultValueIdx = (UINT32)-1;

		AddParameterInternal(std::move(paramDesc), defaultValueIdx);
	}

	template<bool Core>
	void TSHADER_DESC<Core>::AddParameter(SHADER_OBJECT_PARAM_DESC paramDesc, const SamplerStateType& defaultValue)
	{
		UINT32 defaultValueIdx = (UINT32)-1;
		if (Shader::IsSampler(paramDesc.type) && defaultValue != nullptr)
		{
			defaultValueIdx = (UINT32)samplerDefaultValues.size();
			samplerDefaultValues.push_back(defaultValue);
		}

		AddParameterInternal(std::move(paramDesc), defaultValueIdx);
	}

	template<bool Core>
	void TSHADER_DESC<Core>::AddParameter(SHADER_OBJECT_PARAM_DESC paramDesc, const TextureType& defaultValue)
	{
		UINT32 defaultValueIdx = (UINT32)-1;
		if (Shader::IsTexture(paramDesc.type) && defaultValue != nullptr)
		{
			defaultValueIdx = (UINT32)textureDefaultValues.size();
			textureDefaultValues.push_back(defaultValue);
		}

		AddParameterInternal(std::move(paramDesc), defaultValueIdx);
	}

	template<bool Core>
	void TSHADER_DESC<Core>::AddParameterInternal(SHADER_OBJECT_PARAM_DESC paramDesc, UINT32 defaultValueIdx)
	{
		Map<String, SHADER_OBJECT_PARAM_DESC>* DEST_LOOKUP[] = { &textureParams, &bufferParams, &samplerParams };
		UINT32 destIdx = 0;
		if (Shader::IsBuffer(paramDesc.type))
			destIdx = 1;
		else if (Shader::IsSampler(paramDesc.type))
			destIdx = 2;

		Map<String, SHADER_OBJECT_PARAM_DESC>& paramsMap = *DEST_LOOKUP[destIdx];

		auto iterFind = paramsMap.find(paramDesc.name);
		if (iterFind == paramsMap.end())
		{
			paramDesc.defaultValueIdx = defaultValueIdx;
			paramsMap[paramDesc.name] = paramDesc;
		}
		else
		{
			SHADER_OBJECT_PARAM_DESC& desc = iterFind->second;

			// If same name but different properties, we ignore this param
			if (desc.type != paramDesc.type || desc.rendererSemantic != paramDesc.rendererSemantic)
				return;

			Vector<String>& gpuVariableNames = desc.gpuVariableNames;
			bool found = false;
			for (UINT32 i = 0; i < (UINT32)gpuVariableNames.size(); i++)
			{
				if (gpuVariableNames[i] == paramDesc.gpuVariableName)
				{
					found = true;
					break;
				}
			}

			if (!found)
				gpuVariableNames.push_back(paramDesc.gpuVariableName);
		}
	}

	template<bool Core>
	void TSHADER_DESC<Core>::SetParameterAttribute(const String& name, const SHADER_PARAM_ATTRIBUTE& attrib)
	{
		SHADER_DATA_PARAM_DESC* paramDescData = nullptr;

		const auto findIterData = dataParams.find(name);
		if (findIterData != dataParams.end())
			paramDescData = &findIterData->second;

		SHADER_OBJECT_PARAM_DESC* paramDescObj = nullptr;
		if(!paramDescData)
		{
			const auto findIterTexture = textureParams.find(name);
			if (findIterTexture != textureParams.end())
				paramDescObj = &findIterTexture->second;

			if (!paramDescObj)
			{
				const auto findIterSampler = samplerParams.find(name);
				if (findIterSampler != samplerParams.end())
					paramDescObj = &findIterSampler->second;
			}

			if (!paramDescObj)
			{
				const auto findIterBuffer = bufferParams.find(name);
				if (findIterBuffer != bufferParams.end())
					paramDescObj = &findIterBuffer->second;
			}
		}

		SHADER_PARAM_COMMON* paramDesc = paramDescData;
		if(!paramDesc)
			paramDesc = paramDescObj;

		if(!paramDesc)
		{
			BS_LOG(Warning, Material, "Attempting to apply a shader parameter attribute to a non-existing parameter.");
			return;
		}

		if(attrib.type == ShaderParamAttributeType::SpriteUV)
		{
			if(paramDescObj)
			{
				BS_LOG(Warning, Material, "Attempting to apply SpriteUV attribute to an object parameter is not supported.");
				return;
			}

			if(paramDescData->type != GPDT_FLOAT4)
			{
				BS_LOG(Warning, Material, "SpriteUV attribute can only be applied to 4D vectors.");
				return;
			}
		}

		// Look for duplicate attributes
		UINT32 curAttribIdx = paramDesc->attribIdx;
		bool found = false;
		while(curAttribIdx != (UINT32)-1)
		{
			SHADER_PARAM_ATTRIBUTE& curAttrib = paramAttributes[curAttribIdx];
			if(curAttrib.type == attrib.type)
			{
				curAttrib = attrib;

				found = true;
				break;
			}

			curAttribIdx = curAttrib.nextParamIdx;
		}

		if(!found)
		{
			const auto attribIdx = (UINT32)paramAttributes.size();
			paramAttributes.emplace_back(attrib);

			if (paramDesc->attribIdx != (UINT32)-1)
				paramAttributes.back().nextParamIdx = paramDesc->attribIdx;

			paramDesc->attribIdx = attribIdx;
		}
	}

	template<bool Core>
	void TSHADER_DESC<Core>::SetParamBlockAttribs(const String& name, bool shared, GpuBufferUsage usage,
		StringID rendererSemantic)
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
	TShader<Core>::TShader(UINT32 id)
		:mId(id)
	{ }

	template<bool Core>
	TShader<Core>::TShader(const String& name, const TSHADER_DESC<Core>& desc, UINT32 id)
		:mName(name), mDesc(desc), mId(id)
	{ }

	template<bool Core>
	TShader<Core>::~TShader()
	{ }

	template<bool Core>
	GpuParamType TShader<Core>::GetParamType(const String& name) const
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
	const SHADER_DATA_PARAM_DESC& TShader<Core>::GetDataParamDesc(const String& name) const
	{
		auto findIterData = mDesc.dataParams.find(name);
		if (findIterData != mDesc.dataParams.end())
			return findIterData->second;

		BS_EXCEPT(InternalErrorException, "Cannot find the parameter with the name: " + name);
		static SHADER_DATA_PARAM_DESC dummy;
		return dummy;
	}

	template<bool Core>
	const SHADER_OBJECT_PARAM_DESC& TShader<Core>::GetTextureParamDesc(const String& name) const
	{
		auto findIterObject = mDesc.textureParams.find(name);
		if (findIterObject != mDesc.textureParams.end())
			return findIterObject->second;

		BS_EXCEPT(InternalErrorException, "Cannot find the parameter with the name: " + name);
		static SHADER_OBJECT_PARAM_DESC dummy;
		return dummy;
	}

	template<bool Core>
	const SHADER_OBJECT_PARAM_DESC& TShader<Core>::GetSamplerParamDesc(const String& name) const
	{
		auto findIterObject = mDesc.samplerParams.find(name);
		if (findIterObject != mDesc.samplerParams.end())
			return findIterObject->second;

		BS_EXCEPT(InternalErrorException, "Cannot find the parameter with the name: " + name);
		static SHADER_OBJECT_PARAM_DESC dummy;
		return dummy;
	}

	template<bool Core>
	const SHADER_OBJECT_PARAM_DESC& TShader<Core>::GetBufferParamDesc(const String& name) const
	{
		auto findIterObject = mDesc.bufferParams.find(name);
		if (findIterObject != mDesc.bufferParams.end())
			return findIterObject->second;

		BS_EXCEPT(InternalErrorException, "Cannot find the parameter with the name: " + name);
		static SHADER_OBJECT_PARAM_DESC dummy;
		return dummy;
	}

	template<bool Core>
	bool TShader<Core>::HasDataParam(const String& name) const
	{
		auto findIterData = mDesc.dataParams.find(name);
		if (findIterData != mDesc.dataParams.end())
			return true;

		return false;
	}

	template<bool Core>
	bool TShader<Core>::HasTextureParam(const String& name) const
	{
		auto findIterObject = mDesc.textureParams.find(name);
		if (findIterObject != mDesc.textureParams.end())
			return true;

		return false;
	}

	template<bool Core>
	bool TShader<Core>::HasSamplerParam(const String& name) const
	{
		auto findIterObject = mDesc.samplerParams.find(name);
		if (findIterObject != mDesc.samplerParams.end())
			return true;

		return false;
	}

	template<bool Core>
	bool TShader<Core>::HasBufferParam(const String& name) const
	{
		auto findIterObject = mDesc.bufferParams.find(name);
		if (findIterObject != mDesc.bufferParams.end())
			return true;

		return false;
	}

	template<bool Core>
	bool TShader<Core>::HasParamBlock(const String& name) const
	{
		auto findIterObject = mDesc.paramBlocks.find(name);
		if (findIterObject != mDesc.paramBlocks.end())
			return true;

		return false;
	}

	template<bool Core>
	typename TShader<Core>::TextureType TShader<Core>::GetDefaultTexture(UINT32 index) const
	{
		if (index < (UINT32)mDesc.textureDefaultValues.size())
			return mDesc.textureDefaultValues[index];

		return TextureType();
	}

	template<bool Core>
	typename TShader<Core>::SamplerStateType TShader<Core>::GetDefaultSampler(UINT32 index) const
	{
		if (index < (UINT32)mDesc.samplerDefaultValues.size())
			return mDesc.samplerDefaultValues[index];

		return SamplerStateType();
	}

	template<bool Core>
	UINT8* TShader<Core>::GetDefaultValue(UINT32 index) const
	{
		if (index < (UINT32)mDesc.dataDefaultValues.size())
			return (UINT8*)&mDesc.dataDefaultValues[index];

		return nullptr;
	}

	template<bool Core>
	Vector<SPtr<typename TShader<Core>::TechniqueType>> TShader<Core>::GetCompatibleTechniques() const
	{
		Vector<SPtr<TechniqueType>> output;
		for (auto& technique : mDesc.techniques)
		{
			if (technique->IsSupported())
				output.push_back(technique);
		}

		return output;
	}

	template<bool Core>
	Vector<SPtr<typename TShader<Core>::TechniqueType>> TShader<Core>::GetCompatibleTechniques(
		const ShaderVariation& variation, bool exact) const
	{
		Vector<SPtr<TechniqueType>> output;
		for (auto& technique : mDesc.techniques)
		{
			if (technique->IsSupported() && technique->GetVariation().Matches(variation, exact))
				output.push_back(technique);
		}

		return output;
	}

	template class TShader < false > ;
	template class TShader < true >;

	Shader::Shader(const String& name, const SHADER_DESC& desc, UINT32 id)
		:TShader(name, desc, id)
	{
		mMetaData = bs_shared_ptr_new<ShaderMetaData>();
	}

	Shader::Shader(UINT32 id)
		:TShader(id)
	{ }

	SPtr<ct::Shader> Shader::GetCore() const
	{
		return std::static_pointer_cast<ct::Shader>(mCoreSpecific);
	}

	void Shader::SetIncludeFiles(const Vector<String>& includes)
	{
		SPtr<ShaderMetaData> meta = std::static_pointer_cast<ShaderMetaData>(GetMetaData());
		meta->includes = includes;
	}

	SPtr<ct::CoreObject> Shader::CreateCore() const
	{
		Vector<SPtr<ct::Technique>> techniques;
		for (auto& technique : mDesc.techniques)
			techniques.push_back(technique->GetCore());

		ct::Shader* shaderCore = new (bs_alloc<ct::Shader>()) ct::Shader(mName, ConvertDesc(mDesc), mId);
		SPtr<ct::Shader> shaderCorePtr = bs_shared_ptr<ct::Shader>(shaderCore);
		shaderCorePtr->SetThisPtrInternal(shaderCorePtr);

		return shaderCorePtr;
	}

	ct::SHADER_DESC Shader::ConvertDesc(const SHADER_DESC& desc) const
	{
		ct::SHADER_DESC output;
		output.dataParams = desc.dataParams;
		output.textureParams = desc.textureParams;
		output.samplerParams = desc.samplerParams;
		output.bufferParams = desc.bufferParams;
		output.paramBlocks = desc.paramBlocks;
		output.paramAttributes = desc.paramAttributes;

		output.dataDefaultValues = desc.dataDefaultValues;

		output.samplerDefaultValues.resize(desc.samplerDefaultValues.size());
		for (UINT32 i = 0; i < (UINT32)desc.samplerDefaultValues.size(); i++)
		{
			if (desc.samplerDefaultValues[i] != nullptr)
				output.samplerDefaultValues[i] = desc.samplerDefaultValues[i]->GetCore();
			else
				output.samplerDefaultValues[i] = nullptr;
		}

		output.textureDefaultValues.resize(desc.textureDefaultValues.size());
		for (UINT32 i = 0; i < (UINT32)desc.textureDefaultValues.size(); i++)
		{
			if (desc.textureDefaultValues[i].IsLoaded())
				output.textureDefaultValues[i] = desc.textureDefaultValues[i]->GetCore();
			else
				output.textureDefaultValues[i] = nullptr;
		}

		output.queuePriority = desc.queuePriority;
		output.queueSortType = desc.queueSortType;
		output.separablePasses = desc.separablePasses;
		output.flags = desc.flags;

		for(auto& entry : desc.techniques)
		{
			if(entry)
				output.techniques.push_back(entry->GetCore());
		}

		for(auto& entry : desc.subShaders)
		{
			ct::SubShader subShader;
			subShader.name = entry.name;
			
			if(entry.shader)
				subShader.shader = entry.shader->GetCore();

			output.subShaders.push_back(subShader);
		}

		output.variationParams = desc.variationParams;
		
		// Ignoring default values as they are not needed for syncing since
		// they're initialized through the material.
		return output;
	}

	void Shader::GetCoreDependencies(Vector<CoreObject*>& dependencies)
	{
		for (auto& technique : mDesc.techniques)
			dependencies.push_back(technique.get());
	}

	bool Shader::IsSampler(GpuParamObjectType type)
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

	bool Shader::IsTexture(GpuParamObjectType type)
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

	bool Shader::IsLoadStoreTexture(GpuParamObjectType type)
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

	bool Shader::IsBuffer(GpuParamObjectType type)
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

	UINT32 Shader::GetDataParamSize(GpuParamDataType type)
	{
		static const GpuDataParamInfos PARAM_SIZES;

		UINT32 idx = (UINT32)type;
		if (idx < sizeof(GpuParams::PARAM_SIZES.lookup))
			return GpuParams::PARAM_SIZES.lookup[idx].size;

		return 0;
	}

	HShader Shader::Create(const String& name, const SHADER_DESC& desc)
	{
		SPtr<Shader> newShader = CreatePtrInternal(name, desc);

		return static_resource_cast<Shader>(gResources().CreateResourceHandleInternal(newShader));
	}

	SPtr<Shader> Shader::CreatePtrInternal(const String& name, const SHADER_DESC& desc)
	{
		UINT32 id = ct::Shader::mNextShaderId.fetch_add(1, std::memory_order_relaxed);
		assert(id < std::numeric_limits<UINT32>::max() && "Created too many shaders, reached maximum id.");

		SPtr<Shader> newShader = bs_core_ptr<Shader>(new (bs_alloc<Shader>()) Shader(name, desc, id));
		newShader->SetThisPtrInternal(newShader);
		newShader->Initialize();

		return newShader;
	}

	SPtr<Shader> Shader::CreateEmpty()
	{
		UINT32 id = ct::Shader::mNextShaderId.fetch_add(1, std::memory_order_relaxed);
		assert(id < std::numeric_limits<UINT32>::max() && "Created too many shaders, reached maximum id.");

		SPtr<Shader> newShader = bs_core_ptr<Shader>(new (bs_alloc<Shader>()) Shader(id));
		newShader->SetThisPtrInternal(newShader);

		return newShader;
	}

	RTTITypeBase* Shader::GetRttiStatic()
	{
		return ShaderRTTI::Instance();
	}

	RTTITypeBase* Shader::GetRtti() const
	{
		return Shader::GetRttiStatic();
	}

	RTTITypeBase* ShaderMetaData::GetRttiStatic()
	{
		return ShaderMetaDataRTTI::Instance();
	}

	RTTITypeBase* ShaderMetaData::GetRtti() const
	{
		return ShaderMetaData::GetRttiStatic();
	}

	namespace ct
	{
	std::atomic<UINT32> Shader::mNextShaderId;

	Shader::Shader(const String& name, const SHADER_DESC& desc, UINT32 id)
		:TShader(name, desc, id)
	{

	}

	SPtr<Shader> Shader::Create(const String& name, const SHADER_DESC& desc)
	{
		UINT32 id = mNextShaderId.fetch_add(1, std::memory_order_relaxed);
		assert(id < std::numeric_limits<UINT32>::max() && "Created too many shaders, reached maximum id.");

		Shader* shaderCore = new (bs_alloc<Shader>()) Shader(name, desc, id);
		SPtr<Shader> shaderCorePtr = bs_shared_ptr<Shader>(shaderCore);
		shaderCorePtr->SetThisPtrInternal(shaderCorePtr);
		shaderCorePtr->Initialize();

		return shaderCorePtr;
	}
	}
}
