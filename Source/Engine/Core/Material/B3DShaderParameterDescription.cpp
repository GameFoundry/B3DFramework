//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Material/B3DShaderParameterDescription.h"
#include "Material/B3DShader.h"
#include "RTTI/B3DShaderRTTI.h"

using namespace b3d;

void ShaderParameterDescription::AddParameter(ShaderDataParameterInformation parameterInformation, TArrayView<const u8> defaultValue)
{
	if(parameterInformation.Type == GPDT_STRUCT && parameterInformation.ElementSize <= 0)
	{
		B3D_LOG(Error, LogMaterial, "You need to provide a non-zero element size for a struct parameter.");
		return;
	}

	const auto found = mDataParameters.find(parameterInformation.Name);
	if(found != mDataParameters.end())
		return;

	if(defaultValue.Size() != 0)
	{
		parameterInformation.DefaultValueIndex = (u32)mDataDefaultValues.size();
		const u32 elementSize = parameterInformation.Type == GPDT_STRUCT ? parameterInformation.ElementSize : Shader::GetDataParameterSize(parameterInformation.Type);
		const u32 defaultValueSize = elementSize * parameterInformation.ArraySize;
		if(!B3D_ENSURE(defaultValue.Size() == defaultValueSize))
			return;

		mDataDefaultValues.resize(parameterInformation.DefaultValueIndex + defaultValueSize);
		memcpy(&mDataDefaultValues[parameterInformation.DefaultValueIndex], defaultValue.Data(), defaultValueSize);
	}
	else
		parameterInformation.DefaultValueIndex = ~0u;

	mDataParameters[parameterInformation.Name] = parameterInformation;
}

void ShaderParameterDescription::AddParameter(ShaderObjectParameterInformation parameterInformation)
{
	AddParameterInternal(std::move(parameterInformation), ~0u);
}

void ShaderParameterDescription::AddParameter(ShaderObjectParameterInformation parameterInformation, const SamplerStateCreateInformation& defaultValue)
{
	u32 defaultValueIndex = ~0u;
	if(Shader::IsSampler(parameterInformation.Type) && mSamplerParameters.count(parameterInformation.Name) == 0)
	{
		defaultValueIndex = (u32)mSamplerDefaultValues.size();
		mSamplerDefaultValues.push_back(defaultValue);
	}

	AddParameterInternal(std::move(parameterInformation), defaultValueIndex);
}

void ShaderParameterDescription::AddParameter(ShaderObjectParameterInformation parameterInformation, ShaderDefaultTextureType defaultValue)
{
	u32 defaultValueIndex = ~0u;
	if(Shader::IsTexture(parameterInformation.Type) && mTextureParameters.count(parameterInformation.Name) == 0)
	{
		defaultValueIndex = (u32)mTextureDefaultValues.size();
		mTextureDefaultValues.push_back(defaultValue);
	}

	AddParameterInternal(std::move(parameterInformation), defaultValueIndex);
}

void ShaderParameterDescription::AddParameterInternal(ShaderObjectParameterInformation parameterInformation, u32 defaultValueIndex)
{
	Map<String, ShaderObjectParameterInformation>* destinationLookup[] = { &mTextureParameters, &mBufferParameters, &mSamplerParameters };
	u32 destinationIndex = 0;
	if(Shader::IsBuffer(parameterInformation.Type))
		destinationIndex = 1;
	else if(Shader::IsSampler(parameterInformation.Type))
		destinationIndex = 2;

	Map<String, ShaderObjectParameterInformation>& parameterMap = *destinationLookup[destinationIndex];

	auto found = parameterMap.find(parameterInformation.Name);
	if(found == parameterMap.end())
	{
		parameterInformation.DefaultValueIndex = defaultValueIndex;
		parameterMap[parameterInformation.Name] = parameterInformation;
	}
	else
	{
		ShaderObjectParameterInformation& existingParameterInformation = found->second;

		// If same name but different properties, we ignore this param
		if(existingParameterInformation.Type != parameterInformation.Type || existingParameterInformation.RendererSemantic != parameterInformation.RendererSemantic)
			return;

		Vector<String>& gpuVariableNames = existingParameterInformation.GpuVariableNames;
		bool found = false;
		for(u32 i = 0; i < (u32)gpuVariableNames.size(); i++)
		{
			if(gpuVariableNames[i] == parameterInformation.GpuVariableName)
			{
				found = true;
				break;
			}
		}

		if(!found)
			gpuVariableNames.push_back(parameterInformation.GpuVariableName);
	}
}

void ShaderParameterDescription::SetParameterAttribute(const String& name, const ShaderParameterAttribute& attribute)
{
	ShaderDataParameterInformation* dataParameterInformation = nullptr;

	const auto foundDataParameter = mDataParameters.find(name);
	if(foundDataParameter != mDataParameters.end())
		dataParameterInformation = &foundDataParameter->second;

	ShaderObjectParameterInformation* objectParameterInformation = nullptr;
	if(!dataParameterInformation)
	{
		const auto foundTextureParameter = mTextureParameters.find(name);
		if(foundTextureParameter != mTextureParameters.end())
			objectParameterInformation = &foundTextureParameter->second;

		if(!objectParameterInformation)
		{
			const auto foundSamplerParameter = mSamplerParameters.find(name);
			if(foundSamplerParameter != mSamplerParameters.end())
				objectParameterInformation = &foundSamplerParameter->second;
		}

		if(!objectParameterInformation)
		{
			const auto foundBufferParameter = mBufferParameters.find(name);
			if(foundBufferParameter != mBufferParameters.end())
				objectParameterInformation = &foundBufferParameter->second;
		}
	}

	ShaderParameterInformation* parameterInformation = dataParameterInformation;
	if(!parameterInformation)
		parameterInformation = objectParameterInformation;

	if(!parameterInformation)
	{
		B3D_LOG(Warning, LogMaterial, "Attempting to apply a shader parameter attribute to a non-existing parameter.");
		return;
	}

	if(attribute.Type == ShaderParamAttributeType::SpriteUV)
	{
		if(objectParameterInformation)
		{
			B3D_LOG(Warning, LogMaterial, "Attempting to apply SpriteUV attribute to an object parameter is not supported.");
			return;
		}

		if(dataParameterInformation->Type != GPDT_FLOAT4)
		{
			B3D_LOG(Warning, LogMaterial, "SpriteUV attribute can only be applied to 4D vectors.");
			return;
		}
	}

	// Look for duplicate attributes
	u32 currentAttributeIndex = parameterInformation->AttributeIndex;
	bool found = false;
	while(currentAttributeIndex != ~0u)
	{
		ShaderParameterAttribute& currentAttribute = mParameterAttributes[currentAttributeIndex];
		if(currentAttribute.Type == attribute.Type)
		{
			const u32 nextParameterIndex = currentAttribute.NextParameterIndex;
			currentAttribute = attribute;
			currentAttribute.NextParameterIndex = nextParameterIndex;

			found = true;
			break;
		}

		currentAttributeIndex = currentAttribute.NextParameterIndex;
	}

	if(!found)
	{
		const auto attributeIndex = (u32)mParameterAttributes.size();
		mParameterAttributes.emplace_back(attribute);

		mParameterAttributes.back().NextParameterIndex = parameterInformation->AttributeIndex;

		parameterInformation->AttributeIndex = attributeIndex;
	}
}

void ShaderParameterDescription::SetUniformBufferAttributes(const String& name, bool shared, GpuBufferFlags flags, StringID rendererSemantic)
{
	ShaderUniformBufferInformation& information = mUniformBuffers[name];
	information.Name = name;
	information.Shared = shared;
	information.Flags = flags;
	information.RendererSemantic = rendererSemantic;
}

void ShaderParameterDescription::AddUniformBuffer(ShaderUniformBufferInformation information)
{
	mUniformBuffers[information.Name] = std::move(information);
}

void ShaderParameterDescription::SetParameterInternal(const String& name, bool isInternal)
{
	const auto fnSetInternal = [&name, isInternal](auto& parameters)
	{
		const auto found = parameters.find(name);
		if(found != parameters.end())
			found->second.IsInternal = isInternal;
	};

	fnSetInternal(mDataParameters);
	fnSetInternal(mTextureParameters);
	fnSetInternal(mBufferParameters);
	fnSetInternal(mSamplerParameters);
	fnSetInternal(mUniformBuffers);
}

void ShaderParameterDescription::SetParameterBinding(const String& name, u32 set, u32 slot)
{
	const auto fnSetBinding = [&name, set, slot](auto& parameters)
	{
		const auto found = parameters.find(name);
		if(found != parameters.end())
		{
			found->second.Set = set;
			found->second.Slot = slot;
		}
	};

	fnSetBinding(mDataParameters);
	fnSetBinding(mTextureParameters);
	fnSetBinding(mBufferParameters);
	fnSetBinding(mSamplerParameters);
	fnSetBinding(mUniformBuffers);
}

RTTIType* ShaderParameterDescription::GetRttiStatic()
{
	return ShaderParameterDescriptionRTTI::Instance();
}

Result ShaderParameterDescription::ValidateMaterialInterface(TArrayView<const ShaderParameterDescription*> passParameters) const
{
	const auto fnMatchesName = [](const auto& materialParameter, const auto& sourceParameter)
	{
		using ParameterType = std::decay_t<decltype(materialParameter)>;
		if constexpr(std::is_same_v<ParameterType, ShaderUniformBufferInformation>)
			return materialParameter.Name == sourceParameter.Name;
		else if constexpr(std::is_same_v<ParameterType, ShaderObjectParameterInformation>)
			return std::find(materialParameter.GpuVariableNames.begin(), materialParameter.GpuVariableNames.end(), sourceParameter.GpuVariableName) != materialParameter.GpuVariableNames.end();
		else
			return materialParameter.GpuVariableName == sourceParameter.GpuVariableName;
	};

	const auto fnValidateParameters = [this, passParameters, &fnMatchesName](auto getParameters)
	{
		const auto& materialParameters = (this->*getParameters)();
		for(const ShaderParameterDescription* pass : passParameters)
		{
			for(const auto& entry : (pass->*getParameters)())
			{
				const auto& sourceParameter = entry.second;
				if(sourceParameter.IsInternal)
					continue;

				const auto found = std::find_if(materialParameters.begin(), materialParameters.end(), [&sourceParameter, &fnMatchesName](const auto& candidate) { return !candidate.second.IsInternal && fnMatchesName(candidate.second, sourceParameter); });
				if(found == materialParameters.end())
					return Result::Fail("Shader variation adds a parameter to the material interface.", ResultStatus::FailedInvalidInput, entry.first);

				using ParameterType = std::decay_t<decltype(sourceParameter)>;
				if constexpr(!std::is_same_v<ParameterType, ShaderUniformBufferInformation>)
				{
					if(found->second.Type != sourceParameter.Type || found->second.ArraySize != sourceParameter.ArraySize)
						return Result::Fail("Shader variation changes a material parameter's type or array size.", ResultStatus::FailedInvalidInput, found->first);

					if constexpr(std::is_same_v<ParameterType, ShaderDataParameterInformation>)
					{
						if(found->second.ElementSize != sourceParameter.ElementSize)
							return Result::Fail("Shader variation changes a material parameter's element size.", ResultStatus::FailedInvalidInput, found->first);
					}
				}
			}
		}

		for(const auto& entry : materialParameters)
		{
			if(entry.second.IsInternal)
				continue;

			bool found = false;
			for(const ShaderParameterDescription* pass : passParameters)
			{
				const auto& sourceParameters = (pass->*getParameters)();
				found = std::any_of(sourceParameters.begin(), sourceParameters.end(), [&entry, &fnMatchesName](const auto& candidate) { return !candidate.second.IsInternal && fnMatchesName(entry.second, candidate.second); });
				if(found)
					break;
			}

			if(!found)
				return Result::Fail("Shader variation removes a parameter from the material interface.", ResultStatus::FailedInvalidInput, entry.first);
		}

		return Result::Success();
	};

	for(const Result& result : {
		fnValidateParameters(&ShaderParameterDescription::GetDataParameters),
		fnValidateParameters(&ShaderParameterDescription::GetTextureParameters),
		fnValidateParameters(&ShaderParameterDescription::GetBufferParameters),
		fnValidateParameters(&ShaderParameterDescription::GetSamplerParameters),
		fnValidateParameters(&ShaderParameterDescription::GetUniformBuffers) })
	{
		if(!result.IsSuccessful())
			return result;
	}

	return Result::Success();
}

Result ShaderParameterDescription::TryCombine(const ShaderParameterDescription& other)
{
	if(this == &other)
		return Result::Success();

	const auto fnValidateParameters = [this, &other](const auto& destination, const auto& source, const auto& destinationDefaults, const auto& sourceDefaults)
	{
		for(const auto& entry : source)
		{
			const auto found = destination.find(entry.first);
			if(found == destination.end())
			{
				if(mDataParameters.count(entry.first) || mTextureParameters.count(entry.first) || mBufferParameters.count(entry.first) || mSamplerParameters.count(entry.first))
					return Result::Fail("Shader parameter categories conflict.", ResultStatus::FailedInvalidInput, entry.first);

				continue;
			}

			const auto& existing = found->second;
			const auto& incoming = entry.second;
			if(existing.Type != incoming.Type || existing.ArraySize != incoming.ArraySize || existing.RendererSemantic != incoming.RendererSemantic)
				return Result::Fail("Shader parameter declarations conflict.", ResultStatus::FailedInvalidInput, entry.first);

			if constexpr(std::is_same_v<std::decay_t<decltype(existing)>, ShaderDataParameterInformation>)
			{
				if(existing.ElementSize != incoming.ElementSize || existing.GpuVariableName != incoming.GpuVariableName)
					return Result::Fail("Shader data parameter declarations conflict.", ResultStatus::FailedInvalidInput, entry.first);
			}

			if(existing.DefaultValueIndex != ~0u && incoming.DefaultValueIndex != ~0u)
			{
				if constexpr(std::is_same_v<std::decay_t<decltype(existing)>, ShaderDataParameterInformation>)
				{
					const u32 elementSize = incoming.Type == GPDT_STRUCT ? incoming.ElementSize : Shader::GetDataParameterSize(incoming.Type);
					if(memcmp(destinationDefaults.data() + existing.DefaultValueIndex, sourceDefaults.data() + incoming.DefaultValueIndex, elementSize * incoming.ArraySize) != 0)
						return Result::Fail("Shader parameter defaults conflict.", ResultStatus::FailedInvalidInput, entry.first);
				}
				else if(destinationDefaults[existing.DefaultValueIndex] != sourceDefaults[incoming.DefaultValueIndex])
					return Result::Fail("Shader parameter defaults conflict.", ResultStatus::FailedInvalidInput, entry.first);
			}

			for(u32 sourceIndex = incoming.AttributeIndex; sourceIndex != ~0u; sourceIndex = other.mParameterAttributes[sourceIndex].NextParameterIndex)
			{
				const ShaderParameterAttribute& sourceAttribute = other.mParameterAttributes[sourceIndex];
				for(u32 index = existing.AttributeIndex; index != ~0u; index = mParameterAttributes[index].NextParameterIndex)
				{
					const ShaderParameterAttribute& attribute = mParameterAttributes[index];
					if(attribute.Type == sourceAttribute.Type && attribute.Value != sourceAttribute.Value)
						return Result::Fail("Shader parameter attributes conflict.", ResultStatus::FailedInvalidInput, entry.first);
				}
			}
		}

		return Result::Success();
	};

	for(const Result& result : {
		fnValidateParameters(mDataParameters, other.mDataParameters, mDataDefaultValues, other.mDataDefaultValues),
		fnValidateParameters(mTextureParameters, other.mTextureParameters, mTextureDefaultValues, other.mTextureDefaultValues),
		fnValidateParameters(mBufferParameters, other.mBufferParameters, mTextureDefaultValues, other.mTextureDefaultValues),
		fnValidateParameters(mSamplerParameters, other.mSamplerParameters, mSamplerDefaultValues, other.mSamplerDefaultValues) })
	{
		if(!result.IsSuccessful())
			return result;
	}

	for(const auto& entry : other.mUniformBuffers)
	{
		const auto found = mUniformBuffers.find(entry.first);
		if(found != mUniformBuffers.end() && (found->second.Shared != entry.second.Shared || found->second.Flags != entry.second.Flags || found->second.RendererSemantic != entry.second.RendererSemantic))
			return Result::Fail("Shader uniform-buffer declarations conflict.", ResultStatus::FailedInvalidInput, entry.first);
	}

	const auto fnCombineParameters = [this, &other](auto& destination, const auto& source, auto& destinationDefaults, const auto& sourceDefaults)
	{
		for(const auto& entry : source)
		{
			const auto inserted = destination.try_emplace(entry.first, entry.second);
			auto& parameter = inserted.first->second;
			if(inserted.second)
			{
				parameter.DefaultValueIndex = ~0u;
				parameter.AttributeIndex = ~0u;
			}

			if(parameter.DefaultValueIndex == ~0u && entry.second.DefaultValueIndex != ~0u)
			{
				parameter.DefaultValueIndex = (u32)destinationDefaults.size();

				if constexpr(std::is_same_v<std::decay_t<decltype(parameter)>, ShaderDataParameterInformation>)
				{
					const u32 elementSize = parameter.Type == GPDT_STRUCT ? parameter.ElementSize : Shader::GetDataParameterSize(parameter.Type);
					const auto begin = sourceDefaults.begin() + entry.second.DefaultValueIndex;
					destinationDefaults.insert(destinationDefaults.end(), begin, begin + elementSize * parameter.ArraySize);
				}
				else
					destinationDefaults.push_back(sourceDefaults[entry.second.DefaultValueIndex]);
			}

			if constexpr(std::is_same_v<std::decay_t<decltype(parameter)>, ShaderObjectParameterInformation>)
			{
				for(const String& alias : entry.second.GpuVariableNames)
				{
					if(std::find(parameter.GpuVariableNames.begin(), parameter.GpuVariableNames.end(), alias) == parameter.GpuVariableNames.end())
						parameter.GpuVariableNames.push_back(alias);
				}
			}

			for(u32 index = entry.second.AttributeIndex; index != ~0u; index = other.mParameterAttributes[index].NextParameterIndex)
			{
				ShaderParameterAttribute attribute = other.mParameterAttributes[index];
				attribute.NextParameterIndex = ~0u;

				SetParameterAttribute(entry.first, attribute);
			}
		}
	};

	fnCombineParameters(mDataParameters, other.mDataParameters, mDataDefaultValues, other.mDataDefaultValues);
	fnCombineParameters(mTextureParameters, other.mTextureParameters, mTextureDefaultValues, other.mTextureDefaultValues);
	fnCombineParameters(mBufferParameters, other.mBufferParameters, mTextureDefaultValues, other.mTextureDefaultValues);
	fnCombineParameters(mSamplerParameters, other.mSamplerParameters, mSamplerDefaultValues, other.mSamplerDefaultValues);

	mUniformBuffers.insert(other.mUniformBuffers.begin(), other.mUniformBuffers.end());
	return Result::Success();
}

RTTIType* ShaderParameterDescription::GetRtti() const
{
	return GetRttiStatic();
}
