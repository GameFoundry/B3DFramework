//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Material/B3DShader.h"

#include "B3DShaderManager.h"
#include "Material/B3DVariation.h"
#include "Material/B3DShaderCompiler.h"
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

render::ShaderCreateInformation ShaderCreateInformation::ConvertToRenderProxy(const ShaderCreateInformation& value)
{
	render::ShaderCreateInformation output(value.Description);
	output.CompilerMetaData = value.CompilerMetaData;

	for(auto& entry : value.Variations)
	{
		if(entry != nullptr)
			output.Variations.push_back(B3DGetRenderProxy(entry));
	}

	return output;
}

RTTIType* ShaderDescription::GetRttiStatic()
{
	return ShaderDescriptionRTTI::Instance();
}

RTTIType* ShaderDescription::GetRtti() const
{
	return GetRttiStatic();
}

RTTIType* ShaderCreateInformation::GetRttiStatic()
{
	return ShaderCreateInformationRTTI::Instance();
}

RTTIType* ShaderCreateInformation::GetRtti() const
{
	return GetRttiStatic();
}

RTTIType* PrecompiledShaderData::GetRttiStatic()
{
	return PrecompiledShaderDataRTTI::Instance();
}

RTTIType* PrecompiledShaderData::GetRtti() const
{
	return GetRttiStatic();
}

namespace b3d::render {
RTTIType* ShaderCreateInformation::GetRttiStatic()
{
	return ShaderCreateInformationRenderProxyRTTI::Instance();
}

RTTIType* ShaderCreateInformation::GetRtti() const
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
	auto findIterData = mInformation.Description->Parameters->GetDataParameters().find(name);
	if(findIterData != mInformation.Description->Parameters->GetDataParameters().end())
		return &findIterData->second;

	return nullptr;
}

template <bool IsRenderProxy>
const ShaderObjectParameterInformation* TShader<IsRenderProxy>::GetTextureParameterDescription(const String& name) const
{
	auto findIterObject = mInformation.Description->Parameters->GetTextureParameters().find(name);
	if(findIterObject != mInformation.Description->Parameters->GetTextureParameters().end())
		return &findIterObject->second;

	return nullptr;
}

template <bool IsRenderProxy>
const ShaderObjectParameterInformation* TShader<IsRenderProxy>::GetSamplerParameterDescription(const String& name) const
{
	auto findIterObject = mInformation.Description->Parameters->GetSamplerParameters().find(name);
	if(findIterObject != mInformation.Description->Parameters->GetSamplerParameters().end())
		return &findIterObject->second;

	return nullptr;
}

template <bool IsRenderProxy>
const ShaderObjectParameterInformation* TShader<IsRenderProxy>::GetBufferParameterInformation(const String& name) const
{
	auto findIterObject = mInformation.Description->Parameters->GetBufferParameters().find(name);
	if(findIterObject != mInformation.Description->Parameters->GetBufferParameters().end())
		return &findIterObject->second;

	return nullptr;
}

template <bool IsRenderProxy>
bool TShader<IsRenderProxy>::HasDataParameter(const String& name) const
{
	auto findIterData = mInformation.Description->Parameters->GetDataParameters().find(name);
	if(findIterData != mInformation.Description->Parameters->GetDataParameters().end())
		return true;

	return false;
}

template <bool IsRenderProxy>
bool TShader<IsRenderProxy>::HasTextureParameter(const String& name) const
{
	auto findIterObject = mInformation.Description->Parameters->GetTextureParameters().find(name);
	if(findIterObject != mInformation.Description->Parameters->GetTextureParameters().end())
		return true;

	return false;
}

template <bool IsRenderProxy>
bool TShader<IsRenderProxy>::HasSamplerParameter(const String& name) const
{
	auto findIterObject = mInformation.Description->Parameters->GetSamplerParameters().find(name);
	if(findIterObject != mInformation.Description->Parameters->GetSamplerParameters().end())
		return true;

	return false;
}

template <bool IsRenderProxy>
bool TShader<IsRenderProxy>::HasBufferParameter(const String& name) const
{
	auto findIterObject = mInformation.Description->Parameters->GetBufferParameters().find(name);
	if(findIterObject != mInformation.Description->Parameters->GetBufferParameters().end())
		return true;

	return false;
}

template <bool IsRenderProxy>
bool TShader<IsRenderProxy>::HasUniformBuffer(const String& name) const
{
	auto findIterObject = mInformation.Description->Parameters->GetUniformBuffers().find(name);
	if(findIterObject != mInformation.Description->Parameters->GetUniformBuffers().end())
		return true;

	return false;
}

template <bool IsRenderProxy>
typename TShader<IsRenderProxy>::TextureType TShader<IsRenderProxy>::GetDefault2DTexture(u32 index) const
{
	if(index < (u32)mInformation.Description->Parameters->GetTextureDefaultValues().size())
		return GetBuiltin2DTexture<IsRenderProxy>(mInformation.Description->Parameters->GetTextureDefaultValues()[index]);

	return TextureType();
}

template <bool IsRenderProxy>
typename TShader<IsRenderProxy>::TextureType TShader<IsRenderProxy>::GetDefault3DTexture(u32 index) const
{
	if(index < (u32)mInformation.Description->Parameters->GetTextureDefaultValues().size())
		return GetBuiltin3DTexture<IsRenderProxy>(mInformation.Description->Parameters->GetTextureDefaultValues()[index]);

	return TextureType();
}

template <bool IsRenderProxy>
TShared<SamplerState> TShader<IsRenderProxy>::GetDefaultSampler(u32 index) const
{
	if (index < (u32)mInformation.Description->Parameters->GetSamplerDefaultValues().size())
	{
		const TShared<GpuDevice> gpuDevice = GetApplication().GetPrimaryGpuDevice();
		if (!B3D_ENSURE(gpuDevice))
			return nullptr;

		return gpuDevice->CreateSamplerState(mInformation.Description->Parameters->GetSamplerDefaultValues()[index]);
	}

	return TShared<SamplerState>();
}

template <bool IsRenderProxy>
const u8* TShader<IsRenderProxy>::GetDefaultValue(u32 index) const
{
	if(index < (u32)mInformation.Description->Parameters->GetDataDefaultValues().size())
		return &mInformation.Description->Parameters->GetDataDefaultValues()[index];

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

	render::Shader* renderProxy = new(B3DAllocate<render::Shader>()) render::Shader(mName, ShaderCreateInformation::ConvertToRenderProxy(mInformation), mShaderId);
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

namespace
{
	/**
	 * Builds a shader create information from a PrecompiledShaderData snapshot. Copies the shared description and
	 * recreates the (uncompiled) variation shells for each variation/language combination.
	 * Variations are created with no owner; the caller assigns the owner once the shader exists.
	 */
	template <bool IsRenderProxy>
	CoreVariantType<ShaderCreateInformation, IsRenderProxy> CreateShaderCreateInformationFromPrecompiledData(const PrecompiledShaderData& data, const Vector<String>& languages)
	{
		using VariationType = CoreVariantType<Variation, IsRenderProxy>;

		CoreVariantType<ShaderCreateInformation, IsRenderProxy> createInformation(data.Description);
		createInformation.CompilerMetaData = data.CompilerMetaData;

		if(data.CompilerMetaData != nullptr)
		{
			for(const auto& variationParameters : data.CompilerMetaData->Variations)
			{
				for(const auto& language : languages)
					createInformation.Variations.push_back(VariationType::Create({}, language, variationParameters));
			}
		}

		return createInformation;
	}
}

HShader Shader::Create(const String& name, const ShaderCreateInformation& createInformation)
{
	TShared<Shader> newShader = CreateShared(name, createInformation);

	return B3DStaticResourceCast<Shader>(GetResources().CreateResourceHandle(newShader));
}

TShared<Shader> Shader::Create(const PrecompiledShaderData& data, const Vector<String>& languages)
{
	ShaderCreateInformation createInformation = CreateShaderCreateInformationFromPrecompiledData<false>(data, languages);
	TShared<Shader> shader = CreateShared(data.Name, createInformation);

	for(const auto& variation : createInformation.Variations)
		variation->SetOwner(shader);

	return shader;
}

TShared<PrecompiledShaderData> Shader::GetPrecompiledData() const
{
	TShared<PrecompiledShaderData> data = B3DMakeShared<PrecompiledShaderData>();
	FillPrecompiledData(*data);
	data->Name = GetShaderName();

	return data;
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

TShared<Shader> Shader::Create(const PrecompiledShaderData& data, const Vector<String>& languages)
{
	ShaderCreateInformation createInformation = CreateShaderCreateInformationFromPrecompiledData<true>(data, languages);
	TShared<Shader> shader = Create(data.Name, createInformation);

	for(const auto& variation : createInformation.Variations)
		variation->SetOwner(shader);

	return shader;
}

TShared<PrecompiledShaderData> Shader::GetPrecompiledData() const
{
	TShared<PrecompiledShaderData> data = B3DMakeShared<PrecompiledShaderData>();
	FillPrecompiledData(*data);
	data->Name = GetShaderName();

	return data;
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
