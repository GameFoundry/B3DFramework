//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Material/BsShader.h"

#include "BsShaderManager.h"
#include "Material/BsTechnique.h"
#include "Error/BsException.h"
#include "Debug/BsDebug.h"
#include "Private/RTTI/BsShaderRTTI.h"
#include "Resources/BsResources.h"
#include "RenderAPI/BsGpuParameters.h"
#include "Material/BsPass.h"
#include "RenderAPI/BsSamplerState.h"
#include "Image/BsTexture.h"
#include "RenderAPI/BsGpuDevice.h"
#include "Resources/BsBuiltinResources.h"
#include "ThirdParty/CityHash/city.h"

using namespace bs;

ShaderInformationBase::ShaderInformationBase()
	: QueueSortType(QueueSortType::None), QueuePriority(0), SeparablePasses(false), Flags(0)
{
}

void ShaderInformationBase::AddParameter(ShaderDataParameterInformation paramDesc, u8* defaultValue)
{
	if(paramDesc.Type == GPDT_STRUCT && paramDesc.ElementSize <= 0)
	{
		B3D_LOG(Error, Material, "You need to provide a non-zero element size for a struct parameter.");
		return;
	}

	const auto iterFind = DataParameters.find(paramDesc.Name);
	if(iterFind != DataParameters.end())
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

	DataParameters[paramDesc.Name] = paramDesc;
}

void ShaderInformationBase::AddParameter(ShaderObjectParameterInformation paramDesc)
{
	u32 defaultValueIdx = (u32)-1;

	AddParameterInternal(std::move(paramDesc), defaultValueIdx);
}

void ShaderInformationBase::AddParameter(ShaderObjectParameterInformation paramDesc, const SamplerStateCreateInformation& defaultValue)
{
	u32 defaultValueIdx = (u32)-1;
	if(Shader::IsSampler(paramDesc.Type))
	{
		defaultValueIdx = (u32)SamplerDefaultValues.size();
		SamplerDefaultValues.push_back(defaultValue);
	}

	AddParameterInternal(std::move(paramDesc), defaultValueIdx);
}

void ShaderInformationBase::AddParameter(ShaderObjectParameterInformation paramDesc, ShaderDefaultTextureType defaultValue)
{
	u32 defaultValueIdx = (u32)-1;
	if(Shader::IsTexture(paramDesc.Type))
	{
		defaultValueIdx = (u32)TextureDefaultValues.size();
		TextureDefaultValues.push_back(defaultValue);
	}

	AddParameterInternal(std::move(paramDesc), defaultValueIdx);
}

void ShaderInformationBase::AddParameterInternal(ShaderObjectParameterInformation paramDesc, u32 defaultValueIdx)
{
	Map<String, ShaderObjectParameterInformation>* DEST_LOOKUP[] = { &TextureParameters, &BufferParameters, &SamplerParameters };
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

void ShaderInformationBase::SetParameterAttribute(const String& name, const ShaderParameterAttribute& attrib)
{
	ShaderDataParameterInformation* paramDescData = nullptr;

	const auto findIterData = DataParameters.find(name);
	if(findIterData != DataParameters.end())
		paramDescData = &findIterData->second;

	ShaderObjectParameterInformation* paramDescObj = nullptr;
	if(!paramDescData)
	{
		const auto findIterTexture = TextureParameters.find(name);
		if(findIterTexture != TextureParameters.end())
			paramDescObj = &findIterTexture->second;

		if(!paramDescObj)
		{
			const auto findIterSampler = SamplerParameters.find(name);
			if(findIterSampler != SamplerParameters.end())
				paramDescObj = &findIterSampler->second;
		}

		if(!paramDescObj)
		{
			const auto findIterBuffer = BufferParameters.find(name);
			if(findIterBuffer != BufferParameters.end())
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

void ShaderInformationBase::SetParamBlockAttribs(const String& name, bool shared, GpuBufferFlags flags, StringID rendererSemantic)
{
	ShaderParameterBlockInformation desc;
	desc.Name = name;
	desc.Shared = shared;
	desc.Flags = flags;
	desc.RendererSemantic = rendererSemantic;

	DataParameterBlocks[name] = desc;
}

RTTITypeBase* ShaderInformationBase::GetRttiStatic()
{
	return ShaderInformationBaseRTTI::Instance();
}

RTTITypeBase* ShaderInformationBase::GetRtti() const
{
	return GetRttiStatic();
}

ct::ShaderInformation ShaderInformation::ConvertToCore(const ShaderInformation& value)
{
	ct::ShaderInformation output;
	output.DataParameters = value.DataParameters;
	output.TextureParameters = value.TextureParameters;
	output.SamplerParameters = value.SamplerParameters;
	output.BufferParameters = value.BufferParameters;
	output.DataParameterBlocks = value.DataParameterBlocks;
	output.ParamAttributes = value.ParamAttributes;

	output.DataDefaultValues = value.DataDefaultValues;
	output.SamplerDefaultValues = value.SamplerDefaultValues;
	output.TextureDefaultValues = value.TextureDefaultValues;

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

RTTITypeBase* ShaderInformation::GetRttiStatic()
{
	return ShaderInformationRTTI::Instance();
}

RTTITypeBase* ShaderInformation::GetRtti() const
{
	return GetRttiStatic();
}

namespace bs::ct {
RTTITypeBase* ShaderInformation::GetRttiStatic()
{
	return CoreShaderInformationRTTI::Instance();
}

RTTITypeBase* ShaderInformation::GetRtti() const
{
	return GetRttiStatic();
}
} // namespace ct

template <bool Core>
CoreVariantHandleType<Texture, Core> GetBuiltin2DTexture(ShaderDefaultTextureType texture);

template <>
CoreVariantHandleType<Texture, true> GetBuiltin2DTexture<true>(ShaderDefaultTextureType texture)
{
	if(texture == ShaderDefaultTextureType::White)
		return ct::BuiltinResources::Instance().WhiteTexture2D;
	else if(texture == ShaderDefaultTextureType::Black)
		return ct::BuiltinResources::Instance().BlackTexture2D;
	else if(texture == ShaderDefaultTextureType::Normal)
		return ct::BuiltinResources::Instance().NormalTexture2D;

	return nullptr;
}

template <>
CoreVariantHandleType<Texture, false> GetBuiltin2DTexture<false>(ShaderDefaultTextureType texture)
{
	if(texture == ShaderDefaultTextureType::White)
		return BuiltinResources::GetTexture(BuiltinTexture::White);
	else if(texture == ShaderDefaultTextureType::Black)
		return BuiltinResources::GetTexture(BuiltinTexture::Black);
	else if(texture == ShaderDefaultTextureType::Normal)
		return BuiltinResources::GetTexture(BuiltinTexture::Normal);

	return HTexture();
}

template <bool Core>
CoreVariantHandleType<Texture, Core> GetBuiltin3DTexture(ShaderDefaultTextureType texture);

template <>
CoreVariantHandleType<Texture, true> GetBuiltin3DTexture<true>(ShaderDefaultTextureType texture)
{
	if(texture == ShaderDefaultTextureType::White)
		return ct::BuiltinResources::Instance().WhiteTexture3D;
	else if(texture == ShaderDefaultTextureType::Black)
		return ct::BuiltinResources::Instance().BlackTexture3D;

	return nullptr;
}

template <>
CoreVariantHandleType<Texture, false> GetBuiltin3DTexture<false>(ShaderDefaultTextureType texture)
{
	if(texture == ShaderDefaultTextureType::White)
		return BuiltinResources::GetTexture(BuiltinTexture::White3D);
	else if(texture == ShaderDefaultTextureType::Black)
		return BuiltinResources::GetTexture(BuiltinTexture::Black3D);

	return HTexture();
}

template <bool Core>
TShader<Core>::TShader(u32 id)
	: mShaderId(id)
{}

template <bool Core>
TShader<Core>::TShader(const String& name, const ShaderCreateInformationType& createInformation, u32 id)
	: mName(name), mInformation(createInformation), mShaderId(id)
{}

template <bool Core>
TShader<Core>::~TShader()
{}

template <bool Core>
const ShaderDataParameterInformation& TShader<Core>::GetDataParamDesc(const String& name) const
{
	auto findIterData = mInformation.DataParameters.find(name);
	if(findIterData != mInformation.DataParameters.end())
		return findIterData->second;

	B3D_EXCEPT(InternalErrorException, "Cannot find the parameter with the name: " + name);
	static ShaderDataParameterInformation dummy;
	return dummy;
}

template <bool Core>
const ShaderObjectParameterInformation& TShader<Core>::GetTextureParamDesc(const String& name) const
{
	auto findIterObject = mInformation.TextureParameters.find(name);
	if(findIterObject != mInformation.TextureParameters.end())
		return findIterObject->second;

	B3D_EXCEPT(InternalErrorException, "Cannot find the parameter with the name: " + name);
	static ShaderObjectParameterInformation dummy;
	return dummy;
}

template <bool Core>
const ShaderObjectParameterInformation& TShader<Core>::GetSamplerParamDesc(const String& name) const
{
	auto findIterObject = mInformation.SamplerParameters.find(name);
	if(findIterObject != mInformation.SamplerParameters.end())
		return findIterObject->second;

	B3D_EXCEPT(InternalErrorException, "Cannot find the parameter with the name: " + name);
	static ShaderObjectParameterInformation dummy;
	return dummy;
}

template <bool Core>
const ShaderObjectParameterInformation& TShader<Core>::GetBufferParamDesc(const String& name) const
{
	auto findIterObject = mInformation.BufferParameters.find(name);
	if(findIterObject != mInformation.BufferParameters.end())
		return findIterObject->second;

	B3D_EXCEPT(InternalErrorException, "Cannot find the parameter with the name: " + name);
	static ShaderObjectParameterInformation dummy;
	return dummy;
}

template <bool Core>
bool TShader<Core>::HasDataParam(const String& name) const
{
	auto findIterData = mInformation.DataParameters.find(name);
	if(findIterData != mInformation.DataParameters.end())
		return true;

	return false;
}

template <bool Core>
bool TShader<Core>::HasTextureParam(const String& name) const
{
	auto findIterObject = mInformation.TextureParameters.find(name);
	if(findIterObject != mInformation.TextureParameters.end())
		return true;

	return false;
}

template <bool Core>
bool TShader<Core>::HasSamplerParam(const String& name) const
{
	auto findIterObject = mInformation.SamplerParameters.find(name);
	if(findIterObject != mInformation.SamplerParameters.end())
		return true;

	return false;
}

template <bool Core>
bool TShader<Core>::HasBufferParam(const String& name) const
{
	auto findIterObject = mInformation.BufferParameters.find(name);
	if(findIterObject != mInformation.BufferParameters.end())
		return true;

	return false;
}

template <bool Core>
bool TShader<Core>::HasParamBlock(const String& name) const
{
	auto findIterObject = mInformation.DataParameterBlocks.find(name);
	if(findIterObject != mInformation.DataParameterBlocks.end())
		return true;

	return false;
}

template <bool Core>
typename TShader<Core>::TextureType TShader<Core>::GetDefault2DTexture(u32 index) const
{
	if(index < (u32)mInformation.TextureDefaultValues.size())
		return GetBuiltin2DTexture<Core>(mInformation.TextureDefaultValues[index]);

	return TextureType();
}

template <bool Core>
typename TShader<Core>::TextureType TShader<Core>::GetDefault3DTexture(u32 index) const
{
	if(index < (u32)mInformation.TextureDefaultValues.size())
		return GetBuiltin3DTexture<Core>(mInformation.TextureDefaultValues[index]);

	return TextureType();
}

template <bool Core>
SPtr<SamplerState> TShader<Core>::GetDefaultSampler(u32 index) const
{
	if (index < (u32)mInformation.SamplerDefaultValues.size())
	{
		const SPtr<GpuDevice> gpuDevice = GetCoreApplication().GetPrimaryGpuDevice();
		if (!B3D_ENSURE(gpuDevice))
			return nullptr;

		return gpuDevice->CreateSamplerState(mInformation.SamplerDefaultValues[index]);
	}

	return SPtr<SamplerState>();
}

template <bool Core>
u8* TShader<Core>::GetDefaultValue(u32 index) const
{
	if(index < (u32)mInformation.DataDefaultValues.size())
		return (u8*)&mInformation.DataDefaultValues[index];

	return nullptr;
}

template <bool Core>
Vector<SPtr<typename TShader<Core>::TechniqueType>> TShader<Core>::GetCompatibleTechniques() const
{
	Vector<SPtr<TechniqueType>> output;
	for(auto& technique : mInformation.Techniques)
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
	for(auto& technique : mInformation.Techniques)
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
	for(auto& technique : mInformation.Techniques)
		techniques.push_back(technique->GetCore());

	ct::Shader* shaderCore = new(B3DAllocate<ct::Shader>()) ct::Shader(mName, ShaderInformation::ConvertToCore(mInformation), mShaderId);
	SPtr<ct::Shader> shaderCorePtr = B3DMakeSharedFromExisting<ct::Shader>(shaderCore);
	shaderCorePtr->SetShared(shaderCorePtr);

	return shaderCorePtr;
}

void Shader::GetCoreDependencies(Vector<CoreObject*>& dependencies)
{
	for(auto& technique : mInformation.Techniques)
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
	if(idx < sizeof(GpuParameters::kParamSizes.Lookup))
		return GpuParameters::kParamSizes.Lookup[idx].Size;

	return 0;
}

HShader Shader::Create(const String& name, const ShaderCreateInformation& createInformation)
{
	SPtr<Shader> newShader = CreateShared(name, createInformation);

	return B3DStaticResourceCast<Shader>(GetResources().CreateResourceHandle(newShader));
}

SPtr<Shader> Shader::CreateShared(const String& name, const ShaderCreateInformation& createInformation)
{
	u32 id = ct::Shader::mNextShaderId.fetch_add(1, std::memory_order_relaxed);
	B3D_ASSERT(id < std::numeric_limits<u32>::max() && "Created too many shaders, reached maximum id.");

	SPtr<Shader> newShader = B3DMakeSharedFromExisting<Shader>(new(B3DAllocate<Shader>()) Shader(name, createInformation, id));
	newShader->SetShared(newShader);
	newShader->Initialize();

	return newShader;
}

SPtr<Shader> Shader::CreateEmpty()
{
	u32 id = ct::Shader::mNextShaderId.fetch_add(1, std::memory_order_relaxed);
	B3D_ASSERT(id < std::numeric_limits<u32>::max() && "Created too many shaders, reached maximum id.");

	SPtr<Shader> newShader = B3DMakeSharedFromExisting<Shader>(new(B3DAllocate<Shader>()) Shader(id));
	newShader->SetShared(newShader);

	return newShader;
}

Array<u64, 2> Shader::ComputeHash(const String& string)
{
	const uint128 hash = CityHash128(string.data(), string.length());
	return { hash.first, hash.second };
}

Array<u64, 2> Shader::ComputeIncludeHash(const String& path)
{
	const Optional<String> shaderIncludeSource = ShaderManager::Instance().FindIncludeSource(path);
	if(!shaderIncludeSource.has_value())
		return { 0, 0 };

	return ComputeHash(shaderIncludeSource.value());
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

Shader::Shader( u32 id)
	: TShader(id)
{ }

Shader::Shader(const String& name, const ShaderCreateInformation& createInformation, u32 id)
	: TShader(name, createInformation, id)
{ }

SPtr<Shader> Shader::Create(const String& name, const ShaderCreateInformation& createInformation)
{
	const u32 id = mNextShaderId.fetch_add(1, std::memory_order_relaxed);
	B3D_ASSERT(id < std::numeric_limits<u32>::max() && "Created too many shaders, reached maximum id.");

	Shader* const shaderCore = new(B3DAllocate<Shader>()) Shader(name, createInformation, id);
	SPtr<Shader> shaderCorePtr = B3DMakeSharedFromExisting<Shader>(shaderCore);
	shaderCorePtr->SetShared(shaderCorePtr);
	shaderCorePtr->Initialize();

	return shaderCorePtr;
}

SPtr<Shader> Shader::CreateEmpty()
{
	const uint32 id = mNextShaderId.fetch_add(1, std::memory_order_relaxed);
	B3D_ASSERT(id < std::numeric_limits<uint32>::max() && "Created too many shaders, reached maximum id.");

	Shader* const shaderCore = new(B3DAllocate<Shader>()) Shader(id);
	SPtr<Shader> shaderCoreShared = B3DMakeSharedFromExisting<Shader>(shaderCore);
	shaderCoreShared->SetShared(shaderCoreShared);

	return shaderCoreShared;
}

RTTITypeBase* Shader::GetRttiStatic()
{
	return CoreShaderRTTI::Instance();
}

RTTITypeBase* Shader::GetRtti() const
{
	return GetRttiStatic();
}

}}
