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

using namespace bs;

template <bool Core>
TShaderCreateInformation<Core>::TShaderCreateInformation()
	: QueueSortType(QueueSortType::None), QueuePriority(0), SeparablePasses(false), Flags(0)
{
}

template<bool Core>
TShaderCreateInformation<true> TShaderCreateInformation<Core>::ConvertToCore(const TShaderCreateInformation<false>& value)
{
	ct::ShaderCreateInformation output;
	output.DataParams = value.DataParams;
	output.TextureParams = value.TextureParams;
	output.SamplerParams = value.SamplerParams;
	output.BufferParams = value.BufferParams;
	output.ParamBlocks = value.ParamBlocks;
	output.ParamAttributes = value.ParamAttributes;

	output.DataDefaultValues = value.DataDefaultValues;

	output.SamplerDefaultValues.resize(value.SamplerDefaultValues.size());
	for(u32 i = 0; i < (u32)value.SamplerDefaultValues.size(); i++)
	{
		if(value.SamplerDefaultValues[i] != nullptr)
			output.SamplerDefaultValues[i] = value.SamplerDefaultValues[i]->GetCore();
		else
			output.SamplerDefaultValues[i] = nullptr;
	}

	output.TextureDefaultValues.resize(value.TextureDefaultValues.size());
	for(u32 i = 0; i < (u32)value.TextureDefaultValues.size(); i++)
	{
		if(value.TextureDefaultValues[i].IsLoaded())
			output.TextureDefaultValues[i] = value.TextureDefaultValues[i]->GetCore();
		else
			output.TextureDefaultValues[i] = nullptr;
	}

	output.QueuePriority = value.QueuePriority;
	output.QueueSortType = value.QueueSortType;
	output.SeparablePasses = value.SeparablePasses;
	output.Flags = value.Flags;

	for(auto& entry : value.Techniques)
	{
		if(entry != nullptr)
			output.Techniques.push_back(entry->GetCore());
	}

	output.VariationParams = value.VariationParams;
	output.CompilerMetaData = value.CompilerMetaData;

	// Ignoring default values as they are not needed for syncing since
	// they're initialized through the material.
	return output;
}

template <bool Core>
void TShaderCreateInformation<Core>::AddParameter(ShaderDataParameterInformation paramDesc, u8* defaultValue)
{
	if(paramDesc.Type == GPDT_STRUCT && paramDesc.ElementSize <= 0)
	{
		B3D_LOG(Error, Material, "You need to provide a non-zero element size for a struct parameter.");
		return;
	}

	const auto iterFind = DataParams.find(paramDesc.Name);
	if(iterFind != DataParams.end())
		return;

	if(defaultValue != nullptr)
	{
		paramDesc.DefaultValueIndex = (u32)DataDefaultValues.size();
		u32 defaultValueSize = Shader::GetDataParamSize(paramDesc.Type);

		DataDefaultValues.resize(paramDesc.DefaultValueIndex + defaultValueSize);
		memcpy(&DataDefaultValues[paramDesc.DefaultValueIndex], defaultValue, defaultValueSize);
	}
	else
		paramDesc.DefaultValueIndex = (u32)-1;

	DataParams[paramDesc.Name] = paramDesc;
}

template <bool Core>
void TShaderCreateInformation<Core>::AddParameter(ShaderObjectParameterInformation paramDesc)
{
	u32 defaultValueIdx = (u32)-1;

	AddParameterInternal(std::move(paramDesc), defaultValueIdx);
}

template <bool Core>
void TShaderCreateInformation<Core>::AddParameter(ShaderObjectParameterInformation paramDesc, const SamplerStateType& defaultValue)
{
	u32 defaultValueIdx = (u32)-1;
	if(Shader::IsSampler(paramDesc.Type) && defaultValue != nullptr)
	{
		defaultValueIdx = (u32)SamplerDefaultValues.size();
		SamplerDefaultValues.push_back(defaultValue);
	}

	AddParameterInternal(std::move(paramDesc), defaultValueIdx);
}

template <bool Core>
void TShaderCreateInformation<Core>::AddParameter(ShaderObjectParameterInformation paramDesc, const TextureType& defaultValue)
{
	u32 defaultValueIdx = (u32)-1;
	if(Shader::IsTexture(paramDesc.Type) && defaultValue != nullptr)
	{
		defaultValueIdx = (u32)TextureDefaultValues.size();
		TextureDefaultValues.push_back(defaultValue);
	}

	AddParameterInternal(std::move(paramDesc), defaultValueIdx);
}

template <bool Core>
void TShaderCreateInformation<Core>::AddParameterInternal(ShaderObjectParameterInformation paramDesc, u32 defaultValueIdx)
{
	Map<String, ShaderObjectParameterInformation>* DEST_LOOKUP[] = { &TextureParams, &BufferParams, &SamplerParams };
	u32 destIdx = 0;
	if(Shader::IsBuffer(paramDesc.Type))
		destIdx = 1;
	else if(Shader::IsSampler(paramDesc.Type))
		destIdx = 2;

	Map<String, ShaderObjectParameterInformation>& paramsMap = *DEST_LOOKUP[destIdx];

	auto iterFind = paramsMap.find(paramDesc.Name);
	if(iterFind == paramsMap.end())
	{
		paramDesc.DefaultValueIndex = defaultValueIdx;
		paramsMap[paramDesc.Name] = paramDesc;
	}
	else
	{
		ShaderObjectParameterInformation& desc = iterFind->second;

		// If same name but different properties, we ignore this param
		if(desc.Type != paramDesc.Type || desc.RendererSemantic != paramDesc.RendererSemantic)
			return;

		Vector<String>& gpuVariableNames = desc.GpuVariableNames;
		bool found = false;
		for(u32 i = 0; i < (u32)gpuVariableNames.size(); i++)
		{
			if(gpuVariableNames[i] == paramDesc.GpuVariableName)
			{
				found = true;
				break;
			}
		}

		if(!found)
			gpuVariableNames.push_back(paramDesc.GpuVariableName);
	}
}

template <bool Core>
void TShaderCreateInformation<Core>::SetParameterAttribute(const String& name, const ShaderParameterAttribute& attrib)
{
	ShaderDataParameterInformation* paramDescData = nullptr;

	const auto findIterData = DataParams.find(name);
	if(findIterData != DataParams.end())
		paramDescData = &findIterData->second;

	ShaderObjectParameterInformation* paramDescObj = nullptr;
	if(!paramDescData)
	{
		const auto findIterTexture = TextureParams.find(name);
		if(findIterTexture != TextureParams.end())
			paramDescObj = &findIterTexture->second;

		if(!paramDescObj)
		{
			const auto findIterSampler = SamplerParams.find(name);
			if(findIterSampler != SamplerParams.end())
				paramDescObj = &findIterSampler->second;
		}

		if(!paramDescObj)
		{
			const auto findIterBuffer = BufferParams.find(name);
			if(findIterBuffer != BufferParams.end())
				paramDescObj = &findIterBuffer->second;
		}
	}

	ShaderParameterInformation* paramDesc = paramDescData;
	if(!paramDesc)
		paramDesc = paramDescObj;

	if(!paramDesc)
	{
		B3D_LOG(Warning, Material, "Attempting to apply a shader parameter attribute to a non-existing parameter.");
		return;
	}

	if(attrib.Type == ShaderParamAttributeType::SpriteUV)
	{
		if(paramDescObj)
		{
			B3D_LOG(Warning, Material, "Attempting to apply SpriteUV attribute to an object parameter is not supported.");
			return;
		}

		if(paramDescData->Type != GPDT_FLOAT4)
		{
			B3D_LOG(Warning, Material, "SpriteUV attribute can only be applied to 4D vectors.");
			return;
		}
	}

	// Look for duplicate attributes
	u32 curAttribIdx = paramDesc->AttributeIndex;
	bool found = false;
	while(curAttribIdx != (u32)-1)
	{
		ShaderParameterAttribute& curAttrib = ParamAttributes[curAttribIdx];
		if(curAttrib.Type == attrib.Type)
		{
			curAttrib = attrib;

			found = true;
			break;
		}

		curAttribIdx = curAttrib.NextParameterIndex;
	}

	if(!found)
	{
		const auto attribIdx = (u32)ParamAttributes.size();
		ParamAttributes.emplace_back(attrib);

		if(paramDesc->AttributeIndex != (u32)-1)
			ParamAttributes.back().NextParameterIndex = paramDesc->AttributeIndex;

		paramDesc->AttributeIndex = attribIdx;
	}
}

template <bool Core>
void TShaderCreateInformation<Core>::SetParamBlockAttribs(const String& name, bool shared, GpuBufferUsage usage, StringID rendererSemantic)
{
	ShaderParameterBlockInformation desc;
	desc.Name = name;
	desc.Shared = shared;
	desc.Usage = usage;
	desc.RendererSemantic = rendererSemantic;

	ParamBlocks[name] = desc;
}

template struct TShaderCreateInformation<false>;
template struct TShaderCreateInformation<true>;

template <bool Core>
TShader<Core>::TShader(u32 id)
	: mId(id)
{}

template <bool Core>
TShader<Core>::TShader(const String& name, const TShaderCreateInformation<Core>& desc, u32 id)
	: mName(name), mDesc(desc), mId(id)
{}

template <bool Core>
TShader<Core>::~TShader()
{}

template <bool Core>
GpuParameterType TShader<Core>::GetParamType(const String& name) const
{
	auto findIterData = mDesc.DataParams.find(name);
	if(findIterData != mDesc.DataParams.end())
		return GPT_DATA;

	auto findIterTexture = mDesc.TextureParams.find(name);
	if(findIterTexture != mDesc.TextureParams.end())
		return GPT_TEXTURE;

	auto findIterBuffer = mDesc.BufferParams.find(name);
	if(findIterBuffer != mDesc.BufferParams.end())
		return GPT_BUFFER;

	auto findIterSampler = mDesc.SamplerParams.find(name);
	if(findIterSampler != mDesc.SamplerParams.end())
		return GPT_SAMPLER;

	B3D_EXCEPT(InternalErrorException, "Cannot find the parameter with the name: " + name);
	return GPT_DATA;
}

template <bool Core>
const ShaderDataParameterInformation& TShader<Core>::GetDataParamDesc(const String& name) const
{
	auto findIterData = mDesc.DataParams.find(name);
	if(findIterData != mDesc.DataParams.end())
		return findIterData->second;

	B3D_EXCEPT(InternalErrorException, "Cannot find the parameter with the name: " + name);
	static ShaderDataParameterInformation dummy;
	return dummy;
}

template <bool Core>
const ShaderObjectParameterInformation& TShader<Core>::GetTextureParamDesc(const String& name) const
{
	auto findIterObject = mDesc.TextureParams.find(name);
	if(findIterObject != mDesc.TextureParams.end())
		return findIterObject->second;

	B3D_EXCEPT(InternalErrorException, "Cannot find the parameter with the name: " + name);
	static ShaderObjectParameterInformation dummy;
	return dummy;
}

template <bool Core>
const ShaderObjectParameterInformation& TShader<Core>::GetSamplerParamDesc(const String& name) const
{
	auto findIterObject = mDesc.SamplerParams.find(name);
	if(findIterObject != mDesc.SamplerParams.end())
		return findIterObject->second;

	B3D_EXCEPT(InternalErrorException, "Cannot find the parameter with the name: " + name);
	static ShaderObjectParameterInformation dummy;
	return dummy;
}

template <bool Core>
const ShaderObjectParameterInformation& TShader<Core>::GetBufferParamDesc(const String& name) const
{
	auto findIterObject = mDesc.BufferParams.find(name);
	if(findIterObject != mDesc.BufferParams.end())
		return findIterObject->second;

	B3D_EXCEPT(InternalErrorException, "Cannot find the parameter with the name: " + name);
	static ShaderObjectParameterInformation dummy;
	return dummy;
}

template <bool Core>
bool TShader<Core>::HasDataParam(const String& name) const
{
	auto findIterData = mDesc.DataParams.find(name);
	if(findIterData != mDesc.DataParams.end())
		return true;

	return false;
}

template <bool Core>
bool TShader<Core>::HasTextureParam(const String& name) const
{
	auto findIterObject = mDesc.TextureParams.find(name);
	if(findIterObject != mDesc.TextureParams.end())
		return true;

	return false;
}

template <bool Core>
bool TShader<Core>::HasSamplerParam(const String& name) const
{
	auto findIterObject = mDesc.SamplerParams.find(name);
	if(findIterObject != mDesc.SamplerParams.end())
		return true;

	return false;
}

template <bool Core>
bool TShader<Core>::HasBufferParam(const String& name) const
{
	auto findIterObject = mDesc.BufferParams.find(name);
	if(findIterObject != mDesc.BufferParams.end())
		return true;

	return false;
}

template <bool Core>
bool TShader<Core>::HasParamBlock(const String& name) const
{
	auto findIterObject = mDesc.ParamBlocks.find(name);
	if(findIterObject != mDesc.ParamBlocks.end())
		return true;

	return false;
}

template <bool Core>
typename TShader<Core>::TextureType TShader<Core>::GetDefaultTexture(u32 index) const
{
	if(index < (u32)mDesc.TextureDefaultValues.size())
		return mDesc.TextureDefaultValues[index];

	return TextureType();
}

template <bool Core>
typename TShader<Core>::SamplerStateType TShader<Core>::GetDefaultSampler(u32 index) const
{
	if(index < (u32)mDesc.SamplerDefaultValues.size())
		return mDesc.SamplerDefaultValues[index];

	return SamplerStateType();
}

template <bool Core>
u8* TShader<Core>::GetDefaultValue(u32 index) const
{
	if(index < (u32)mDesc.DataDefaultValues.size())
		return (u8*)&mDesc.DataDefaultValues[index];

	return nullptr;
}

template <bool Core>
Vector<SPtr<typename TShader<Core>::TechniqueType>> TShader<Core>::GetCompatibleTechniques() const
{
	Vector<SPtr<TechniqueType>> output;
	for(auto& technique : mDesc.Techniques)
	{
		if(technique->IsSupported())
			output.push_back(technique);
	}

	return output;
}

template <bool Core>
Vector<SPtr<typename TShader<Core>::TechniqueType>> TShader<Core>::GetCompatibleTechniques(
	const ShaderVariationParameters& variation, bool exact) const
{
	Vector<SPtr<TechniqueType>> output;
	for(auto& technique : mDesc.Techniques)
	{
		if(technique->IsSupported() && technique->GetVariationParameters().Matches(variation, exact))
			output.push_back(technique);
	}

	return output;
}

template class TShader<false>;
template class TShader<true>;

Shader::Shader(const String& name, const ShaderCreateInformation& createInformation, u32 id)
	: TShader(name, createInformation, id)
{
	mMetaData = B3DMakeShared<ShaderMetaData>();
}

Shader::Shader(u32 id)
	: TShader(id)
{}

SPtr<ct::Shader> Shader::GetCore() const
{
	return std::static_pointer_cast<ct::Shader>(mCoreSpecific);
}

void Shader::SetIncludeFiles(const Vector<String>& includes)
{
	SPtr<ShaderMetaData> meta = std::static_pointer_cast<ShaderMetaData>(GetMetaData());
	meta->Includes = includes;
}

SPtr<ct::CoreObject> Shader::CreateCore() const
{
	Vector<SPtr<ct::Technique>> techniques;
	for(auto& technique : mDesc.Techniques)
		techniques.push_back(technique->GetCore());

	ct::Shader* shaderCore = new(B3DAllocate<ct::Shader>()) ct::Shader(mName, TShaderCreateInformation<true>::ConvertToCore(mDesc), mId);
	SPtr<ct::Shader> shaderCorePtr = B3DMakeSharedFromExisting<ct::Shader>(shaderCore);
	shaderCorePtr->SetShared(shaderCorePtr);

	return shaderCorePtr;
}

void Shader::GetCoreDependencies(Vector<CoreObject*>& dependencies)
{
	for(auto& technique : mDesc.Techniques)
		dependencies.push_back(technique.get());
}

bool Shader::IsSampler(GpuParameterObjectType type)
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

bool Shader::IsTexture(GpuParameterObjectType type)
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

bool Shader::IsLoadStoreTexture(GpuParameterObjectType type)
{
	switch(type)
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

bool Shader::IsBuffer(GpuParameterObjectType type)
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

u32 Shader::GetDataParamSize(GpuDataParameterType type)
{
	static const GpuDataParameterTypeInformationLookup kParamSizes;

	u32 idx = (u32)type;
	if(idx < sizeof(GpuParams::kParamSizes.Lookup))
		return GpuParams::kParamSizes.Lookup[idx].Size;

	return 0;
}

HShader Shader::Create(const String& name, const ShaderCreateInformation& createInformation)
{
	SPtr<Shader> newShader = CreateShared(name, createInformation);

	return B3DStaticResourceCast<Shader>(GetResources().CreateResourceHandleInternal(newShader));
}

SPtr<Shader> Shader::CreateShared(const String& name, const ShaderCreateInformation& createInformation)
{
	u32 id = ct::Shader::mNextShaderId.fetch_add(1, std::memory_order_relaxed);
	B3D_ASSERT(id < std::numeric_limits<u32>::max() && "Created too many shaders, reached maximum id.");

	SPtr<Shader> newShader = B3DMakeCoreFromExisting<Shader>(new(B3DAllocate<Shader>()) Shader(name, createInformation, id));
	newShader->SetThisPtrInternal(newShader);
	newShader->Initialize();

	return newShader;
}

SPtr<Shader> Shader::CreateEmpty()
{
	u32 id = ct::Shader::mNextShaderId.fetch_add(1, std::memory_order_relaxed);
	B3D_ASSERT(id < std::numeric_limits<u32>::max() && "Created too many shaders, reached maximum id.");

	SPtr<Shader> newShader = B3DMakeCoreFromExisting<Shader>(new(B3DAllocate<Shader>()) Shader(id));
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

namespace bs { namespace ct
{
std::atomic<u32> Shader::mNextShaderId;

Shader::Shader(const String& name, const ShaderCreateInformation& createInformation, u32 id)
	: TShader(name, createInformation, id)
{
}

SPtr<Shader> Shader::Create(const String& name, const ShaderCreateInformation& createInformation)
{
	u32 id = mNextShaderId.fetch_add(1, std::memory_order_relaxed);
	B3D_ASSERT(id < std::numeric_limits<u32>::max() && "Created too many shaders, reached maximum id.");

	Shader* shaderCore = new(B3DAllocate<Shader>()) Shader(name, createInformation, id);
	SPtr<Shader> shaderCorePtr = B3DMakeSharedFromExisting<Shader>(shaderCore);
	shaderCorePtr->SetShared(shaderCorePtr);
	shaderCorePtr->Initialize();

	return shaderCorePtr;
}
}}
