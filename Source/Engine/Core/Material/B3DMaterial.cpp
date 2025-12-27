//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Material/B3DMaterial.h"
#include "Material/B3DShader.h"
#include "Material/B3DVariation.h"
#include "Material/B3DPass.h"
#include "RTTI/B3DMaterialRTTI.h"
#include "Resources/B3DResources.h"
#include "Math/B3DMatrixNxM.h"
#include "Math/B3DVector3I.h"
#include "Math/B3DVector4I.h"
#include "Material/B3DMaterialParameters.h"
#include "Material/B3DMaterialParameterAdapter.h"
#include "Animation/B3DAnimationCurve.h"
#include "CoreObject/B3DCoreObjectSync.h"
#include "RTTI/B3DShaderVariationRTTI.h"
#include "Serialization/B3DBinarySerializer.h"
#include "FileSystem/B3DDataStream.h"

using namespace b3d;

enum MaterialLoadFlags
{
	Load_None = 0,
	Load_Shader = 1,
	Load_All = 2
};

template <class T>
bool IsShaderValid(const T& shader)
{
	return false;
}

template <>
bool IsShaderValid(const HShader& shader)
{
	return shader.IsLoaded();
}

template <>
bool IsShaderValid(const SPtr<render::Shader>& shader)
{
	return shader != nullptr;
}

template <bool IsRenderProxy>
SPtr<CoreVariantType<Material, IsRenderProxy>> GetMaterialPtr(const TMaterial<IsRenderProxy>* material)
{
	return std::static_pointer_cast<CoreVariantType<Material, IsRenderProxy>>(
		static_cast<const CoreVariantType<Material, IsRenderProxy>*>(material)->GetShared());
}

template <bool IsRenderProxy>
SPtr<typename TMaterial<IsRenderProxy>::MaterialParameterAdapterType> TMaterial<IsRenderProxy>::CreateParameterAdapter(u32 variationIndex)
{
	if(variationIndex >= (u32)mVariations.size())
		return nullptr;

	SPtr<VariationType> variation = mVariations[variationIndex];
	return B3DMakeShared<MaterialParameterAdapterType>(variation, mShader, mParameters);
}

template <bool IsRenderProxy>
u32 TMaterial<IsRenderProxy>::FindVariation(const FindVariationInformation& information) const
{
	u32 bestVariationIndex = ~0u;
	u32 bestVariationScore = std::numeric_limits<u32>::max();

	for(u32 variationIndex = 0; variationIndex < (u32)mVariations.size(); variationIndex++)
	{
		// Make sure tags match
		bool foundMatch = true;

		const ShaderVariationParameters& currentVariationParameters = mVariations[variationIndex]->GetVariationParameters();
		const auto& currentVariationParameterList = currentVariationParameters.GetParameterList();
		const auto& internalVariationParameterList = mVariationParameters.GetParameterList();

		u32 matchedSearchParameterCount = 0;
		u32 matchedInternalParameterCount = 0;
		u32 currentScore = 0;
		for(auto& variationParameter : currentVariationParameterList)
		{
			enum SearchResult
			{
				NoParam,
				NotMatching,
				Matching
			};

			SearchResult matchesSearch = NoParam;
			if(information.VariationParameters)
			{
				const auto findSearch = information.VariationParameters->FindParameter(variationParameter.Name);
				if(findSearch != nullptr)
					matchesSearch = findSearch->SignedInteger == variationParameter.SignedInteger ? Matching : NotMatching;
			}

			SearchResult matchesInternal = NoParam;
			const auto findInternal = mVariationParameters.FindParameter(variationParameter.Name);
			if(findInternal != nullptr)
				matchesInternal = findInternal->SignedInteger == variationParameter.SignedInteger ? Matching : NotMatching;

			switch(matchesSearch)
			{
			default:
			case NoParam:
				switch(matchesInternal)
				{
				default:
				case NoParam:
					// When it comes to parameters not part of the search, prefer those with 0 default value
					currentScore += variationParameter.UnsignedInteger;
					break;
				case NotMatching:
					foundMatch = false;
					break;
				case Matching:
					matchedInternalParameterCount++;
					break;
				}
				break;
			case NotMatching:
				if(information.Override)
				{
					foundMatch = false;
					break;
				}

				switch(matchesInternal)
				{
				default:
				case NoParam:
					foundMatch = false;
					break;
				case NotMatching:
					foundMatch = false;
					break;
				case Matching:
					matchedSearchParameterCount++;
					matchedInternalParameterCount++;
					break;
				}
				break;
			case Matching:
				switch(matchesInternal)
				{
				default:
				case NoParam:
					matchedSearchParameterCount++;
					break;
				case NotMatching:
					if(information.Override)
					{
						matchedSearchParameterCount++;
						matchedInternalParameterCount++;
					}
					else
						foundMatch = false;
					break;
				case Matching:
					matchedSearchParameterCount++;
					matchedInternalParameterCount++;
					break;
				}
				break;
			}

			if(!foundMatch)
				break;
		}

		if(!foundMatch)
			continue;

		if(information.VariationParameters)
		{
			const auto& searchVarParams = information.VariationParameters->GetParameterList();
			if(matchedSearchParameterCount != (u32)searchVarParams.size())
				continue;
		}

		if(matchedInternalParameterCount != (u32)internalVariationParameterList.size())
			continue;

		if(currentScore < bestVariationScore)
		{
			bestVariationIndex = variationIndex;
			bestVariationScore = currentScore;
		}
	}

	return bestVariationIndex;
}

template <bool IsRenderProxy>
u32 TMaterial<IsRenderProxy>::GetDefaultVariation() const
{
	u32 bestVariationIndex = 0;
	u32 bestVariationScore = std::numeric_limits<u32>::max();

	for(u32 variationIndex = 0; variationIndex < (u32)mVariations.size(); variationIndex++)
	{
		const ShaderVariationParameters& currentVariationParameters = mVariations[variationIndex]->GetVariationParameters();
		const auto& currentVariationParameterList = currentVariationParameters.GetParameterList();
		const auto& internalVariationParameterList = mVariationParameters.GetParameterList();

		bool foundMatch = true;
		u32 matchedParameterCount = 0;
		u32 currentScore = 0;
		for(auto& param : currentVariationParameterList)
		{
			enum SearchResult
			{
				NoParam,
				NotMatching,
				Matching
			};

			SearchResult matches = NoParam;
			const auto findInternal = mVariationParameters.FindParameter(param.Name);
			if(findInternal != nullptr)
				matches = findInternal->SignedInteger == param.SignedInteger ? Matching : NotMatching;

			switch(matches)
			{
			default:
			case NoParam:
				// When it comes to parameters not part of the search, prefer those with 0 default value
				currentScore += param.UnsignedInteger;
				break;
			case NotMatching:
				foundMatch = false;
				break;
			case Matching:
				matchedParameterCount++;
				break;
			}

			if(!foundMatch)
				break;
		}

		if(!foundMatch)
			continue;

		if(matchedParameterCount != (u32)internalVariationParameterList.size())
			continue;

		if(currentScore < bestVariationScore)
		{
			bestVariationIndex = variationIndex;
			bestVariationScore = currentScore;
		}
	}

	return bestVariationIndex;
}

template <bool IsRenderProxy>
u32 TMaterial<IsRenderProxy>::GetPassCount(u32 variationIndex) const
{
	if(mShader == nullptr)
		return 0;

	if(variationIndex >= (u32)mVariations.size())
		return 0;

	return mVariations[variationIndex]->GetPassCount();
}

template <bool IsRenderProxy>
SPtr<typename TMaterial<IsRenderProxy>::PassType> TMaterial<IsRenderProxy>::GetPass(u32 passIndex, u32 variationIndex) const
{
	if(mShader == nullptr)
		return nullptr;

	if(variationIndex >= (u32)mVariations.size())
		return nullptr;

	if(passIndex < 0 || passIndex >= mVariations[variationIndex]->GetPassCount())
		return nullptr;

	return mVariations[variationIndex]->GetPass(passIndex);
}

template <bool IsRenderProxy>
TMaterialParameterStruct<IsRenderProxy> TMaterial<IsRenderProxy>::GetParamStruct(const String& name) const
{
	ThrowIfNotInitialized();

	return TMaterialParameterStruct<IsRenderProxy>(name, GetMaterialPtr(this));
}

template <bool IsRenderProxy>
TMaterialParameterColorGradient<IsRenderProxy> TMaterial<IsRenderProxy>::GetParamColorGradient(const String& name) const
{
	ThrowIfNotInitialized();

	return TMaterialParameterColorGradient<IsRenderProxy>(name, GetMaterialPtr(this));
}

template <bool IsRenderProxy>
TMaterialParameterCurve<float, IsRenderProxy> TMaterial<IsRenderProxy>::GetParamFloatCurve(const String& name) const
{
	ThrowIfNotInitialized();

	return TMaterialParameterCurve<float, IsRenderProxy>(name, GetMaterialPtr(this));
}

template <bool IsRenderProxy>
TMaterialParameterSampledTexture<IsRenderProxy> TMaterial<IsRenderProxy>::GetParamTexture(const String& name) const
{
	ThrowIfNotInitialized();

	return TMaterialParameterSampledTexture<IsRenderProxy>(name, GetMaterialPtr(this));
}

template <bool IsRenderProxy>
TMaterialParamSpriteImage<IsRenderProxy> TMaterial<IsRenderProxy>::GetParamSpriteImage(const String& name) const
{
	ThrowIfNotInitialized();

	return TMaterialParamSpriteImage<IsRenderProxy>(name, GetMaterialPtr(this));
}

template <bool IsRenderProxy>
TMaterialParameterStorageTexture<IsRenderProxy> TMaterial<IsRenderProxy>::GetParamLoadStoreTexture(const String& name) const
{
	ThrowIfNotInitialized();

	return TMaterialParameterStorageTexture<IsRenderProxy>(name, GetMaterialPtr(this));
}

template <bool IsRenderProxy>
TMaterialParameterBuffer<IsRenderProxy> TMaterial<IsRenderProxy>::GetParamBuffer(const String& name) const
{
	ThrowIfNotInitialized();

	return TMaterialParameterBuffer<IsRenderProxy>(name, GetMaterialPtr(this));
}

template <bool IsRenderProxy>
TMaterialParameterSampler<IsRenderProxy> TMaterial<IsRenderProxy>::GetParamSamplerState(const String& name) const
{
	ThrowIfNotInitialized();

	return TMaterialParameterSampler<IsRenderProxy>(name, GetMaterialPtr(this));
}

template <bool IsRenderProxy>
bool TMaterial<IsRenderProxy>::IsAnimated(const String& name, u32 arrayIdx)
{
	return mParameters->IsAnimated(name, arrayIdx);
}

template <bool IsRenderProxy>
void TMaterial<IsRenderProxy>::InitializeVariations()
{
	mVariations.clear();

	if(IsShaderValid(mShader))
	{
		mParameters = B3DMakeShared<MaterialParametersType>(mShader);
		mVariations = mShader->GetCompatibleTechniques();

		if(mVariations.empty())
			return;

		InitializeDefaultParameters();
	}
	else
		mParameters = nullptr;

	MarkDependenciesDirtyInternal();
}

template <bool IsRenderProxy>
template <typename T>
void TMaterial<IsRenderProxy>::SetParamValue(const String& name, u8* buffer, u32 numElements)
{
	TMaterialParameterPrimitive<T, IsRenderProxy> param;
	GetParam(name, param);

	T* ptr = (T*)buffer;
	for(u32 i = 0; i < numElements; i++)
		param.Set(ptr[i], i);
}

template <bool IsRenderProxy>
void TMaterial<IsRenderProxy>::InitializeDefaultParameters()
{
	const Map<String, ShaderDataParameterInformation>& dataParams = mShader->GetDataParams();
	for(auto& paramData : dataParams)
	{
		if(paramData.second.DefaultValueIndex == (u32)-1)
			continue;

		u8* buffer = (u8*)mShader->GetDefaultValue(paramData.second.DefaultValueIndex);
		if(buffer == nullptr)
			continue;

		switch(paramData.second.Type)
		{
		case GPDT_FLOAT1:
			SetParamValue<float>(paramData.first, buffer, paramData.second.ArraySize);
			break;
		case GPDT_FLOAT2:
			SetParamValue<Vector2>(paramData.first, buffer, paramData.second.ArraySize);
			break;
		case GPDT_FLOAT3:
			SetParamValue<Vector3>(paramData.first, buffer, paramData.second.ArraySize);
			break;
		case GPDT_FLOAT4:
			SetParamValue<Vector4>(paramData.first, buffer, paramData.second.ArraySize);
			break;
		case GPDT_MATRIX_2X2:
			SetParamValue<Matrix2>(paramData.first, buffer, paramData.second.ArraySize);
			break;
		case GPDT_MATRIX_2X3:
			SetParamValue<Matrix2x3>(paramData.first, buffer, paramData.second.ArraySize);
			break;
		case GPDT_MATRIX_2X4:
			SetParamValue<Matrix2x4>(paramData.first, buffer, paramData.second.ArraySize);
			break;
		case GPDT_MATRIX_3X2:
			SetParamValue<Matrix3x2>(paramData.first, buffer, paramData.second.ArraySize);
			break;
		case GPDT_MATRIX_3X3:
			SetParamValue<Matrix3>(paramData.first, buffer, paramData.second.ArraySize);
			break;
		case GPDT_MATRIX_3X4:
			SetParamValue<Matrix3x4>(paramData.first, buffer, paramData.second.ArraySize);
			break;
		case GPDT_MATRIX_4X2:
			SetParamValue<Matrix4x2>(paramData.first, buffer, paramData.second.ArraySize);
			break;
		case GPDT_MATRIX_4X3:
			SetParamValue<Matrix4x3>(paramData.first, buffer, paramData.second.ArraySize);
			break;
		case GPDT_MATRIX_4X4:
			SetParamValue<Matrix4>(paramData.first, buffer, paramData.second.ArraySize);
			break;
		case GPDT_INT1:
			SetParamValue<i32>(paramData.first, buffer, paramData.second.ArraySize);
			break;
		case GPDT_INT2:
			SetParamValue<Vector2I>(paramData.first, buffer, paramData.second.ArraySize);
			break;
		case GPDT_INT3:
			SetParamValue<Vector3I>(paramData.first, buffer, paramData.second.ArraySize);
			break;
		case GPDT_INT4:
			SetParamValue<Vector4I>(paramData.first, buffer, paramData.second.ArraySize);
			break;
		case GPDT_UINT1:
			SetParamValue<u32>(paramData.first, buffer, paramData.second.ArraySize);
			break;
		case GPDT_UINT2:
			SetParamValue<Vector2UI>(paramData.first, buffer, paramData.second.ArraySize);
			break;
		case GPDT_UINT3:
			SetParamValue<Vector3UI>(paramData.first, buffer, paramData.second.ArraySize);
			break;
		case GPDT_UINT4:
			SetParamValue<Vector4UI>(paramData.first, buffer, paramData.second.ArraySize);
			break;
		case GPDT_BOOL:
			SetParamValue<int>(paramData.first, buffer, paramData.second.ArraySize);
			break;
		case GPDT_COLOR:
			SetParamValue<Color>(paramData.first, buffer, paramData.second.ArraySize);
			break;
		case GPDT_STRUCT:
			{
				TMaterialParameterStruct<IsRenderProxy> param = GetParamStruct(paramData.first);

				u32 elementSizeBytes = paramData.second.ElementSize * sizeof(u32);
				u8* ptr = buffer;
				for(u32 i = 0; i < paramData.second.ArraySize; i++)
				{
					param.Set(ptr, elementSizeBytes, i);
					ptr += elementSizeBytes;
				}
			}
			break;
		default:
			break;
		}
	}

	const Map<String, ShaderObjectParameterInformation>& textureParams = mShader->GetTextureParams();
	for(auto& param : textureParams)
	{
		if(param.second.DefaultValueIndex == (u32)-1)
			continue;

		const TextureType texture = param.second.Type == GPOT_TEXTURE3D ? mShader->GetDefault3DTexture(param.second.DefaultValueIndex) : mShader->GetDefault2DTexture(param.second.DefaultValueIndex);
		GetParamTexture(param.first).Set(texture);
	}

	const Map<String, ShaderObjectParameterInformation>& samplerParams = mShader->GetSamplerParams();
	for(auto& param : samplerParams)
	{
		if(param.second.DefaultValueIndex == (u32)-1)
			continue;

		SPtr<SamplerState> defaultSampler = mShader->GetDefaultSampler(param.second.DefaultValueIndex);
		GetParamSamplerState(param.first).Set(defaultSampler);
	}
}

template <bool IsRenderProxy>
template <typename T>
void TMaterial<IsRenderProxy>::GetParam(const String& name, TMaterialParameterPrimitive<T, IsRenderProxy>& output) const
{
	ThrowIfNotInitialized();

	output = TMaterialParameterPrimitive<T, IsRenderProxy>(name, GetMaterialPtr(this));
}

template <bool IsRenderProxy>
void TMaterial<IsRenderProxy>::ThrowIfNotInitialized() const
{
	if(mShader == nullptr)
		B3D_EXCEPT(InternalErrorException, "Material does not have shader set.");

	if(mVariations.empty())
		B3D_EXCEPT(InternalErrorException, "Shader does not contain a supported variation.");
}

template class TMaterial<false>;
template class TMaterial<true>;

template B3D_EXPORT void TMaterial<false>::GetParam(const String&, TMaterialParameterPrimitive<float, false>&) const;
template B3D_EXPORT void TMaterial<false>::GetParam(const String&, TMaterialParameterPrimitive<int, false>&) const;
template B3D_EXPORT void TMaterial<false>::GetParam(const String&, TMaterialParameterPrimitive<Color, false>&) const;
template B3D_EXPORT void TMaterial<false>::GetParam(const String&, TMaterialParameterPrimitive<Vector2, false>&) const;
template B3D_EXPORT void TMaterial<false>::GetParam(const String&, TMaterialParameterPrimitive<Vector3, false>&) const;
template B3D_EXPORT void TMaterial<false>::GetParam(const String&, TMaterialParameterPrimitive<Vector4, false>&) const;
template B3D_EXPORT void TMaterial<false>::GetParam(const String&, TMaterialParameterPrimitive<Vector2I, false>&) const;
template B3D_EXPORT void TMaterial<false>::GetParam(const String&, TMaterialParameterPrimitive<Vector3I, false>&) const;
template B3D_EXPORT void TMaterial<false>::GetParam(const String&, TMaterialParameterPrimitive<Vector4I, false>&) const;
template B3D_EXPORT void TMaterial<false>::GetParam(const String&, TMaterialParameterPrimitive<Matrix2, false>&) const;
template B3D_EXPORT void TMaterial<false>::GetParam(const String&, TMaterialParameterPrimitive<Matrix2x3, false>&) const;
template B3D_EXPORT void TMaterial<false>::GetParam(const String&, TMaterialParameterPrimitive<Matrix2x4, false>&) const;
template B3D_EXPORT void TMaterial<false>::GetParam(const String&, TMaterialParameterPrimitive<Matrix3, false>&) const;
template B3D_EXPORT void TMaterial<false>::GetParam(const String&, TMaterialParameterPrimitive<Matrix3x2, false>&) const;
template B3D_EXPORT void TMaterial<false>::GetParam(const String&, TMaterialParameterPrimitive<Matrix3x4, false>&) const;
template B3D_EXPORT void TMaterial<false>::GetParam(const String&, TMaterialParameterPrimitive<Matrix4, false>&) const;
template B3D_EXPORT void TMaterial<false>::GetParam(const String&, TMaterialParameterPrimitive<Matrix4x2, false>&) const;
template B3D_EXPORT void TMaterial<false>::GetParam(const String&, TMaterialParameterPrimitive<Matrix4x3, false>&) const;

template B3D_EXPORT void TMaterial<true>::GetParam(const String&, TMaterialParameterPrimitive<float, true>&) const;
template B3D_EXPORT void TMaterial<true>::GetParam(const String&, TMaterialParameterPrimitive<int, true>&) const;
template B3D_EXPORT void TMaterial<true>::GetParam(const String&, TMaterialParameterPrimitive<Color, true>&) const;
template B3D_EXPORT void TMaterial<true>::GetParam(const String&, TMaterialParameterPrimitive<Vector2, true>&) const;
template B3D_EXPORT void TMaterial<true>::GetParam(const String&, TMaterialParameterPrimitive<Vector3, true>&) const;
template B3D_EXPORT void TMaterial<true>::GetParam(const String&, TMaterialParameterPrimitive<Vector4, true>&) const;
template B3D_EXPORT void TMaterial<true>::GetParam(const String&, TMaterialParameterPrimitive<Vector2I, true>&) const;
template B3D_EXPORT void TMaterial<true>::GetParam(const String&, TMaterialParameterPrimitive<Vector3I, true>&) const;
template B3D_EXPORT void TMaterial<true>::GetParam(const String&, TMaterialParameterPrimitive<Vector4I, true>&) const;
template B3D_EXPORT void TMaterial<true>::GetParam(const String&, TMaterialParameterPrimitive<Matrix2, true>&) const;
template B3D_EXPORT void TMaterial<true>::GetParam(const String&, TMaterialParameterPrimitive<Matrix2x3, true>&) const;
template B3D_EXPORT void TMaterial<true>::GetParam(const String&, TMaterialParameterPrimitive<Matrix2x4, true>&) const;
template B3D_EXPORT void TMaterial<true>::GetParam(const String&, TMaterialParameterPrimitive<Matrix3, true>&) const;
template B3D_EXPORT void TMaterial<true>::GetParam(const String&, TMaterialParameterPrimitive<Matrix3x2, true>&) const;
template B3D_EXPORT void TMaterial<true>::GetParam(const String&, TMaterialParameterPrimitive<Matrix3x4, true>&) const;
template B3D_EXPORT void TMaterial<true>::GetParam(const String&, TMaterialParameterPrimitive<Matrix4, true>&) const;
template B3D_EXPORT void TMaterial<true>::GetParam(const String&, TMaterialParameterPrimitive<Matrix4x2, true>&) const;
template B3D_EXPORT void TMaterial<true>::GetParam(const String&, TMaterialParameterPrimitive<Matrix4x3, true>&) const;

Material::Material()
	: mLoadFlags(Load_None)
{}

Material::Material(const HShader& shader, const ShaderVariationParameters& variation)
	: mLoadFlags(Load_None)
{
	mShader = shader;
	mVariationParameters = variation;
}

void Material::Initialize()
{
	AddResourceDependency(mShader);
	MarkResourcesDirtyInternal();
	InitializeIfLoaded();

	Resource::Initialize();
}

void Material::SetShader(const HShader& shader)
{
	if(mShader == shader)
		return;

	RemoveResourceDependency(mShader);
	mShader = shader;
	AddResourceDependency(mShader);

	mVariations.clear();
	mLoadFlags = Load_None;

	// Make sure to clear params, because the default behaviour is to re-apply them (which won't work due to changed
	// shader)
	mParameters = nullptr;

	MarkResourcesDirtyInternal();

	InitializeIfLoaded();
}

void Material::SetVariation(const ShaderVariationParameters& variation)
{
	mVariationParameters = variation;
	MarkRenderProxyDataDirty();
}

void Material::MarkRenderProxyDataDirtyInternal(MaterialDirtyFlags flags)
{
	MarkRenderProxyDataDirty((u32)flags);
}

void Material::MarkDependenciesDirtyInternal()
{
	MarkDependenciesDirty();
}

void Material::MarkResourcesDirtyInternal()
{
	MarkListenerResourcesDirty();
}

SPtr<render::RenderProxy> Material::CreateRenderProxy() const
{
	render::Material* renderProxy = nullptr;

	SPtr<render::Shader> shader;
	if(mShader.IsLoaded())
	{
		shader = B3DGetRenderProxy(mShader);

		Vector<SPtr<render::Variation>> variations(mVariations.size());
		for(u32 i = 0; i < (u32)mVariations.size(); i++)
			variations[i] = B3DGetRenderProxy(mVariations[i]);

		SPtr<render::MaterialParameters> materialParams = B3DMakeShared<render::MaterialParameters>(shader, mParameters);

		renderProxy = new(B3DAllocate<render::Material>()) render::Material(shader, variations, materialParams, mVariationParameters);
	}

	if(renderProxy == nullptr)
		renderProxy = new(B3DAllocate<render::Material>()) render::Material(shader, mVariationParameters);

	SPtr<render::Material> renderProxyShared = B3DMakeSharedFromExisting<render::Material>(renderProxy);
	renderProxyShared->SetShared(renderProxyShared);

	return renderProxyShared;
}

namespace b3d
{
	B3D_SYNC_BLOCK_BEGIN(Material, SyncPacket)
		B3D_SYNC_BLOCK_ENTRY(mShader)
		B3D_SYNC_BLOCK_ENTRY(mVariations)
		B3D_SYNC_BLOCK_ENTRY(mVariationParameters)
		B3D_SYNC_BLOCK_ENTRY_CUSTOM(bool, IsSyncingAllParameters)
		B3D_SYNC_BLOCK_ENTRY_CUSTOM(MaterialParameters::SyncPacket*, DirtyMaterialParametersPacket)
	B3D_SYNC_BLOCK_END
}

RenderProxySyncPacket* Material::CreateRenderProxySyncPacket(FrameAllocator& allocator, u32 flags)
{
	SyncPacket* const syncPacket = allocator.Construct<SyncPacket>(*this, allocator, flags);

	syncPacket->IsSyncingAllParameters = (GetRenderProxyDirtyFlags() & ~((u32)MaterialDirtyFlags::Parameter)) != 0;
	syncPacket->DirtyMaterialParametersPacket = nullptr;

	if(mParameters != nullptr)
		syncPacket->DirtyMaterialParametersPacket = mParameters->CreateSyncPacket(allocator, syncPacket->IsSyncingAllParameters);
	
	return syncPacket;
}

void Material::GetCoreDependencies(Vector<CoreObject*>& dependencies)
{
	if(mShader.IsLoaded())
		dependencies.push_back(mShader.Get());

	if(mParameters != nullptr)
		mParameters->GetCoreObjectDependencies(dependencies);
}

void Material::GetListenerResources(Vector<HResource>& resources)
{
	if(mShader != nullptr)
		resources.push_back(mShader);

	if(mParameters != nullptr)
		mParameters->GetResourceDependencies(resources);
}

void Material::InitializeIfLoaded()
{
	if(AreDependenciesLoaded())
	{
		if(mLoadFlags != Load_All)
		{
			mLoadFlags = Load_All;

			// Shader about to change, so save parameters, rebuild material and restore parameters
			SPtr<MaterialParameters> oldParams = mParameters;

			InitializeVariations();
			MarkRenderProxyDataDirty();

			if(mVariations.empty()) // Wasn't initialized
				return;

			if(oldParams)
				SetParams(oldParams);
		}
	}
	else
	{
		if(mShader.IsLoaded() && mLoadFlags == Load_None)
		{
			mLoadFlags = Load_Shader;
			MarkListenerResourcesDirty(); // Need to register resources dependent on shader now
		}
	}
}

void Material::NotifyResourceLoaded(const HResource& resource)
{
	// Ready to initialize as soon as shader loads
	if(resource->GetRtti()->GetRttiId() == TID_Shader)
		InitializeIfLoaded();
	else
	{
		// Otherwise just sync changes (most likely just a texture got loaded)
		MarkRenderProxyDataDirtyInternal(MaterialDirtyFlags::ParameterResource);
	}
}

void Material::NotifyResourceChanged(const HResource& resource)
{
	// Need full rebuild if shader changed
	if(resource->GetRtti()->GetRttiId() == TID_Shader)
	{
		mLoadFlags = Load_None;
		InitializeIfLoaded();
	}
	else
	{
		// Otherwise just sync changes (most likely just a texture got reimported)
		MarkRenderProxyDataDirtyInternal(MaterialDirtyFlags::ParameterResource);
	}
}

HMaterial Material::Clone()
{
	SPtr<MemoryDataStream> outputStream = B3DMakeShared<MemoryDataStream>();
	BinarySerializer serializer;

	serializer.Encode(this, outputStream);
	outputStream->Seek(0);
	SPtr<Material> cloneObj = std::static_pointer_cast<Material>(serializer.Decode(outputStream, (u32)outputStream->Size()));

	return B3DStaticResourceCast<Material>(GetResources().CreateResourceHandle(cloneObj));
}

template <class T>
void CopyParam(const SPtr<MaterialParameters>& from, Material* to, const String& name, const MaterialParameters::ParamData& paramRef, u32 arraySize)
{
	TMaterialParameterPrimitive<T, false> param;
	to->GetParam(name, param);

	T paramData;
	for(u32 i = 0; i < arraySize; i++)
	{
		from->GetDataParam(paramRef, i, paramData);
		param.Set(paramData, i);
	}
}

void Material::SetParams(const SPtr<MaterialParameters>& params)
{
	if(params == nullptr)
		return;

	std::function<
		void(const SPtr<MaterialParameters>&, Material*, const String&, const MaterialParameters::ParamData&, u32)>
		copyParamLookup[GPDT_COUNT];

	copyParamLookup[GPDT_FLOAT1] = &CopyParam<float>;
	copyParamLookup[GPDT_FLOAT2] = &CopyParam<Vector2>;
	copyParamLookup[GPDT_FLOAT3] = &CopyParam<Vector3>;
	copyParamLookup[GPDT_FLOAT4] = &CopyParam<Vector4>;

	copyParamLookup[GPDT_INT1] = &CopyParam<i32>;
	copyParamLookup[GPDT_INT2] = &CopyParam<Vector2I>;
	copyParamLookup[GPDT_INT3] = &CopyParam<Vector3I>;
	copyParamLookup[GPDT_INT4] = &CopyParam<Vector4I>;

	copyParamLookup[GPDT_UINT1] = &CopyParam<u32>;
	copyParamLookup[GPDT_UINT2] = &CopyParam<Vector2UI>;
	copyParamLookup[GPDT_UINT3] = &CopyParam<Vector3UI>;
	copyParamLookup[GPDT_UINT4] = &CopyParam<Vector4UI>;

	copyParamLookup[GPDT_MATRIX_2X2] = &CopyParam<Matrix2>;
	copyParamLookup[GPDT_MATRIX_2X3] = &CopyParam<Matrix2x3>;
	copyParamLookup[GPDT_MATRIX_2X4] = &CopyParam<Matrix2x4>;

	copyParamLookup[GPDT_MATRIX_3X3] = &CopyParam<Matrix3>;
	copyParamLookup[GPDT_MATRIX_3X2] = &CopyParam<Matrix3x2>;
	copyParamLookup[GPDT_MATRIX_3X4] = &CopyParam<Matrix3x4>;

	copyParamLookup[GPDT_MATRIX_4X4] = &CopyParam<Matrix4>;
	copyParamLookup[GPDT_MATRIX_4X2] = &CopyParam<Matrix4x2>;
	copyParamLookup[GPDT_MATRIX_4X3] = &CopyParam<Matrix4x3>;

	copyParamLookup[GPDT_BOOL] = &CopyParam<int>;
	copyParamLookup[GPDT_COLOR] = &CopyParam<Color>;

	auto& dataParams = mShader->GetDataParams();
	for(auto& param : dataParams)
	{
		u32 arraySize = param.second.ArraySize > 1 ? param.second.ArraySize : 1;

		const MaterialParameters::ParamData* paramData = nullptr;
		auto result = params->GetParamData(param.first, MaterialParameters::ParamType::Data, param.second.Type, 0, &paramData);

		if(result != MaterialParameters::GetParamResult::Success)
			continue;

		u32 elemsToCopy = std::min(arraySize, paramData->ArraySize);

		auto& copyFunction = copyParamLookup[param.second.Type];
		if(copyFunction != nullptr)
			copyFunction(params, this, param.first, *paramData, elemsToCopy);
		else
		{
			if(param.second.Type == GPDT_STRUCT)
			{
				TMaterialParameterStruct<false> curParam = GetParamStruct(param.first);

				u32 structSize = params->GetStructSize(*paramData);
				if(param.second.ElementSize != structSize)
					continue;

				u8* structData = (u8*)B3DStackAllocate(structSize);
				for(u32 i = 0; i < elemsToCopy; i++)
				{
					params->GetStructData(*paramData, structData, structSize, i);
					curParam.Set(structData, structSize, i);
				}

				B3DStackFree(structData);
			}
		}

		for(u32 i = 0; i < arraySize; i++)
		{
			const bool isAnimated = params->IsAnimated(*paramData, i);
			if(!isAnimated)
				continue;

			if(param.second.Type == GPDT_FLOAT1)
			{
				TMaterialParameterCurve<float, false> curParam = GetParamFloatCurve(param.first);
				curParam.Set(params->GetCurveParam<float>(*paramData, i), i);
			}
			else if(param.second.Type == GPDT_COLOR)
			{
				TMaterialParameterColorGradient<false> curParam = GetParamColorGradient(param.first);
				curParam.Set(params->GetColorGradientParam(*paramData, i), i);
			}
		}
	}

	auto& textureParams = mShader->GetTextureParams();
	for(auto& param : textureParams)
	{
		const MaterialParameters::ParamData* paramData = nullptr;
		auto result = params->GetParamData(param.first, MaterialParameters::ParamType::Texture, GPDT_UNKNOWN, 0, &paramData);

		if(result != MaterialParameters::GetParamResult::Success)
			continue;

		MateralParamTextureType texType = params->GetTextureType(*paramData);
		switch(texType)
		{
		default:
		case MateralParamTextureType::Normal:
			{
				TMaterialParameterSampledTexture<false> curParam = GetParamTexture(param.first);

				HTexture texture;
				TextureSurface surface;
				params->GetTexture(*paramData, texture, surface);
				curParam.Set(texture);
			}
			break;
		case MateralParamTextureType::LoadStore:
			{
				TMaterialParameterStorageTexture<false> curParam = GetParamLoadStoreTexture(param.first);

				HTexture texture;
				TextureSurface surface;
				params->GetStorageTexture(*paramData, texture, surface);
				curParam.Set(texture, surface);
			}
			break;
		case MateralParamTextureType::Sprite:
			{
				TMaterialParamSpriteImage<false> curParam = GetParamSpriteImage(param.first);

				HSpriteImage image;
				params->GetSpriteImage(*paramData, image);
				curParam.Set(image);
			}
			break;
		}
	}

	auto& bufferParams = mShader->GetBufferParams();
	for(auto& param : bufferParams)
	{
		const MaterialParameters::ParamData* paramData = nullptr;
		auto result = params->GetParamData(param.first, MaterialParameters::ParamType::Buffer, GPDT_UNKNOWN, 0, &paramData);

		if(result != MaterialParameters::GetParamResult::Success)
			continue;

		TMaterialParameterBuffer<false> curParam = GetParamBuffer(param.first);

		SPtr<GpuBuffer> buffer;
		params->GetBuffer(*paramData, buffer);
		curParam.Set(buffer);
	}

	auto& samplerParams = mShader->GetSamplerParams();
	for(auto& param : samplerParams)
	{
		const MaterialParameters::ParamData* paramData = nullptr;
		auto result = params->GetParamData(param.first, MaterialParameters::ParamType::Sampler, GPDT_UNKNOWN, 0, &paramData);

		if(result != MaterialParameters::GetParamResult::Success)
			continue;

		TMaterialParameterSampler<false> curParam = GetParamSamplerState(param.first);

		SPtr<SamplerState> samplerState;
		params->GetSamplerState(*paramData, samplerState);
		curParam.Set(samplerState);
	}
}

HMaterial Material::Create()
{
	const SPtr<Material> materialPtr = CreateEmpty();
	materialPtr->Initialize();

	return B3DStaticResourceCast<Material>(GetResources().CreateResourceHandle(materialPtr));
}

HMaterial Material::Create(const HShader& shader)
{
	return Create(shader, ShaderVariationParameters::kEmpty);
}

HMaterial Material::Create(const HShader& shader, const ShaderVariationParameters& variation)
{
	SPtr<Material> materialPtr = B3DMakeSharedFromExisting<Material>(new(B3DAllocate<Material>()) Material(shader, variation));
	materialPtr->SetShared(materialPtr);
	materialPtr->Initialize();

	return B3DStaticResourceCast<Material>(GetResources().CreateResourceHandle(materialPtr));
}

SPtr<Material> Material::CreateEmpty()
{
	SPtr<Material> newMat = B3DMakeSharedFromExisting<Material>(new(B3DAllocate<Material>()) Material());
	newMat->SetShared(newMat);

	return newMat;
}

RTTIType* Material::GetRttiStatic()
{
	return MaterialRTTI::Instance();
}

RTTIType* Material::GetRtti() const
{
	return Material::GetRttiStatic();
}

namespace b3d { namespace render
{
Material::Material(const SPtr<Shader>& shader, const ShaderVariationParameters& variation)
{
	mVariationParameters = variation;
	SetShader(shader);
}

Material::Material(const SPtr<Shader>& shader, const Vector<SPtr<Variation>>& variations, const SPtr<MaterialParameters>& materialParameters, const ShaderVariationParameters& variation)
{
	mShader = shader;
	mParameters = materialParameters;
	mVariations = variations;
	mVariationParameters = variation;
}

void Material::SetShader(const SPtr<Shader>& shader)
{
	mShader = shader;

	InitializeVariations();
}

void Material::SetVariation(const ShaderVariationParameters& variation)
{
	mVariationParameters = variation;
}

void Material::SyncFromCoreObject(const CoreSyncData& data, FrameAllocator& allocator)
{
	auto* syncPacket = data.GetSyncPacket<b3d::Material::SyncPacket>();
	if(!syncPacket)
		return;

	u64 initialParamVersion = mParameters != nullptr ? mParameters->GetParamVersion() : 1;
	if(syncPacket->IsSyncingAllParameters)
		mParameters = nullptr;

	const u32 originalVariationIndex = mVariationParameters.GetIndex();
	syncPacket->ApplySyncData(this);

	if(mParameters == nullptr && mShader != nullptr)
		mParameters = B3DMakeShared<MaterialParameters>(mShader, initialParamVersion);

	if(mParameters != nullptr && syncPacket->DirtyMaterialParametersPacket)
		mParameters->ApplyAndDestroySyncPacket(allocator, *syncPacket->DirtyMaterialParametersPacket);

	mVariationParameters.SetIndex(originalVariationIndex);
}

SPtr<Material> Material::Create(const SPtr<Shader>& shader)
{
	Material* material = new(B3DAllocate<Material>()) Material(shader, ShaderVariationParameters::kEmpty);
	SPtr<Material> materialPtr = B3DMakeSharedFromExisting<Material>(material);
	materialPtr->SetShared(materialPtr);
	materialPtr->Initialize();

	return materialPtr;
}
}}
