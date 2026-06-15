//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Material/B3DShader.h"

#include "B3DShaderManager.h"
#include "Material/B3DVariation.h"
#include "Debug/B3DDebug.h"
#include "RTTI/B3DShaderRTTI.h"
#include "Resources/B3DResources.h"
#include "GpuBackend/B3DGpuParameterSet.h"
#include "Material/B3DPass.h"
#include "GpuBackend/B3DSamplerState.h"
#include "Image/B3DTexture.h"
#include "GpuBackend/B3DGpuDevice.h"
#include "Resources/B3DBuiltinResources.h"
#include "ThirdParty/CityHash/city.h"

using namespace b3d;

ShaderInformationBase::ShaderInformationBase()
	: QueueSortType(QueueSortType::None), QueuePriority(0), SeparablePasses(false), Flags(0)
{
}

void ShaderInformationBase::AddParameter(ShaderDataParameterInformation parameterInformation, u8* defaultValue)
{
	if(parameterInformation.Type == GPDT_STRUCT && parameterInformation.ElementSize <= 0)
	{
		B3D_LOG(Error, LogMaterial, "You need to provide a non-zero element size for a struct parameter.");
		return;
	}

	const auto found = DataParameters.find(parameterInformation.Name);
	if(found != DataParameters.end())
		return;

	if(defaultValue != nullptr)
	{
		parameterInformation.DefaultValueIndex = (u32)DataDefaultValues.size();
		u32 defaultValueSize = Shader::GetDataParameterSize(parameterInformation.Type);

		DataDefaultValues.resize(parameterInformation.DefaultValueIndex + defaultValueSize);
		memcpy(&DataDefaultValues[parameterInformation.DefaultValueIndex], defaultValue, defaultValueSize);
	}
	else
		parameterInformation.DefaultValueIndex = (u32)-1;

	DataParameters[parameterInformation.Name] = parameterInformation;
}

void ShaderInformationBase::AddParameter(ShaderObjectParameterInformation parameterInformation)
{
	AddParameterInternal(std::move(parameterInformation), ~0u);
}

void ShaderInformationBase::AddParameter(ShaderObjectParameterInformation parameterInformation, const SamplerStateCreateInformation& defaultValue)
{
	u32 defaultValueIndex = ~0u;
	if(Shader::IsSampler(parameterInformation.Type))
	{
		defaultValueIndex = (u32)SamplerDefaultValues.size();
		SamplerDefaultValues.push_back(defaultValue);
	}

	AddParameterInternal(std::move(parameterInformation), defaultValueIndex);
}

void ShaderInformationBase::AddParameter(ShaderObjectParameterInformation parameterInformation, ShaderDefaultTextureType defaultValue)
{
	u32 defaultValueIndex = ~0u;
	if(Shader::IsTexture(parameterInformation.Type))
	{
		defaultValueIndex = (u32)TextureDefaultValues.size();
		TextureDefaultValues.push_back(defaultValue);
	}

	AddParameterInternal(std::move(parameterInformation), defaultValueIndex);
}

void ShaderInformationBase::AddParameterInternal(ShaderObjectParameterInformation parameterInformation, u32 defaultValueIndex)
{
	Map<String, ShaderObjectParameterInformation>* destinationLookup[] = { &TextureParameters, &BufferParameters, &SamplerParameters };
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
		ShaderObjectParameterInformation& parameterInformation = found->second;

		// If same name but different properties, we ignore this param
		if(parameterInformation.Type != parameterInformation.Type || parameterInformation.RendererSemantic != parameterInformation.RendererSemantic)
			return;

		Vector<String>& gpuVariableNames = parameterInformation.GpuVariableNames;
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

void ShaderInformationBase::SetParameterAttribute(const String& name, const ShaderParameterAttribute& attribute)
{
	ShaderDataParameterInformation* dataParameterInformation = nullptr;

	const auto foundDataParameter = DataParameters.find(name);
	if(foundDataParameter != DataParameters.end())
		dataParameterInformation = &foundDataParameter->second;

	ShaderObjectParameterInformation* objectParameterInformation = nullptr;
	if(!dataParameterInformation)
	{
		const auto foundTextureParameter = TextureParameters.find(name);
		if(foundTextureParameter != TextureParameters.end())
			objectParameterInformation = &foundTextureParameter->second;

		if(!objectParameterInformation)
		{
			const auto foundSamplerParameter = SamplerParameters.find(name);
			if(foundSamplerParameter != SamplerParameters.end())
				objectParameterInformation = &foundSamplerParameter->second;
		}

		if(!objectParameterInformation)
		{
			const auto foundBufferParameter = BufferParameters.find(name);
			if(foundBufferParameter != BufferParameters.end())
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
		ShaderParameterAttribute& currentAttribute = ParameterAttributes[currentAttributeIndex];
		if(currentAttribute.Type == attribute.Type)
		{
			currentAttribute = attribute;

			found = true;
			break;
		}

		currentAttributeIndex = currentAttribute.NextParameterIndex;
	}

	if(!found)
	{
		const auto attributeIndex = (u32)ParameterAttributes.size();
		ParameterAttributes.emplace_back(attribute);

		if(parameterInformation->AttributeIndex != ~0u)
			ParameterAttributes.back().NextParameterIndex = parameterInformation->AttributeIndex;

		parameterInformation->AttributeIndex = attributeIndex;
	}
}

void ShaderInformationBase::SetUniformBufferAttributes(const String& name, bool shared, GpuBufferFlags flags, StringID rendererSemantic)
{
	ShaderUniformBufferInformation information;
	information.Name = name;
	information.Shared = shared;
	information.Flags = flags;
	information.RendererSemantic = rendererSemantic;

	UniformBuffers[name] = information;
}

RTTIType* ShaderInformationBase::GetRttiStatic()
{
	return ShaderInformationBaseRTTI::Instance();
}

RTTIType* ShaderInformationBase::GetRtti() const
{
	return GetRttiStatic();
}

render::ShaderInformation ShaderInformation::ConvertToRenderProxy(const ShaderInformation& value)
{
	render::ShaderInformation output;
	output.DataParameters = value.DataParameters;
	output.TextureParameters = value.TextureParameters;
	output.SamplerParameters = value.SamplerParameters;
	output.BufferParameters = value.BufferParameters;
	output.UniformBuffers = value.UniformBuffers;
	output.ParameterAttributes = value.ParameterAttributes;

	output.DataDefaultValues = value.DataDefaultValues;
	output.SamplerDefaultValues = value.SamplerDefaultValues;
	output.TextureDefaultValues = value.TextureDefaultValues;

	output.QueuePriority = value.QueuePriority;
	output.QueueSortType = value.QueueSortType;
	output.SeparablePasses = value.SeparablePasses;
	output.Flags = value.Flags;

	for(auto& entry : value.Variations)
	{
		if(entry != nullptr)
			output.Variations.push_back(B3DGetRenderProxy(entry));
	}

	output.VariationParameters = value.VariationParameters;
	output.CompilerMetaData = value.CompilerMetaData;

	// Ignoring default values as they are not needed for syncing since
	// they're initialized through the material.
	return output;
}

RTTIType* ShaderInformation::GetRttiStatic()
{
	return ShaderInformationRTTI::Instance();
}

RTTIType* ShaderInformation::GetRtti() const
{
	return GetRttiStatic();
}

namespace b3d::render {
RTTIType* ShaderInformation::GetRttiStatic()
{
	return ShaderInformationRenderProxyRTTI::Instance();
}

RTTIType* ShaderInformation::GetRtti() const
{
	return GetRttiStatic();
}
} // namespace render

template <bool IsRenderProxy>
CoreVariantHandleType<Texture, IsRenderProxy> GetBuiltin2DTexture(ShaderDefaultTextureType texture);

template <>
CoreVariantHandleType<Texture, true> GetBuiltin2DTexture<true>(ShaderDefaultTextureType texture)
{
	if(texture == ShaderDefaultTextureType::White)
		return render::BuiltinResources::Instance().WhiteTexture2D;
	else if(texture == ShaderDefaultTextureType::Black)
		return render::BuiltinResources::Instance().BlackTexture2D;
	else if(texture == ShaderDefaultTextureType::Normal)
		return render::BuiltinResources::Instance().NormalTexture2D;

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

template <bool IsRenderProxy>
CoreVariantHandleType<Texture, IsRenderProxy> GetBuiltin3DTexture(ShaderDefaultTextureType texture);

template <>
CoreVariantHandleType<Texture, true> GetBuiltin3DTexture<true>(ShaderDefaultTextureType texture)
{
	if(texture == ShaderDefaultTextureType::White)
		return render::BuiltinResources::Instance().WhiteTexture3D;
	else if(texture == ShaderDefaultTextureType::Black)
		return render::BuiltinResources::Instance().BlackTexture3D;

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

template <bool IsRenderProxy>
TShader<IsRenderProxy>::TShader(u32 id)
	: mShaderId(id)
{}

template <bool IsRenderProxy>
TShader<IsRenderProxy>::TShader(const ShaderCreateInformationType& createInformation, u32 id)
	: mInformation(createInformation), mShaderId(id)
{
}

template <bool IsRenderProxy>
TShader<IsRenderProxy>::~TShader()
{}

template <bool IsRenderProxy>
const ShaderDataParameterInformation* TShader<IsRenderProxy>::GetDataParameterDescription(const String& name) const
{
	auto findIterData = mInformation.DataParameters.find(name);
	if(findIterData != mInformation.DataParameters.end())
		return &findIterData->second;

	return nullptr;
}

template <bool IsRenderProxy>
const ShaderObjectParameterInformation* TShader<IsRenderProxy>::GetTextureParameterDescription(const String& name) const
{
	auto findIterObject = mInformation.TextureParameters.find(name);
	if(findIterObject != mInformation.TextureParameters.end())
		return &findIterObject->second;

	return nullptr;
}

template <bool IsRenderProxy>
const ShaderObjectParameterInformation* TShader<IsRenderProxy>::GetSamplerParameterDescription(const String& name) const
{
	auto findIterObject = mInformation.SamplerParameters.find(name);
	if(findIterObject != mInformation.SamplerParameters.end())
		return &findIterObject->second;

	return nullptr;
}

template <bool IsRenderProxy>
const ShaderObjectParameterInformation* TShader<IsRenderProxy>::GetBufferParameterInformation(const String& name) const
{
	auto findIterObject = mInformation.BufferParameters.find(name);
	if(findIterObject != mInformation.BufferParameters.end())
		return &findIterObject->second;

	return nullptr;
}

template <bool IsRenderProxy>
bool TShader<IsRenderProxy>::HasDataParameter(const String& name) const
{
	auto findIterData = mInformation.DataParameters.find(name);
	if(findIterData != mInformation.DataParameters.end())
		return true;

	return false;
}

template <bool IsRenderProxy>
bool TShader<IsRenderProxy>::HasTextureParameter(const String& name) const
{
	auto findIterObject = mInformation.TextureParameters.find(name);
	if(findIterObject != mInformation.TextureParameters.end())
		return true;

	return false;
}

template <bool IsRenderProxy>
bool TShader<IsRenderProxy>::HasSamplerParameter(const String& name) const
{
	auto findIterObject = mInformation.SamplerParameters.find(name);
	if(findIterObject != mInformation.SamplerParameters.end())
		return true;

	return false;
}

template <bool IsRenderProxy>
bool TShader<IsRenderProxy>::HasBufferParameter(const String& name) const
{
	auto findIterObject = mInformation.BufferParameters.find(name);
	if(findIterObject != mInformation.BufferParameters.end())
		return true;

	return false;
}

template <bool IsRenderProxy>
bool TShader<IsRenderProxy>::HasUniformBuffer(const String& name) const
{
	auto findIterObject = mInformation.UniformBuffers.find(name);
	if(findIterObject != mInformation.UniformBuffers.end())
		return true;

	return false;
}

template <bool IsRenderProxy>
typename TShader<IsRenderProxy>::TextureType TShader<IsRenderProxy>::GetDefault2DTexture(u32 index) const
{
	if(index < (u32)mInformation.TextureDefaultValues.size())
		return GetBuiltin2DTexture<IsRenderProxy>(mInformation.TextureDefaultValues[index]);

	return TextureType();
}

template <bool IsRenderProxy>
typename TShader<IsRenderProxy>::TextureType TShader<IsRenderProxy>::GetDefault3DTexture(u32 index) const
{
	if(index < (u32)mInformation.TextureDefaultValues.size())
		return GetBuiltin3DTexture<IsRenderProxy>(mInformation.TextureDefaultValues[index]);

	return TextureType();
}

template <bool IsRenderProxy>
TShared<SamplerState> TShader<IsRenderProxy>::GetDefaultSampler(u32 index) const
{
	if (index < (u32)mInformation.SamplerDefaultValues.size())
	{
		const TShared<GpuDevice> gpuDevice = GetApplication().GetPrimaryGpuDevice();
		if (!B3D_ENSURE(gpuDevice))
			return nullptr;

		return gpuDevice->CreateSamplerState(mInformation.SamplerDefaultValues[index]);
	}

	return TShared<SamplerState>();
}

template <bool IsRenderProxy>
u8* TShader<IsRenderProxy>::GetDefaultValue(u32 index) const
{
	if(index < (u32)mInformation.DataDefaultValues.size())
		return (u8*)&mInformation.DataDefaultValues[index];

	return nullptr;
}

template <bool IsRenderProxy>
Vector<TShared<typename TShader<IsRenderProxy>::VariationType>> TShader<IsRenderProxy>::GetCompatibleVariations() const
{
	Vector<TShared<VariationType>> output;
	for(auto& variation : mInformation.Variations)
	{
		if(variation->IsSupported())
			output.push_back(variation);
	}

	return output;
}

template <bool IsRenderProxy>
Vector<TShared<typename TShader<IsRenderProxy>::VariationType>> TShader<IsRenderProxy>::GetCompatibleVariations(
	const ShaderVariationParameters& variationParameters, bool exact) const
{
	Vector<TShared<VariationType>> output;
	for(auto& variation : mInformation.Variations)
	{
		if(variation->IsSupported() && variation->GetVariationParameters().Matches(variationParameters, exact))
			output.push_back(variation);
	}

	return output;
}

namespace b3d
{
	template class TShader<false>;
	template class TShader<true>;
} // namespace b3d

Shader::Shader(const String& name, const ShaderCreateInformation& createInformation, u32 id)
	: Resource(true, name), TShader(createInformation, id)
{
	mMetaData = B3DMakeShared<ShaderMetaData>();
}

Shader::Shader(u32 id)
	: TShader(id)
{}

void Shader::SetIncludeFiles(const Vector<String>& includes)
{
	TShared<ShaderMetaData> meta = std::static_pointer_cast<ShaderMetaData>(GetMetaData());
	meta->Includes = includes;
}

TShared<render::RenderProxy> Shader::CreateRenderProxy() const
{
	Vector<TShared<render::Variation>> variations;
	for(auto& variation : mInformation.Variations)
		variations.push_back(B3DGetRenderProxy(variation));

	render::Shader* renderProxy = new(B3DAllocate<render::Shader>()) render::Shader(mName, ShaderInformation::ConvertToRenderProxy(mInformation), mShaderId);
	TShared<render::Shader> renderProxyShared = B3DMakeSharedFromExisting<render::Shader>(renderProxy);
	renderProxyShared->SetShared(renderProxyShared);

	return renderProxyShared;
}

void Shader::GetCoreDependencies(Vector<CoreObject*>& dependencies)
{
	for(auto& variation : mInformation.Variations)
		dependencies.push_back(variation.get());
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

u32 Shader::GetDataParameterSize(GpuDataParameterType type)
{
	static const GpuDataParameterTypeInformationLookup kParamSizes;

	u32 idx = (u32)type;
	if(idx < sizeof(GpuParameterSet::kParamSizes.Lookup))
		return GpuParameterSet::kParamSizes.Lookup[idx].Size;

	return 0;
}

HShader Shader::Create(const String& name, const ShaderCreateInformation& createInformation)
{
	TShared<Shader> newShader = CreateShared(name, createInformation);

	return B3DStaticResourceCast<Shader>(GetResources().CreateResourceHandle(newShader));
}

TShared<Shader> Shader::CreateShared(const String& name, const ShaderCreateInformation& createInformation)
{
	u32 id = render::Shader::mNextShaderId.fetch_add(1, std::memory_order_relaxed);
	B3D_ASSERT(id < std::numeric_limits<u32>::max() && "Created too many shaders, reached maximum id.");

	TShared<Shader> newShader = B3DMakeSharedFromExisting<Shader>(new(B3DAllocate<Shader>()) Shader(name, createInformation, id));
	newShader->SetShared(newShader);
	newShader->Initialize();

	return newShader;
}

TShared<Shader> Shader::CreateEmpty()
{
	u32 id = render::Shader::mNextShaderId.fetch_add(1, std::memory_order_relaxed);
	B3D_ASSERT(id < std::numeric_limits<u32>::max() && "Created too many shaders, reached maximum id.");

	TShared<Shader> newShader = B3DMakeSharedFromExisting<Shader>(new(B3DAllocate<Shader>()) Shader(id));
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
	const TOptional<String> shaderIncludeSource = ShaderManager::Instance().FindIncludeSource(path);
	if(!shaderIncludeSource.has_value())
		return { 0, 0 };

	return ComputeHash(shaderIncludeSource.value());
}

RTTIType* Shader::GetRttiStatic()
{
	return ShaderRTTI::Instance();
}

RTTIType* Shader::GetRtti() const
{
	return Shader::GetRttiStatic();
}

RTTIType* ShaderMetaData::GetRttiStatic()
{
	return ShaderMetaDataRTTI::Instance();
}

RTTIType* ShaderMetaData::GetRtti() const
{
	return ShaderMetaData::GetRttiStatic();
}

namespace b3d { namespace render
{
std::atomic<u32> Shader::mNextShaderId;

Shader::Shader( u32 id)
	: TShader(id)
{ }

Shader::Shader(const String& name, const ShaderCreateInformation& createInformation, u32 id)
	: TShader(createInformation, id), mName(name)
{ }

TShared<Shader> Shader::Create(const String& name, const ShaderCreateInformation& createInformation)
{
	const u32 id = mNextShaderId.fetch_add(1, std::memory_order_relaxed);
	B3D_ASSERT(id < std::numeric_limits<u32>::max() && "Created too many shaders, reached maximum id.");

	Shader* const shader = new(B3DAllocate<Shader>()) Shader(name, createInformation, id);
	TShared<Shader> shaderShared = B3DMakeSharedFromExisting<Shader>(shader);
	shaderShared->SetShared(shaderShared);
	shaderShared->Initialize();

	return shaderShared;
}

TShared<Shader> Shader::CreateEmpty()
{
	const uint32 id = mNextShaderId.fetch_add(1, std::memory_order_relaxed);
	B3D_ASSERT(id < std::numeric_limits<uint32>::max() && "Created too many shaders, reached maximum id.");

	Shader* const shader = new(B3DAllocate<Shader>()) Shader(id);
	TShared<Shader> shaderShared = B3DMakeSharedFromExisting<Shader>(shader);
	shaderShared->SetShared(shaderShared);

	return shaderShared;
}

RTTIType* Shader::GetRttiStatic()
{
	return ShaderRenderProxyRTTI::Instance();
}

RTTIType* Shader::GetRtti() const
{
	return GetRttiStatic();
}

}}
