//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Material/BsMaterial.h"
#include "Material/BsShader.h"
#include "Material/BsTechnique.h"
#include "Material/BsPass.h"
#include "Private/RTTI/BsMaterialRTTI.h"
#include "Resources/BsResources.h"
#include "Math/BsMatrixNxM.h"
#include "Math/BsVector2I.h"
#include "Math/BsVector3I.h"
#include "Math/BsVector4I.h"
#include "Material/BsMaterialParams.h"
#include "Material/BsGpuParamsSet.h"
#include "Animation/BsAnimationCurve.h"
#include "CoreThread/BsCoreObjectSync.h"
#include "Private/RTTI/BsShaderVariationRTTI.h"
#include "Serialization/BsBinarySerializer.h"
#include "FileSystem/BsDataStream.h"

using namespace bs;

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
bool IsShaderValid(const SPtr<ct::Shader>& shader)
{
	return shader != nullptr;
}

template <bool Core>
SPtr<CoreVariantType<Material, Core>> GetMaterialPtr(const TMaterial<Core>* material)
{
	return std::static_pointer_cast<CoreVariantType<Material, Core>>(
		static_cast<const CoreVariantType<Material, Core>*>(material)->GetShared());
}

template <bool Core>
SPtr<typename TMaterial<Core>::GpuParamsSetType> TMaterial<Core>::CreateParamsSet(u32 techniqueIdx)
{
	if(techniqueIdx >= (u32)mTechniques.size())
		return nullptr;

	SPtr<TechniqueType> technique = mTechniques[techniqueIdx];
	return B3DMakeShared<GpuParamsSetType>(technique, mShader, mParams);
}

template <bool Core>
void TMaterial<Core>::UpdateParamsSet(const SPtr<GpuParamsSetType>& paramsSet, float t, bool updateAll)
{
	paramsSet->Update(mParams, t, updateAll);
}

template <bool Core>
u32 TMaterial<Core>::FindTechnique(const FindVariationInformation& desc) const
{
	u32 bestTechniqueIdx = (u32)-1;
	u32 bestTechniqueScore = std::numeric_limits<u32>::max();

	for(u32 i = 0; i < (u32)mTechniques.size(); i++)
	{
		// Make sure tags match
		bool foundMatch = true;

		const ShaderVariationParameters& curVariation = mTechniques[i]->GetVariationParameters();
		const auto& curVarParams = curVariation.GetParams();
		const auto& internalVarParams = mVariation.GetParams();

		u32 numMatchedSearchParams = 0;
		u32 numMatchedInternalParams = 0;
		u32 currentScore = 0;
		for(auto& param : curVarParams)
		{
			enum SearchResult
			{
				NoParam,
				NotMatching,
				Matching
			};

			SearchResult matchesSearch = NoParam;
			if(desc.VariationParameters)
			{
				const auto& searchVarParams = desc.VariationParameters->GetParams();
				const auto findSearch = searchVarParams.find(param.first);
				if(findSearch != searchVarParams.end())
					matchesSearch = findSearch->second.I == param.second.I ? Matching : NotMatching;
			}

			SearchResult matchesInternal = NoParam;
			const auto findInternal = internalVarParams.find(param.first);
			if(findInternal != internalVarParams.end())
				matchesInternal = findInternal->second.I == param.second.I ? Matching : NotMatching;

			switch(matchesSearch)
			{
			default:
			case NoParam:
				switch(matchesInternal)
				{
				default:
				case NoParam:
					// When it comes to parameters not part of the search, prefer those with 0 default value
					currentScore += param.second.Ui;
					break;
				case NotMatching:
					foundMatch = false;
					break;
				case Matching:
					numMatchedInternalParams++;
					break;
				}
				break;
			case NotMatching:
				if(desc.Override)
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
					numMatchedSearchParams++;
					numMatchedInternalParams++;
					break;
				}
				break;
			case Matching:
				switch(matchesInternal)
				{
				default:
				case NoParam:
					numMatchedSearchParams++;
					break;
				case NotMatching:
					if(desc.Override)
					{
						numMatchedSearchParams++;
						numMatchedInternalParams++;
					}
					else
						foundMatch = false;
					break;
				case Matching:
					numMatchedSearchParams++;
					numMatchedInternalParams++;
					break;
				}
				break;
			}

			if(!foundMatch)
				break;
		}

		if(!foundMatch)
			continue;

		if(desc.VariationParameters)
		{
			const auto& searchVarParams = desc.VariationParameters->GetParams();
			if(numMatchedSearchParams != (u32)searchVarParams.size())
				continue;
		}

		if(numMatchedInternalParams != (u32)internalVarParams.size())
			continue;

		if(currentScore < bestTechniqueScore)
		{
			bestTechniqueIdx = i;
			bestTechniqueScore = currentScore;
		}
	}

	return bestTechniqueIdx;
}

template <bool Core>
u32 TMaterial<Core>::GetDefaultTechnique() const
{
	u32 bestTechniqueIdx = 0;
	u32 bestTechniqueScore = std::numeric_limits<u32>::max();

	for(u32 i = 0; i < (u32)mTechniques.size(); i++)
	{
		const ShaderVariationParameters& curVariation = mTechniques[i]->GetVariationParameters();
		const auto& curVarParams = curVariation.GetParams();
		const auto& internalVarParams = mVariation.GetParams();

		bool foundMatch = true;
		u32 numMatchedParams = 0;
		u32 currentScore = 0;
		for(auto& param : curVarParams)
		{
			enum SearchResult
			{
				NoParam,
				NotMatching,
				Matching
			};

			SearchResult matches = NoParam;
			const auto findInternal = internalVarParams.find(param.first);
			if(findInternal != internalVarParams.end())
				matches = findInternal->second.I == param.second.I ? Matching : NotMatching;

			switch(matches)
			{
			default:
			case NoParam:
				// When it comes to parameters not part of the search, prefer those with 0 default value
				currentScore += param.second.Ui;
				break;
			case NotMatching:
				foundMatch = false;
				break;
			case Matching:
				numMatchedParams++;
				break;
			}

			if(!foundMatch)
				break;
		}

		if(!foundMatch)
			continue;

		if(numMatchedParams != (u32)internalVarParams.size())
			continue;

		if(currentScore < bestTechniqueScore)
		{
			bestTechniqueIdx = i;
			bestTechniqueScore = currentScore;
		}
	}

	return bestTechniqueIdx;
}

template <bool Core>
u32 TMaterial<Core>::GetNumPasses(u32 techniqueIdx) const
{
	if(mShader == nullptr)
		return 0;

	if(techniqueIdx >= (u32)mTechniques.size())
		return 0;

	return mTechniques[techniqueIdx]->GetPassCount();
}

template <bool Core>
SPtr<typename TMaterial<Core>::PassType> TMaterial<Core>::GetPass(u32 passIdx, u32 techniqueIdx) const
{
	if(mShader == nullptr)
		return nullptr;

	if(techniqueIdx >= (u32)mTechniques.size())
		return nullptr;

	if(passIdx < 0 || passIdx >= mTechniques[techniqueIdx]->GetPassCount())
		return nullptr;

	return mTechniques[techniqueIdx]->GetPass(passIdx);
}

template <bool Core>
TMaterialParameterStruct<Core> TMaterial<Core>::GetParamStruct(const String& name) const
{
	ThrowIfNotInitialized();

	return TMaterialParameterStruct<Core>(name, GetMaterialPtr(this));
}

template <bool Core>
TMaterialParameterColorGradient<Core> TMaterial<Core>::GetParamColorGradient(const String& name) const
{
	ThrowIfNotInitialized();

	return TMaterialParameterColorGradient<Core>(name, GetMaterialPtr(this));
}

template <bool Core>
TMaterialParameterCurve<float, Core> TMaterial<Core>::GetParamFloatCurve(const String& name) const
{
	ThrowIfNotInitialized();

	return TMaterialParameterCurve<float, Core>(name, GetMaterialPtr(this));
}

template <bool Core>
TMaterialParameterSampledTexture<Core> TMaterial<Core>::GetParamTexture(const String& name) const
{
	ThrowIfNotInitialized();

	return TMaterialParameterSampledTexture<Core>(name, GetMaterialPtr(this));
}

template <bool Core>
TMaterialParamSpriteTexture<Core> TMaterial<Core>::GetParamSpriteTexture(const String& name) const
{
	ThrowIfNotInitialized();

	return TMaterialParamSpriteTexture<Core>(name, GetMaterialPtr(this));
}

template <bool Core>
TMaterialParameterStorageTexture<Core> TMaterial<Core>::GetParamLoadStoreTexture(const String& name) const
{
	ThrowIfNotInitialized();

	return TMaterialParameterStorageTexture<Core>(name, GetMaterialPtr(this));
}

template <bool Core>
TMaterialParameterBuffer<Core> TMaterial<Core>::GetParamBuffer(const String& name) const
{
	ThrowIfNotInitialized();

	return TMaterialParameterBuffer<Core>(name, GetMaterialPtr(this));
}

template <bool Core>
TMaterialParameterSampler<Core> TMaterial<Core>::GetParamSamplerState(const String& name) const
{
	ThrowIfNotInitialized();

	return TMaterialParameterSampler<Core>(name, GetMaterialPtr(this));
}

template <bool Core>
bool TMaterial<Core>::IsAnimated(const String& name, u32 arrayIdx)
{
	return mParams->IsAnimated(name, arrayIdx);
}

template <bool Core>
void TMaterial<Core>::InitializeTechniques()
{
	mTechniques.clear();

	if(IsShaderValid(mShader))
	{
		mParams = B3DMakeShared<MaterialParamsType>(mShader);
		mTechniques = mShader->GetCompatibleTechniques();

		if(mTechniques.empty())
			return;

		InitDefaultParameters();
	}
	else
		mParams = nullptr;

	MarkDependenciesDirtyInternal();
}

template <bool Core>
template <typename T>
void TMaterial<Core>::SetParamValue(const String& name, u8* buffer, u32 numElements)
{
	TMaterialParameterPrimitive<T, Core> param;
	GetParam(name, param);

	T* ptr = (T*)buffer;
	for(u32 i = 0; i < numElements; i++)
		param.Set(ptr[i], i);
}

template <bool Core>
void TMaterial<Core>::InitDefaultParameters()
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
				TMaterialParameterStruct<Core> param = GetParamStruct(paramData.first);

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

template <bool Core>
template <typename T>
void TMaterial<Core>::GetParam(const String& name, TMaterialParameterPrimitive<T, Core>& output) const
{
	ThrowIfNotInitialized();

	output = TMaterialParameterPrimitive<T, Core>(name, GetMaterialPtr(this));
}

template <bool Core>
void TMaterial<Core>::ThrowIfNotInitialized() const
{
	if(mShader == nullptr)
		B3D_EXCEPT(InternalErrorException, "Material does not have shader set.");

	if(mTechniques.empty())
		B3D_EXCEPT(InternalErrorException, "Shader does not contain a supported technique.");
}

template class TMaterial<false>;
template class TMaterial<true>;

template B3D_CORE_EXPORT void TMaterial<false>::GetParam(const String&, TMaterialParameterPrimitive<float, false>&) const;
template B3D_CORE_EXPORT void TMaterial<false>::GetParam(const String&, TMaterialParameterPrimitive<int, false>&) const;
template B3D_CORE_EXPORT void TMaterial<false>::GetParam(const String&, TMaterialParameterPrimitive<Color, false>&) const;
template B3D_CORE_EXPORT void TMaterial<false>::GetParam(const String&, TMaterialParameterPrimitive<Vector2, false>&) const;
template B3D_CORE_EXPORT void TMaterial<false>::GetParam(const String&, TMaterialParameterPrimitive<Vector3, false>&) const;
template B3D_CORE_EXPORT void TMaterial<false>::GetParam(const String&, TMaterialParameterPrimitive<Vector4, false>&) const;
template B3D_CORE_EXPORT void TMaterial<false>::GetParam(const String&, TMaterialParameterPrimitive<Vector2I, false>&) const;
template B3D_CORE_EXPORT void TMaterial<false>::GetParam(const String&, TMaterialParameterPrimitive<Vector3I, false>&) const;
template B3D_CORE_EXPORT void TMaterial<false>::GetParam(const String&, TMaterialParameterPrimitive<Vector4I, false>&) const;
template B3D_CORE_EXPORT void TMaterial<false>::GetParam(const String&, TMaterialParameterPrimitive<Matrix2, false>&) const;
template B3D_CORE_EXPORT void TMaterial<false>::GetParam(const String&, TMaterialParameterPrimitive<Matrix2x3, false>&) const;
template B3D_CORE_EXPORT void TMaterial<false>::GetParam(const String&, TMaterialParameterPrimitive<Matrix2x4, false>&) const;
template B3D_CORE_EXPORT void TMaterial<false>::GetParam(const String&, TMaterialParameterPrimitive<Matrix3, false>&) const;
template B3D_CORE_EXPORT void TMaterial<false>::GetParam(const String&, TMaterialParameterPrimitive<Matrix3x2, false>&) const;
template B3D_CORE_EXPORT void TMaterial<false>::GetParam(const String&, TMaterialParameterPrimitive<Matrix3x4, false>&) const;
template B3D_CORE_EXPORT void TMaterial<false>::GetParam(const String&, TMaterialParameterPrimitive<Matrix4, false>&) const;
template B3D_CORE_EXPORT void TMaterial<false>::GetParam(const String&, TMaterialParameterPrimitive<Matrix4x2, false>&) const;
template B3D_CORE_EXPORT void TMaterial<false>::GetParam(const String&, TMaterialParameterPrimitive<Matrix4x3, false>&) const;

template B3D_CORE_EXPORT void TMaterial<true>::GetParam(const String&, TMaterialParameterPrimitive<float, true>&) const;
template B3D_CORE_EXPORT void TMaterial<true>::GetParam(const String&, TMaterialParameterPrimitive<int, true>&) const;
template B3D_CORE_EXPORT void TMaterial<true>::GetParam(const String&, TMaterialParameterPrimitive<Color, true>&) const;
template B3D_CORE_EXPORT void TMaterial<true>::GetParam(const String&, TMaterialParameterPrimitive<Vector2, true>&) const;
template B3D_CORE_EXPORT void TMaterial<true>::GetParam(const String&, TMaterialParameterPrimitive<Vector3, true>&) const;
template B3D_CORE_EXPORT void TMaterial<true>::GetParam(const String&, TMaterialParameterPrimitive<Vector4, true>&) const;
template B3D_CORE_EXPORT void TMaterial<true>::GetParam(const String&, TMaterialParameterPrimitive<Vector2I, true>&) const;
template B3D_CORE_EXPORT void TMaterial<true>::GetParam(const String&, TMaterialParameterPrimitive<Vector3I, true>&) const;
template B3D_CORE_EXPORT void TMaterial<true>::GetParam(const String&, TMaterialParameterPrimitive<Vector4I, true>&) const;
template B3D_CORE_EXPORT void TMaterial<true>::GetParam(const String&, TMaterialParameterPrimitive<Matrix2, true>&) const;
template B3D_CORE_EXPORT void TMaterial<true>::GetParam(const String&, TMaterialParameterPrimitive<Matrix2x3, true>&) const;
template B3D_CORE_EXPORT void TMaterial<true>::GetParam(const String&, TMaterialParameterPrimitive<Matrix2x4, true>&) const;
template B3D_CORE_EXPORT void TMaterial<true>::GetParam(const String&, TMaterialParameterPrimitive<Matrix3, true>&) const;
template B3D_CORE_EXPORT void TMaterial<true>::GetParam(const String&, TMaterialParameterPrimitive<Matrix3x2, true>&) const;
template B3D_CORE_EXPORT void TMaterial<true>::GetParam(const String&, TMaterialParameterPrimitive<Matrix3x4, true>&) const;
template B3D_CORE_EXPORT void TMaterial<true>::GetParam(const String&, TMaterialParameterPrimitive<Matrix4, true>&) const;
template B3D_CORE_EXPORT void TMaterial<true>::GetParam(const String&, TMaterialParameterPrimitive<Matrix4x2, true>&) const;
template B3D_CORE_EXPORT void TMaterial<true>::GetParam(const String&, TMaterialParameterPrimitive<Matrix4x3, true>&) const;

Material::Material()
	: mLoadFlags(Load_None)
{}

Material::Material(const HShader& shader, const ShaderVariationParameters& variation)
	: mLoadFlags(Load_None)
{
	mShader = shader;
	mVariation = variation;
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

	mTechniques.clear();
	mLoadFlags = Load_None;

	// Make sure to clear params, because the default behaviour is to re-apply them (which won't work due to changed
	// shader)
	mParams = nullptr;

	MarkResourcesDirtyInternal();

	InitializeIfLoaded();
}

void Material::SetVariation(const ShaderVariationParameters& variation)
{
	mVariation = variation;
	MarkCoreDirty();
}

void Material::MarkCoreDirtyInternal(MaterialDirtyFlags flags)
{
	MarkCoreDirty((u32)flags);
}

void Material::MarkDependenciesDirtyInternal()
{
	MarkDependenciesDirty();
}

void Material::MarkResourcesDirtyInternal()
{
	MarkListenerResourcesDirty();
}

SPtr<ct::Material> Material::GetCore() const
{
	return std::static_pointer_cast<ct::Material>(mCoreSpecific);
}

SPtr<ct::CoreObject> Material::CreateCore() const
{
	ct::Material* material = nullptr;

	SPtr<ct::Shader> shader;
	if(mShader.IsLoaded())
	{
		shader = mShader->GetCore();

		Vector<SPtr<ct::Technique>> techniques(mTechniques.size());
		for(u32 i = 0; i < (u32)mTechniques.size(); i++)
			techniques[i] = mTechniques[i]->GetCore();

		SPtr<ct::MaterialParams> materialParams = B3DMakeShared<ct::MaterialParams>(shader, mParams);

		material = new(B3DAllocate<ct::Material>()) ct::Material(shader, techniques, materialParams, mVariation);
	}

	if(material == nullptr)
		material = new(B3DAllocate<ct::Material>()) ct::Material(shader, mVariation);

	SPtr<ct::Material> materialPtr = B3DMakeSharedFromExisting<ct::Material>(material);
	materialPtr->SetShared(materialPtr);

	return materialPtr;
}

CoreSyncData Material::SyncToCore(FrameAlloc* allocator)
{
	const u32 dirtyParam = (u32)MaterialDirtyFlags::Param;
	const bool syncAllParams = (GetCoreDirtyFlags() & ~dirtyParam) != 0;

	u32 paramsSize = 0;
	if(mParams != nullptr)
		mParams->GetSyncData(nullptr, paramsSize, syncAllParams);

	u32 numTechniques = (u32)mTechniques.size();
	u32 size = sizeof(bool) + sizeof(u32) * 2 + sizeof(SPtr<ct::Shader>) +
		sizeof(SPtr<ct::Technique>) * numTechniques + paramsSize;

	size += CoreSyncGetSize(mVariation);

	u8* buffer = allocator->Alloc(size);
	Bitstream stream(buffer, size);

	B3DRTTIWrite(syncAllParams, stream);

	SPtr<ct::Shader>* shader = new(stream.Cursor()) SPtr<ct::Shader>();
	if(mShader.IsLoaded(false))
		*shader = mShader->GetCore();
	else
		*shader = nullptr;

	stream.SkipBytes(sizeof(SPtr<ct::Shader>));
	B3DRTTIWrite(numTechniques, stream);

	for(u32 i = 0; i < numTechniques; i++)
	{
		SPtr<ct::Technique>* technique = new(stream.Cursor()) SPtr<ct::Technique>();
		*technique = mTechniques[i]->GetCore();

		stream.SkipBytes(sizeof(SPtr<ct::Technique>));
	}

	B3DRTTIWrite(paramsSize, stream);
	if(mParams != nullptr)
		mParams->GetSyncData(stream.Cursor(), paramsSize, syncAllParams);

	stream.SkipBytes(paramsSize);
	B3DCoreSyncWrite(mVariation, stream);

	return CoreSyncData(buffer, size);
}

void Material::GetCoreDependencies(Vector<CoreObject*>& dependencies)
{
	if(mShader.IsLoaded())
		dependencies.push_back(mShader.Get());

	if(mParams != nullptr)
		mParams->GetCoreObjectDependencies(dependencies);
}

void Material::GetListenerResources(Vector<HResource>& resources)
{
	if(mShader != nullptr)
		resources.push_back(mShader);

	if(mParams != nullptr)
		mParams->GetResourceDependencies(resources);
}

void Material::InitializeIfLoaded()
{
	if(AreDependenciesLoaded())
	{
		if(mLoadFlags != Load_All)
		{
			mLoadFlags = Load_All;

			// Shader about to change, so save parameters, rebuild material and restore parameters
			SPtr<MaterialParams> oldParams = mParams;

			InitializeTechniques();
			MarkCoreDirty();

			if(mTechniques.empty()) // Wasn't initialized
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
		MarkCoreDirtyInternal(MaterialDirtyFlags::ParamResource);
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
		MarkCoreDirtyInternal(MaterialDirtyFlags::ParamResource);
	}
}

HMaterial Material::Clone()
{
	SPtr<MemoryDataStream> outputStream = B3DMakeShared<MemoryDataStream>();
	BinarySerializer serializer;

	serializer.Encode(this, outputStream);
	outputStream->Seek(0);
	SPtr<Material> cloneObj = std::static_pointer_cast<Material>(serializer.Decode(outputStream, (u32)outputStream->Size()));

	return B3DStaticResourceCast<Material>(GetResources().CreateResourceHandleInternal(cloneObj));
}

template <class T>
void CopyParam(const SPtr<MaterialParams>& from, Material* to, const String& name, const MaterialParams::ParamData& paramRef, u32 arraySize)
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

void Material::SetParams(const SPtr<MaterialParams>& params)
{
	if(params == nullptr)
		return;

	std::function<
		void(const SPtr<MaterialParams>&, Material*, const String&, const MaterialParams::ParamData&, u32)>
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

		const MaterialParams::ParamData* paramData = nullptr;
		auto result = params->GetParamData(param.first, MaterialParams::ParamType::Data, param.second.Type, 0, &paramData);

		if(result != MaterialParams::GetParamResult::Success)
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
		const MaterialParams::ParamData* paramData = nullptr;
		auto result = params->GetParamData(param.first, MaterialParams::ParamType::Texture, GPDT_UNKNOWN, 0, &paramData);

		if(result != MaterialParams::GetParamResult::Success)
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
				TMaterialParamSpriteTexture<false> curParam = GetParamSpriteTexture(param.first);

				HSpriteTexture texture;
				params->GetSpriteTexture(*paramData, texture);
				curParam.Set(texture);
			}
			break;
		}
	}

	auto& bufferParams = mShader->GetBufferParams();
	for(auto& param : bufferParams)
	{
		const MaterialParams::ParamData* paramData = nullptr;
		auto result = params->GetParamData(param.first, MaterialParams::ParamType::Buffer, GPDT_UNKNOWN, 0, &paramData);

		if(result != MaterialParams::GetParamResult::Success)
			continue;

		TMaterialParameterBuffer<false> curParam = GetParamBuffer(param.first);

		SPtr<GpuBuffer> buffer;
		params->GetBuffer(*paramData, buffer);
		curParam.Set(buffer);
	}

	auto& samplerParams = mShader->GetSamplerParams();
	for(auto& param : samplerParams)
	{
		const MaterialParams::ParamData* paramData = nullptr;
		auto result = params->GetParamData(param.first, MaterialParams::ParamType::Sampler, GPDT_UNKNOWN, 0, &paramData);

		if(result != MaterialParams::GetParamResult::Success)
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

	return B3DStaticResourceCast<Material>(GetResources().CreateResourceHandleInternal(materialPtr));
}

HMaterial Material::Create(const HShader& shader)
{
	return Create(shader, ShaderVariationParameters::kEmpty);
}

HMaterial Material::Create(const HShader& shader, const ShaderVariationParameters& variation)
{
	SPtr<Material> materialPtr = B3DMakeCoreFromExisting<Material>(new(B3DAllocate<Material>()) Material(shader, variation));
	materialPtr->SetShared(materialPtr);
	materialPtr->Initialize();

	return B3DStaticResourceCast<Material>(GetResources().CreateResourceHandleInternal(materialPtr));
}

SPtr<Material> Material::CreateEmpty()
{
	SPtr<Material> newMat = B3DMakeCoreFromExisting<Material>(new(B3DAllocate<Material>()) Material());
	newMat->SetShared(newMat);

	return newMat;
}

RTTITypeBase* Material::GetRttiStatic()
{
	return MaterialRTTI::Instance();
}

RTTITypeBase* Material::GetRtti() const
{
	return Material::GetRttiStatic();
}

namespace bs { namespace ct
{
Material::Material(const SPtr<Shader>& shader, const ShaderVariationParameters& variation)
{
	mVariation = variation;
	SetShader(shader);
}

Material::Material(const SPtr<Shader>& shader, const Vector<SPtr<Technique>>& techniques, const SPtr<MaterialParams>& materialParams, const ShaderVariationParameters& variation)
{
	mShader = shader;
	mParams = materialParams;
	mTechniques = techniques;
	mVariation = variation;
}

void Material::SetShader(const SPtr<Shader>& shader)
{
	mShader = shader;

	InitializeTechniques();
}

void Material::SetVariation(const ShaderVariationParameters& variation)
{
	mVariation = variation;
}

void Material::SyncToCore(const CoreSyncData& data)
{
	Bitstream stream(data.GetBuffer(), data.GetBufferSize());

	bool syncAllParams;
	B3DRTTIRead(syncAllParams, stream);

	u64 initialParamVersion = mParams != nullptr ? mParams->GetParamVersion() : 1;
	if(syncAllParams)
		mParams = nullptr;

	SPtr<Shader>* shader = (SPtr<Shader>*)stream.Cursor();

	mShader = *shader;
	shader->~SPtr<Shader>();
	stream.SkipBytes(sizeof(SPtr<Shader>));

	u32 numTechniques;
	B3DRTTIRead(numTechniques, stream);

	mTechniques.resize(numTechniques);
	for(u32 i = 0; i < numTechniques; i++)
	{
		SPtr<Technique>* technique = (SPtr<Technique>*)stream.Cursor();
		mTechniques[i] = *technique;
		technique->~SPtr<Technique>();
		stream.SkipBytes(sizeof(SPtr<Technique>));
	}

	u32 paramsSize = 0;
	B3DRTTIRead(paramsSize, stream);
	if(mParams == nullptr && mShader != nullptr)
		mParams = B3DMakeShared<MaterialParams>(mShader, initialParamVersion);

	if(mParams != nullptr && paramsSize > 0)
		mParams->SetSyncData(stream.Cursor(), paramsSize);

	stream.SkipBytes(paramsSize);

	mVariation.ClearParams();
	B3DCoreSyncRead(mVariation, stream);
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
