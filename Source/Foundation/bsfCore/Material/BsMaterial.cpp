//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Material/BsMaterial.h"
#include "Material/BsShader.h"
#include "Material/BsTechnique.h"
#include "Material/BsPass.h"
#include "RenderAPI/BsRenderAPI.h"
#include "Private/RTTI/BsMaterialRTTI.h"
#include "Resources/BsResources.h"
#include "Math/BsMatrixNxM.h"
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
bool isShaderValid(const T& shader)
{
	return false;
}

template <>
bool isShaderValid(const HShader& shader)
{
	return shader.IsLoaded();
}

template <>
bool isShaderValid(const SPtr<ct::Shader>& shader)
{
	return shader != nullptr;
}

template <bool Core>
SPtr<CoreVariantType<Material, Core>> getMaterialPtr(const TMaterial<Core>* material)
{
	return std::static_pointer_cast<CoreVariantType<Material, Core>>(
		static_cast<const CoreVariantType<Material, Core>*>(material)->GetThisPtr());
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
u32 TMaterial<Core>::FindTechnique(const FIND_TECHNIQUE_DESC& desc) const
{
	u32 bestTechniqueIdx = (u32)-1;
	u32 bestTechniqueScore = std::numeric_limits<u32>::max();

	for(u32 i = 0; i < (u32)mTechniques.size(); i++)
	{
		// Make sure tags match
		bool foundMatch = true;
		for(u32 j = 0; j < desc.NumTags; j++)
		{
			if(!mTechniques[i]->HasTag(desc.Tags[j]))
			{
				foundMatch = false;
				break;
			}
		}

		if(!foundMatch)
			continue;

		const ShaderVariation& curVariation = mTechniques[i]->GetVariation();
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
			if(desc.Variation)
			{
				const auto& searchVarParams = desc.Variation->GetParams();
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

		if(desc.Variation)
		{
			const auto& searchVarParams = desc.Variation->GetParams();
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
		if(mTechniques[i]->HasTags())
			continue;

		const ShaderVariation& curVariation = mTechniques[i]->GetVariation();
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

	return mTechniques[techniqueIdx]->GetNumPasses();
}

template <bool Core>
SPtr<typename TMaterial<Core>::PassType> TMaterial<Core>::GetPass(u32 passIdx, u32 techniqueIdx) const
{
	if(mShader == nullptr)
		return nullptr;

	if(techniqueIdx >= (u32)mTechniques.size())
		return nullptr;

	if(passIdx < 0 || passIdx >= mTechniques[techniqueIdx]->GetNumPasses())
		return nullptr;

	return mTechniques[techniqueIdx]->GetPass(passIdx);
}

template <bool Core>
TMaterialParamStruct<Core> TMaterial<Core>::GetParamStruct(const String& name) const
{
	ThrowIfNotInitialized();

	return TMaterialParamStruct<Core>(name, getMaterialPtr(this));
}

template <bool Core>
TMaterialColorGradientParam<Core> TMaterial<Core>::GetParamColorGradient(const String& name) const
{
	ThrowIfNotInitialized();

	return TMaterialColorGradientParam<Core>(name, getMaterialPtr(this));
}

template <bool Core>
TMaterialCurveParam<float, Core> TMaterial<Core>::GetParamFloatCurve(const String& name) const
{
	ThrowIfNotInitialized();

	return TMaterialCurveParam<float, Core>(name, getMaterialPtr(this));
}

template <bool Core>
TMaterialParamTexture<Core> TMaterial<Core>::GetParamTexture(const String& name) const
{
	ThrowIfNotInitialized();

	return TMaterialParamTexture<Core>(name, getMaterialPtr(this));
}

template <bool Core>
TMaterialParamSpriteTexture<Core> TMaterial<Core>::GetParamSpriteTexture(const String& name) const
{
	ThrowIfNotInitialized();

	return TMaterialParamSpriteTexture<Core>(name, getMaterialPtr(this));
}

template <bool Core>
TMaterialParamLoadStoreTexture<Core> TMaterial<Core>::GetParamLoadStoreTexture(const String& name) const
{
	ThrowIfNotInitialized();

	return TMaterialParamLoadStoreTexture<Core>(name, getMaterialPtr(this));
}

template <bool Core>
TMaterialParamBuffer<Core> TMaterial<Core>::GetParamBuffer(const String& name) const
{
	ThrowIfNotInitialized();

	return TMaterialParamBuffer<Core>(name, getMaterialPtr(this));
}

template <bool Core>
TMaterialParamSampState<Core> TMaterial<Core>::GetParamSamplerState(const String& name) const
{
	ThrowIfNotInitialized();

	return TMaterialParamSampState<Core>(name, getMaterialPtr(this));
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

	if(isShaderValid(mShader))
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
	TMaterialDataParam<T, Core> param;
	GetParam(name, param);

	T* ptr = (T*)buffer;
	for(u32 i = 0; i < numElements; i++)
		param.Set(ptr[i], i);
}

template <bool Core>
void TMaterial<Core>::InitDefaultParameters()
{
	const Map<String, SHADER_DATA_PARAM_DESC>& dataParams = mShader->GetDataParams();
	for(auto& paramData : dataParams)
	{
		if(paramData.second.DefaultValueIdx == (u32)-1)
			continue;

		u8* buffer = (u8*)mShader->GetDefaultValue(paramData.second.DefaultValueIdx);
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
			SetParamValue<int>(paramData.first, buffer, paramData.second.ArraySize);
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
		case GPDT_BOOL:
			SetParamValue<int>(paramData.first, buffer, paramData.second.ArraySize);
			break;
		case GPDT_COLOR:
			SetParamValue<Color>(paramData.first, buffer, paramData.second.ArraySize);
			break;
		case GPDT_STRUCT:
			{
				TMaterialParamStruct<Core> param = GetParamStruct(paramData.first);

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

	const Map<String, SHADER_OBJECT_PARAM_DESC>& textureParams = mShader->GetTextureParams();
	for(auto& param : textureParams)
	{
		if(param.second.DefaultValueIdx == (u32)-1)
			continue;

		TextureType defaultTex = mShader->GetDefaultTexture(param.second.DefaultValueIdx);
		GetParamTexture(param.first).Set(defaultTex);
	}

	const Map<String, SHADER_OBJECT_PARAM_DESC>& samplerParams = mShader->GetSamplerParams();
	for(auto& param : samplerParams)
	{
		if(param.second.DefaultValueIdx == (u32)-1)
			continue;

		SamplerStateType defaultSampler = mShader->GetDefaultSampler(param.second.DefaultValueIdx);
		GetParamSamplerState(param.first).Set(defaultSampler);
	}
}

template <bool Core>
template <typename T>
void TMaterial<Core>::GetParam(const String& name, TMaterialDataParam<T, Core>& output) const
{
	ThrowIfNotInitialized();

	output = TMaterialDataParam<T, Core>(name, getMaterialPtr(this));
}

template <bool Core>
void TMaterial<Core>::ThrowIfNotInitialized() const
{
	if(mShader == nullptr)
		BS_EXCEPT(InternalErrorException, "Material does not have shader set.");

	if(mTechniques.empty())
		BS_EXCEPT(InternalErrorException, "Shader does not contain a supported technique.");
}

template class TMaterial<false>;
template class TMaterial<true>;

template BS_CORE_EXPORT void TMaterial<false>::GetParam(const String&, TMaterialDataParam<float, false>&) const;
template BS_CORE_EXPORT void TMaterial<false>::GetParam(const String&, TMaterialDataParam<int, false>&) const;
template BS_CORE_EXPORT void TMaterial<false>::GetParam(const String&, TMaterialDataParam<Color, false>&) const;
template BS_CORE_EXPORT void TMaterial<false>::GetParam(const String&, TMaterialDataParam<Vector2, false>&) const;
template BS_CORE_EXPORT void TMaterial<false>::GetParam(const String&, TMaterialDataParam<Vector3, false>&) const;
template BS_CORE_EXPORT void TMaterial<false>::GetParam(const String&, TMaterialDataParam<Vector4, false>&) const;
template BS_CORE_EXPORT void TMaterial<false>::GetParam(const String&, TMaterialDataParam<Vector2I, false>&) const;
template BS_CORE_EXPORT void TMaterial<false>::GetParam(const String&, TMaterialDataParam<Vector3I, false>&) const;
template BS_CORE_EXPORT void TMaterial<false>::GetParam(const String&, TMaterialDataParam<Vector4I, false>&) const;
template BS_CORE_EXPORT void TMaterial<false>::GetParam(const String&, TMaterialDataParam<Matrix2, false>&) const;
template BS_CORE_EXPORT void TMaterial<false>::GetParam(const String&, TMaterialDataParam<Matrix2x3, false>&) const;
template BS_CORE_EXPORT void TMaterial<false>::GetParam(const String&, TMaterialDataParam<Matrix2x4, false>&) const;
template BS_CORE_EXPORT void TMaterial<false>::GetParam(const String&, TMaterialDataParam<Matrix3, false>&) const;
template BS_CORE_EXPORT void TMaterial<false>::GetParam(const String&, TMaterialDataParam<Matrix3x2, false>&) const;
template BS_CORE_EXPORT void TMaterial<false>::GetParam(const String&, TMaterialDataParam<Matrix3x4, false>&) const;
template BS_CORE_EXPORT void TMaterial<false>::GetParam(const String&, TMaterialDataParam<Matrix4, false>&) const;
template BS_CORE_EXPORT void TMaterial<false>::GetParam(const String&, TMaterialDataParam<Matrix4x2, false>&) const;
template BS_CORE_EXPORT void TMaterial<false>::GetParam(const String&, TMaterialDataParam<Matrix4x3, false>&) const;

template BS_CORE_EXPORT void TMaterial<true>::GetParam(const String&, TMaterialDataParam<float, true>&) const;
template BS_CORE_EXPORT void TMaterial<true>::GetParam(const String&, TMaterialDataParam<int, true>&) const;
template BS_CORE_EXPORT void TMaterial<true>::GetParam(const String&, TMaterialDataParam<Color, true>&) const;
template BS_CORE_EXPORT void TMaterial<true>::GetParam(const String&, TMaterialDataParam<Vector2, true>&) const;
template BS_CORE_EXPORT void TMaterial<true>::GetParam(const String&, TMaterialDataParam<Vector3, true>&) const;
template BS_CORE_EXPORT void TMaterial<true>::GetParam(const String&, TMaterialDataParam<Vector4, true>&) const;
template BS_CORE_EXPORT void TMaterial<true>::GetParam(const String&, TMaterialDataParam<Vector2I, true>&) const;
template BS_CORE_EXPORT void TMaterial<true>::GetParam(const String&, TMaterialDataParam<Vector3I, true>&) const;
template BS_CORE_EXPORT void TMaterial<true>::GetParam(const String&, TMaterialDataParam<Vector4I, true>&) const;
template BS_CORE_EXPORT void TMaterial<true>::GetParam(const String&, TMaterialDataParam<Matrix2, true>&) const;
template BS_CORE_EXPORT void TMaterial<true>::GetParam(const String&, TMaterialDataParam<Matrix2x3, true>&) const;
template BS_CORE_EXPORT void TMaterial<true>::GetParam(const String&, TMaterialDataParam<Matrix2x4, true>&) const;
template BS_CORE_EXPORT void TMaterial<true>::GetParam(const String&, TMaterialDataParam<Matrix3, true>&) const;
template BS_CORE_EXPORT void TMaterial<true>::GetParam(const String&, TMaterialDataParam<Matrix3x2, true>&) const;
template BS_CORE_EXPORT void TMaterial<true>::GetParam(const String&, TMaterialDataParam<Matrix3x4, true>&) const;
template BS_CORE_EXPORT void TMaterial<true>::GetParam(const String&, TMaterialDataParam<Matrix4, true>&) const;
template BS_CORE_EXPORT void TMaterial<true>::GetParam(const String&, TMaterialDataParam<Matrix4x2, true>&) const;
template BS_CORE_EXPORT void TMaterial<true>::GetParam(const String&, TMaterialDataParam<Matrix4x3, true>&) const;

Material::Material()
	: mLoadFlags(Load_None)
{}

Material::Material(const HShader& shader, const ShaderVariation& variation)
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

void Material::SetVariation(const ShaderVariation& variation)
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
	materialPtr->SetThisPtrInternal(materialPtr);

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

	size += csync_size(mVariation);

	u8* buffer = allocator->Alloc(size);
	Bitstream stream(buffer, size);

	rtti_write(syncAllParams, stream);

	SPtr<ct::Shader>* shader = new(stream.Cursor()) SPtr<ct::Shader>();
	if(mShader.IsLoaded(false))
		*shader = mShader->GetCore();
	else
		*shader = nullptr;

	stream.SkipBytes(sizeof(SPtr<ct::Shader>));
	rtti_write(numTechniques, stream);

	for(u32 i = 0; i < numTechniques; i++)
	{
		SPtr<ct::Technique>* technique = new(stream.Cursor()) SPtr<ct::Technique>();
		*technique = mTechniques[i]->GetCore();

		stream.SkipBytes(sizeof(SPtr<ct::Technique>));
	}

	rtti_write(paramsSize, stream);
	if(mParams != nullptr)
		mParams->GetSyncData(stream.Cursor(), paramsSize, syncAllParams);

	stream.SkipBytes(paramsSize);
	csync_write(mVariation, stream);

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

	return static_resource_cast<Material>(GetResources().CreateResourceHandleInternal(cloneObj));
}

template <class T>
void copyParam(const SPtr<MaterialParams>& from, Material* to, const String& name, const MaterialParams::ParamData& paramRef, u32 arraySize)
{
	TMaterialDataParam<T, false> param;
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

	copyParamLookup[GPDT_FLOAT1] = &copyParam<float>;
	copyParamLookup[GPDT_FLOAT2] = &copyParam<Vector2>;
	copyParamLookup[GPDT_FLOAT3] = &copyParam<Vector3>;
	copyParamLookup[GPDT_FLOAT4] = &copyParam<Vector4>;

	copyParamLookup[GPDT_INT1] = &copyParam<int>;
	copyParamLookup[GPDT_INT2] = &copyParam<Vector2I>;
	copyParamLookup[GPDT_INT3] = &copyParam<Vector3I>;
	copyParamLookup[GPDT_INT4] = &copyParam<Vector4I>;

	copyParamLookup[GPDT_MATRIX_2X2] = &copyParam<Matrix2>;
	copyParamLookup[GPDT_MATRIX_2X3] = &copyParam<Matrix2x3>;
	copyParamLookup[GPDT_MATRIX_2X4] = &copyParam<Matrix2x4>;

	copyParamLookup[GPDT_MATRIX_3X3] = &copyParam<Matrix3>;
	copyParamLookup[GPDT_MATRIX_3X2] = &copyParam<Matrix3x2>;
	copyParamLookup[GPDT_MATRIX_3X4] = &copyParam<Matrix3x4>;

	copyParamLookup[GPDT_MATRIX_4X4] = &copyParam<Matrix4>;
	copyParamLookup[GPDT_MATRIX_4X2] = &copyParam<Matrix4x2>;
	copyParamLookup[GPDT_MATRIX_4X3] = &copyParam<Matrix4x3>;

	copyParamLookup[GPDT_BOOL] = &copyParam<int>;
	copyParamLookup[GPDT_COLOR] = &copyParam<Color>;

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
				TMaterialParamStruct<false> curParam = GetParamStruct(param.first);

				u32 structSize = params->GetStructSize(*paramData);
				if(param.second.ElementSize != structSize)
					continue;

				u8* structData = (u8*)bs_stack_alloc(structSize);
				for(u32 i = 0; i < elemsToCopy; i++)
				{
					params->GetStructData(*paramData, structData, structSize, i);
					curParam.Set(structData, structSize, i);
				}

				bs_stack_free(structData);
			}
		}

		for(u32 i = 0; i < arraySize; i++)
		{
			const bool isAnimated = params->IsAnimated(*paramData, i);
			if(!isAnimated)
				continue;

			if(param.second.Type == GPDT_FLOAT1)
			{
				TMaterialCurveParam<float, false> curParam = GetParamFloatCurve(param.first);
				curParam.Set(params->GetCurveParam<float>(*paramData, i), i);
			}
			else if(param.second.Type == GPDT_COLOR)
			{
				TMaterialColorGradientParam<false> curParam = GetParamColorGradient(param.first);
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
				TMaterialParamTexture<false> curParam = GetParamTexture(param.first);

				HTexture texture;
				TextureSurface surface;
				params->GetTexture(*paramData, texture, surface);
				curParam.Set(texture);
			}
			break;
		case MateralParamTextureType::LoadStore:
			{
				TMaterialParamLoadStoreTexture<false> curParam = GetParamLoadStoreTexture(param.first);

				HTexture texture;
				TextureSurface surface;
				params->GetLoadStoreTexture(*paramData, texture, surface);
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

		TMaterialParamBuffer<false> curParam = GetParamBuffer(param.first);

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

		TMaterialParamSampState<false> curParam = GetParamSamplerState(param.first);

		SPtr<SamplerState> samplerState;
		params->GetSamplerState(*paramData, samplerState);
		curParam.Set(samplerState);
	}
}

HMaterial Material::Create()
{
	const SPtr<Material> materialPtr = CreateEmpty();
	materialPtr->Initialize();

	return static_resource_cast<Material>(GetResources().CreateResourceHandleInternal(materialPtr));
}

HMaterial Material::Create(const HShader& shader)
{
	return Create(shader, ShaderVariation::kEmpty);
}

HMaterial Material::Create(const HShader& shader, const ShaderVariation& variation)
{
	SPtr<Material> materialPtr = B3DMakeCoreFromExisting<Material>(new(B3DAllocate<Material>()) Material(shader, variation));
	materialPtr->SetThisPtrInternal(materialPtr);
	materialPtr->Initialize();

	return static_resource_cast<Material>(GetResources().CreateResourceHandleInternal(materialPtr));
}

SPtr<Material> Material::CreateEmpty()
{
	SPtr<Material> newMat = B3DMakeCoreFromExisting<Material>(new(B3DAllocate<Material>()) Material());
	newMat->SetThisPtrInternal(newMat);

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
Material::Material(const SPtr<Shader>& shader, const ShaderVariation& variation)
{
	mVariation = variation;
	SetShader(shader);
}

Material::Material(const SPtr<Shader>& shader, const Vector<SPtr<Technique>>& techniques, const SPtr<MaterialParams>& materialParams, const ShaderVariation& variation)
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

void Material::SetVariation(const ShaderVariation& variation)
{
	mVariation = variation;
}

void Material::SyncToCore(const CoreSyncData& data)
{
	Bitstream stream(data.GetBuffer(), data.GetBufferSize());

	bool syncAllParams;
	rtti_read(syncAllParams, stream);

	u64 initialParamVersion = mParams != nullptr ? mParams->GetParamVersion() : 1;
	if(syncAllParams)
		mParams = nullptr;

	SPtr<Shader>* shader = (SPtr<Shader>*)stream.Cursor();

	mShader = *shader;
	shader->~SPtr<Shader>();
	stream.SkipBytes(sizeof(SPtr<Shader>));

	u32 numTechniques;
	rtti_read(numTechniques, stream);

	mTechniques.resize(numTechniques);
	for(u32 i = 0; i < numTechniques; i++)
	{
		SPtr<Technique>* technique = (SPtr<Technique>*)stream.Cursor();
		mTechniques[i] = *technique;
		technique->~SPtr<Technique>();
		stream.SkipBytes(sizeof(SPtr<Technique>));
	}

	u32 paramsSize = 0;
	rtti_read(paramsSize, stream);
	if(mParams == nullptr && mShader != nullptr)
		mParams = B3DMakeShared<MaterialParams>(mShader, initialParamVersion);

	if(mParams != nullptr && paramsSize > 0)
		mParams->SetSyncData(stream.Cursor(), paramsSize);

	stream.SkipBytes(paramsSize);

	mVariation.ClearParams();
	csync_read(mVariation, stream);
}

SPtr<Material> Material::Create(const SPtr<Shader>& shader)
{
	Material* material = new(B3DAllocate<Material>()) Material(shader, ShaderVariation::kEmpty);
	SPtr<Material> materialPtr = B3DMakeSharedFromExisting<Material>(material);
	materialPtr->SetThisPtrInternal(materialPtr);
	materialPtr->Initialize();

	return materialPtr;
}
}}
