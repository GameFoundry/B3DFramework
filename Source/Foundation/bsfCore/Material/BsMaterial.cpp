//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Material/BsMaterial.h"
#include "Material/BsShader.h"
#include "Material/BsTechnique.h"
#include "Material/BsPass.h"
#include "RenderAPI/BsRenderAPI.h"
#include "Private/RTTI/BsMaterialRTTI.h"
#include "Material/BsMaterialManager.h"
#include "Resources/BsResources.h"
#include "Math/BsMatrixNxM.h"
#include "Math/BsVector3I.h"
#include "Math/BsVector4I.h"
#include "Serialization/BsMemorySerializer.h"
#include "Material/BsMaterialParams.h"
#include "Material/BsGpuParamsSet.h"
#include "Animation/BsAnimationCurve.h"

namespace bs
{
	enum MaterialLoadFlags
	{
		Load_None	= 0,
		Load_Shader	= 1,
		Load_All	= 2
	};
	
	template<class T>
	bool isShaderValid(const T& shader) { return false; }

	template<>
	bool isShaderValid(const HShader& shader) { return shader.isLoaded(); }

	template<>
	bool isShaderValid(const SPtr<ct::Shader>& shader) { return shader != nullptr; }

	template<bool Core>
	SPtr<CoreVariantType<Material, Core>> getMaterialPtr(const TMaterial<Core>* material)
	{
		return std::static_pointer_cast<CoreVariantType<Material, Core>>(
			static_cast<const CoreVariantType<Material, Core>*>(material)->getThisPtr());
	}

	template<bool Core>
	SPtr<typename TMaterial<Core>::GpuParamsSetType> TMaterial<Core>::createParamsSet(UINT32 techniqueIdx)
	{
		if (techniqueIdx >= (UINT32)mTechniques.size())
			return nullptr;

		SPtr<TechniqueType> technique = mTechniques[techniqueIdx];
		return bs_shared_ptr_new<GpuParamsSetType>(technique, mShader, mParams);
	}

	template<bool Core>
	void TMaterial<Core>::updateParamsSet(const SPtr<GpuParamsSetType>& paramsSet, float t, bool updateAll)
	{
		paramsSet->update(mParams, t, updateAll);
	}
	
	template<bool Core>
	UINT32 TMaterial<Core>::findTechnique(const FIND_TECHNIQUE_DESC& desc) const
	{
		for(UINT32 i = 0; i < (UINT32)mTechniques.size(); i++)
		{
			bool foundMatch = true;
			for(UINT32 j = 0; j < desc.numTags; j++)
			{
				if (!mTechniques[i]->hasTag(desc.tags[j]))
				{
					foundMatch = false;
					break;
				}
			}

			if(!foundMatch)
				continue;

			if(desc.variation)
			{
				const ShaderVariation& curVariation = mTechniques[i]->getVariation();
				const auto& curVarParams = curVariation.getParams();

				foundMatch = true;
				const auto& searchVarParams = desc.variation->getParams();
				for (auto& param : searchVarParams)
				{
					auto iterFind = curVarParams.find(param.first);
					if (iterFind == curVarParams.end())
					{
						foundMatch = false;
						break;
					}

					if (param.second.i != iterFind->second.i)
					{
						foundMatch = false;
						break;
					}
				}
			}

			if(foundMatch)
				return i;
		}

		return (UINT32)-1;
		
	}

	template<bool Core>
	UINT32 TMaterial<Core>::getDefaultTechnique() const
	{
		for (UINT32 i = 0; i < (UINT32)mTechniques.size(); i++)
		{
			if (!mTechniques[i]->hasTags())
				return i;
		}

		return 0;
	}

	template<bool Core>
	UINT32 TMaterial<Core>::getNumPasses(UINT32 techniqueIdx) const
	{
		if (mShader == nullptr)
			return 0;

		if (techniqueIdx >= (UINT32)mTechniques.size())
			return 0;

		return mTechniques[techniqueIdx]->getNumPasses();
	}

	template<bool Core>
	SPtr<typename TMaterial<Core>::PassType> TMaterial<Core>::getPass(UINT32 passIdx, UINT32 techniqueIdx) const
	{
		if (mShader == nullptr)
			return nullptr;

		if (techniqueIdx >= (UINT32)mTechniques.size())
			return nullptr;

		if (passIdx < 0 || passIdx >= mTechniques[techniqueIdx]->getNumPasses())
			return nullptr;

		return mTechniques[techniqueIdx]->getPass(passIdx);
	}

	template<bool Core>
	TMaterialParamStruct<Core> TMaterial<Core>::getParamStruct(const String& name) const
	{
		throwIfNotInitialized();

		return TMaterialParamStruct<Core>(name, getMaterialPtr(this));
	}

	template<bool Core>
	TMaterialColorGradientParam<Core> TMaterial<Core>::getParamColorGradient(const String& name) const
	{
		throwIfNotInitialized();

		return TMaterialColorGradientParam<Core>(name, getMaterialPtr(this));
	}

	template <bool Core>
	TMaterialCurveParam<float, Core> TMaterial<Core>::getParamFloatCurve(const String& name) const
	{
		throwIfNotInitialized();

		return TMaterialCurveParam<float, Core>(name, getMaterialPtr(this));
	}

	template<bool Core>
	TMaterialParamTexture<Core> TMaterial<Core>::getParamTexture(const String& name) const
	{
		throwIfNotInitialized();

		return TMaterialParamTexture<Core>(name, getMaterialPtr(this));
	}

	template<bool Core>
	TMaterialParamSpriteTexture<Core> TMaterial<Core>::getParamSpriteTexture(const String& name) const
	{
		throwIfNotInitialized();

		return TMaterialParamSpriteTexture<Core>(name, getMaterialPtr(this));
	}

	template<bool Core>
	TMaterialParamLoadStoreTexture<Core> TMaterial<Core>::getParamLoadStoreTexture(const String& name) const
	{
		throwIfNotInitialized();

		return TMaterialParamLoadStoreTexture<Core>(name, getMaterialPtr(this));
	}

	template<bool Core>
	TMaterialParamBuffer<Core> TMaterial<Core>::getParamBuffer(const String& name) const
	{
		throwIfNotInitialized();

		return TMaterialParamBuffer<Core>(name, getMaterialPtr(this));
	}

	template<bool Core>
	TMaterialParamSampState<Core> TMaterial<Core>::getParamSamplerState(const String& name) const
	{
		throwIfNotInitialized();

		return TMaterialParamSampState<Core>(name, getMaterialPtr(this));
	}

	template<bool Core>
	void TMaterial<Core>::initializeTechniques(bool allVariations, const ShaderVariation& variation)
	{
		mTechniques.clear();

		if (isShaderValid(mShader))
		{
			mParams = bs_shared_ptr_new<MaterialParamsType>(mShader);

			if(allVariations)
				mTechniques = mShader->getCompatibleTechniques();
			else
				mTechniques = mShader->getCompatibleTechniques(variation);

			if (mTechniques.size() == 0)
				return;

			for(auto& entry : mTechniques)
				entry->compile();

			initDefaultParameters();
		}
		else
			mParams = nullptr;

		_markDependenciesDirty();
	}

	template <bool Core>
	template <typename T>
	void TMaterial<Core>::setParamValue(const String& name, UINT8* buffer, UINT32 numElements)
	{
		TMaterialDataParam<T, Core> param;
		getParam(name, param);

		T* ptr = (T*)buffer;
		for (UINT32 i = 0; i < numElements; i++)
			param.set(ptr[i], i);
	}

	template <bool Core>
	void TMaterial<Core>::initDefaultParameters()
	{
		const Map<String, SHADER_DATA_PARAM_DESC>& dataParams = mShader->getDataParams();
		for (auto& paramData : dataParams)
		{
			if (paramData.second.defaultValueIdx == (UINT32)-1)
				continue;

			UINT8* buffer = (UINT8*)mShader->getDefaultValue(paramData.second.defaultValueIdx);
			if (buffer == nullptr)
				continue;

			switch (paramData.second.type)
			{
			case GPDT_FLOAT1: 
				setParamValue<float>(paramData.first, buffer, paramData.second.arraySize);
				break;
			case GPDT_FLOAT2:
				setParamValue<Vector2>(paramData.first, buffer, paramData.second.arraySize);
				break;
			case GPDT_FLOAT3:
				setParamValue<Vector3>(paramData.first, buffer, paramData.second.arraySize);
				break;
			case GPDT_FLOAT4:
				setParamValue<Vector4>(paramData.first, buffer, paramData.second.arraySize);
				break;
			case GPDT_MATRIX_2X2:
				setParamValue<Matrix2>(paramData.first, buffer, paramData.second.arraySize);
				break;
			case GPDT_MATRIX_2X3:
				setParamValue<Matrix2x3>(paramData.first, buffer, paramData.second.arraySize);
				break;
			case GPDT_MATRIX_2X4:
				setParamValue<Matrix2x4>(paramData.first, buffer, paramData.second.arraySize);
				break;
			case GPDT_MATRIX_3X2:
				setParamValue<Matrix3x2>(paramData.first, buffer, paramData.second.arraySize);
				break;
			case GPDT_MATRIX_3X3:
				setParamValue<Matrix3>(paramData.first, buffer, paramData.second.arraySize);
				break;
			case GPDT_MATRIX_3X4:
				setParamValue<Matrix3x4>(paramData.first, buffer, paramData.second.arraySize);
				break;
			case GPDT_MATRIX_4X2:
				setParamValue<Matrix4x2>(paramData.first, buffer, paramData.second.arraySize);
				break;
			case GPDT_MATRIX_4X3:
				setParamValue<Matrix4x3>(paramData.first, buffer, paramData.second.arraySize);
				break;
			case GPDT_MATRIX_4X4:
				setParamValue<Matrix4>(paramData.first, buffer, paramData.second.arraySize);
				break;
			case GPDT_INT1:
				setParamValue<int>(paramData.first, buffer, paramData.second.arraySize);
				break;
			case GPDT_INT2:
				setParamValue<Vector2I>(paramData.first, buffer, paramData.second.arraySize);
				break;
			case GPDT_INT3:
				setParamValue<Vector3I>(paramData.first, buffer, paramData.second.arraySize);
				break;
			case GPDT_INT4:
				setParamValue<Vector4I>(paramData.first, buffer, paramData.second.arraySize);
				break;
			case GPDT_BOOL:
				setParamValue<int>(paramData.first, buffer, paramData.second.arraySize);
				break;
			case GPDT_COLOR:
				setParamValue<Color>(paramData.first, buffer, paramData.second.arraySize);
				break;
			case GPDT_STRUCT:
			{
				TMaterialParamStruct<Core> param = getParamStruct(paramData.first);

				UINT32 elementSizeBytes = paramData.second.elementSize * sizeof(UINT32);
				UINT8* ptr = buffer;
				for (UINT32 i = 0; i < paramData.second.arraySize; i++)
				{
					param.set(ptr, elementSizeBytes, i);
					ptr += elementSizeBytes;
				}
			}
			break;
			default:
				break;
			}
		}

		const Map<String, SHADER_OBJECT_PARAM_DESC>& textureParams = mShader->getTextureParams();
		for (auto& param : textureParams)
		{
			if (param.second.defaultValueIdx == (UINT32)-1)
				continue;

			TextureType defaultTex = mShader->getDefaultTexture(param.second.defaultValueIdx);
			getParamTexture(param.first).set(defaultTex);
		}

		const Map<String, SHADER_OBJECT_PARAM_DESC>& samplerParams = mShader->getSamplerParams();
		for (auto& param : samplerParams)
		{
			if (param.second.defaultValueIdx == (UINT32)-1)
				continue;

			SamplerStateType defaultSampler = mShader->getDefaultSampler(param.second.defaultValueIdx);
			getParamSamplerState(param.first).set(defaultSampler);
		}
	}

	template <bool Core>
	template <typename T>
	void TMaterial<Core>::getParam(const String& name, TMaterialDataParam<T, Core>& output) const
	{
		throwIfNotInitialized();

		output = TMaterialDataParam<T, Core>(name, getMaterialPtr(this));
	}

	template<bool Core>
	void TMaterial<Core>::throwIfNotInitialized() const
	{
		if (mShader == nullptr)
		{
			BS_EXCEPT(InternalErrorException, "Material does not have shader set.");
		}

		if (mTechniques.size() == 0)
		{
			BS_EXCEPT(InternalErrorException, "Shader does not contain a supported technique.");
		}
	}

	template class TMaterial < false > ;
	template class TMaterial < true > ;

	template BS_CORE_EXPORT void TMaterial<false>::getParam(const String&, TMaterialDataParam<float, false>&) const;
	template BS_CORE_EXPORT void TMaterial<false>::getParam(const String&, TMaterialDataParam<int, false>&) const;
	template BS_CORE_EXPORT void TMaterial<false>::getParam(const String&, TMaterialDataParam<Color, false>&) const;
	template BS_CORE_EXPORT void TMaterial<false>::getParam(const String&, TMaterialDataParam<Vector2, false>&) const;
	template BS_CORE_EXPORT void TMaterial<false>::getParam(const String&, TMaterialDataParam<Vector3, false>&) const;
	template BS_CORE_EXPORT void TMaterial<false>::getParam(const String&, TMaterialDataParam<Vector4, false>&) const;
	template BS_CORE_EXPORT void TMaterial<false>::getParam(const String&, TMaterialDataParam<Vector2I, false>&) const;
	template BS_CORE_EXPORT void TMaterial<false>::getParam(const String&, TMaterialDataParam<Vector3I, false>&) const;
	template BS_CORE_EXPORT void TMaterial<false>::getParam(const String&, TMaterialDataParam<Vector4I, false>&) const;
	template BS_CORE_EXPORT void TMaterial<false>::getParam(const String&, TMaterialDataParam<Matrix2, false>&) const;
	template BS_CORE_EXPORT void TMaterial<false>::getParam(const String&, TMaterialDataParam<Matrix2x3, false>&) const;
	template BS_CORE_EXPORT void TMaterial<false>::getParam(const String&, TMaterialDataParam<Matrix2x4, false>&) const;
	template BS_CORE_EXPORT void TMaterial<false>::getParam(const String&, TMaterialDataParam<Matrix3, false>&) const;
	template BS_CORE_EXPORT void TMaterial<false>::getParam(const String&, TMaterialDataParam<Matrix3x2, false>&) const;
	template BS_CORE_EXPORT void TMaterial<false>::getParam(const String&, TMaterialDataParam<Matrix3x4, false>&) const;
	template BS_CORE_EXPORT void TMaterial<false>::getParam(const String&, TMaterialDataParam<Matrix4, false>&) const;
	template BS_CORE_EXPORT void TMaterial<false>::getParam(const String&, TMaterialDataParam<Matrix4x2, false>&) const;
	template BS_CORE_EXPORT void TMaterial<false>::getParam(const String&, TMaterialDataParam<Matrix4x3, false>&) const;

	template BS_CORE_EXPORT void TMaterial<true>::getParam(const String&, TMaterialDataParam<float, true>&) const;
	template BS_CORE_EXPORT void TMaterial<true>::getParam(const String&, TMaterialDataParam<int, true>&) const;
	template BS_CORE_EXPORT void TMaterial<true>::getParam(const String&, TMaterialDataParam<Color, true>&) const;
	template BS_CORE_EXPORT void TMaterial<true>::getParam(const String&, TMaterialDataParam<Vector2, true>&) const;
	template BS_CORE_EXPORT void TMaterial<true>::getParam(const String&, TMaterialDataParam<Vector3, true>&) const;
	template BS_CORE_EXPORT void TMaterial<true>::getParam(const String&, TMaterialDataParam<Vector4, true>&) const;
	template BS_CORE_EXPORT void TMaterial<true>::getParam(const String&, TMaterialDataParam<Vector2I, true>&) const;
	template BS_CORE_EXPORT void TMaterial<true>::getParam(const String&, TMaterialDataParam<Vector3I, true>&) const;
	template BS_CORE_EXPORT void TMaterial<true>::getParam(const String&, TMaterialDataParam<Vector4I, true>&) const;
	template BS_CORE_EXPORT void TMaterial<true>::getParam(const String&, TMaterialDataParam<Matrix2, true>&) const;
	template BS_CORE_EXPORT void TMaterial<true>::getParam(const String&, TMaterialDataParam<Matrix2x3, true>&) const;
	template BS_CORE_EXPORT void TMaterial<true>::getParam(const String&, TMaterialDataParam<Matrix2x4, true>&) const;
	template BS_CORE_EXPORT void TMaterial<true>::getParam(const String&, TMaterialDataParam<Matrix3, true>&) const;
	template BS_CORE_EXPORT void TMaterial<true>::getParam(const String&, TMaterialDataParam<Matrix3x2, true>&) const;
	template BS_CORE_EXPORT void TMaterial<true>::getParam(const String&, TMaterialDataParam<Matrix3x4, true>&) const;
	template BS_CORE_EXPORT void TMaterial<true>::getParam(const String&, TMaterialDataParam<Matrix4, true>&) const;
	template BS_CORE_EXPORT void TMaterial<true>::getParam(const String&, TMaterialDataParam<Matrix4x2, true>&) const;
	template BS_CORE_EXPORT void TMaterial<true>::getParam(const String&, TMaterialDataParam<Matrix4x3, true>&) const;

	Material::Material()
		:mLoadFlags(Load_None)
	{ }

	Material::Material(const HShader& shader)
		:mLoadFlags(Load_None)
	{
		mShader = shader;
	}

	void Material::initialize()
	{
		_markResourcesDirty();
		initializeIfLoaded();

		Resource::initialize();
	}

	void Material::setShader(const HShader& shader)
	{
		if (mShader == shader)
			return;

		mShader = shader;
		mTechniques.clear();
		mLoadFlags = Load_None;

		// Make sure to clear params, because the default behaviour is to re-apply them (which won't work due to changed
		// shader)
		mParams = nullptr;

		_markResourcesDirty();

		initializeIfLoaded();
	}

	void Material::_markCoreDirty(MaterialDirtyFlags flags)
	{
		markCoreDirty((UINT32)flags);
	}

	void Material::_markDependenciesDirty()
	{
		markDependenciesDirty();
	}

	void Material::_markResourcesDirty()
	{
		markListenerResourcesDirty();
	}

	SPtr<ct::Material> Material::getCore() const
	{
		return std::static_pointer_cast<ct::Material>(mCoreSpecific);
	}

	SPtr<ct::CoreObject> Material::createCore() const
	{
		ct::Material* material = nullptr;

		SPtr<ct::Shader> shader;
		if (mShader.isLoaded())
		{
			shader = mShader->getCore();

			Vector<SPtr<ct::Technique>> techniques(mTechniques.size());
			for (UINT32 i = 0; i < (UINT32)mTechniques.size(); i++)
				techniques[i] = mTechniques[i]->getCore();
			
			SPtr<ct::MaterialParams> materialParams = bs_shared_ptr_new<ct::MaterialParams>(shader, mParams);

			material = new (bs_alloc<ct::Material>()) ct::Material(shader, techniques, materialParams);
		}
		
		if (material == nullptr)
			material = new (bs_alloc<ct::Material>()) ct::Material(shader);

		SPtr<ct::Material> materialPtr = bs_shared_ptr<ct::Material>(material);
		materialPtr->_setThisPtr(materialPtr);

		return materialPtr;
	}

	CoreSyncData Material::syncToCore(FrameAlloc* allocator)
	{
		bool syncAllParams = (getCoreDirtyFlags() & (UINT32)MaterialDirtyFlags::ParamResource) != 0;

		UINT32 paramsSize = 0;
		if (mParams != nullptr)
			mParams->getSyncData(nullptr, paramsSize, syncAllParams);

		UINT32 numTechniques = (UINT32)mTechniques.size();
		UINT32 size = sizeof(bool) + sizeof(UINT32) * 2 + sizeof(SPtr<ct::Shader>) + 
			sizeof(SPtr<ct::Technique>) * numTechniques + paramsSize;

		UINT8* buffer = allocator->alloc(size);
		char* dataPtr = (char*)buffer;

		dataPtr = rttiWriteElem(syncAllParams, dataPtr);
		
		SPtr<ct::Shader>* shader = new (dataPtr)SPtr<ct::Shader>();
		if (mShader.isLoaded(false))
			*shader = mShader->getCore();
		else
			*shader = nullptr;

		dataPtr += sizeof(SPtr<ct::Shader>);
		dataPtr = rttiWriteElem(numTechniques, dataPtr);

		for(UINT32 i = 0; i < numTechniques; i++)
		{
			SPtr<ct::Technique>* technique = new (dataPtr) SPtr<ct::Technique>();
			*technique = mTechniques[i]->getCore();

			dataPtr += sizeof(SPtr<ct::Technique>);
		}

		dataPtr = rttiWriteElem(paramsSize, dataPtr);
		if (mParams != nullptr)
			mParams->getSyncData((UINT8*)dataPtr, paramsSize, syncAllParams);

		dataPtr += paramsSize;

		return CoreSyncData(buffer, size);
	}

	void Material::getCoreDependencies(Vector<CoreObject*>& dependencies)
	{
		if (mShader.isLoaded())
			dependencies.push_back(mShader.get());

		if(mParams != nullptr)
			mParams->getCoreObjectDependencies(dependencies);
	}

	void Material::getListenerResources(Vector<HResource>& resources)
	{
		if (mShader != nullptr)
			resources.push_back(mShader);

		if (mParams != nullptr)
			mParams->getResourceDependencies(resources);
	}

	void Material::getResourceDependencies(FrameVector<HResource>& dependencies) const
	{
		if (mShader != nullptr)
			dependencies.push_back(mShader);
	}

	void Material::initializeIfLoaded()
	{
		if (areDependenciesLoaded())
		{
			if (mLoadFlags != Load_All)
			{
				mLoadFlags = Load_All;

				// Shader about to change, so save parameters, rebuild material and restore parameters
				SPtr<MaterialParams> oldParams = mParams;

				initializeTechniques();
				markCoreDirty();

				if (mTechniques.size() == 0) // Wasn't initialized
					return;

				if(oldParams)
					setParams(oldParams);
			}
		}
		else
		{
			if (mShader.isLoaded() && mLoadFlags == Load_None)
			{
				mLoadFlags = Load_Shader;
				markListenerResourcesDirty(); // Need to register resources dependent on shader now
			}
		}
	}

	void Material::notifyResourceLoaded(const HResource& resource)
	{
		// Ready to initialize as soon as shader loads
		if (resource->getRTTI()->getRTTIId() == TID_Shader)
			initializeIfLoaded();
	}

	void Material::notifyResourceChanged(const HResource& resource)
	{
		// Need full rebuild if shader changed
		if (resource->getRTTI()->getRTTIId() == TID_Shader)
		{
			mLoadFlags = Load_None;
			initializeIfLoaded();
		}
		else
		{
			// Otherwise just sync changes (most likely just a texture got reimported)
			_markCoreDirty(MaterialDirtyFlags::ParamResource);
		}
	}

	HMaterial Material::clone()
	{
		UINT32 bufferSize = 0;

		MemorySerializer serializer;
		UINT8* buffer = serializer.encode(this, bufferSize, (void*(*)(size_t))&bs_alloc);

		SPtr<Material> cloneObj = std::static_pointer_cast<Material>(serializer.decode(buffer, bufferSize));
		bs_free(buffer);

		return static_resource_cast<Material>(gResources()._createResourceHandle(cloneObj));
	}

	template<class T>
	void copyParam(const SPtr<MaterialParams>& from, Material* to, const String& name, 
		const MaterialParams::ParamData& paramRef, UINT32 arraySize)
	{
		TMaterialDataParam<T, false> param;
		to->getParam(name, param);

		T paramData;
		for (UINT32 i = 0; i < arraySize; i++)
		{
			from->getDataParam(paramRef, i, paramData);
			param.set(paramData, i);
		}
	}

	void Material::setParams(const SPtr<MaterialParams>& params)
	{
		if (params == nullptr)
			return;

		std::function<
			void(const SPtr<MaterialParams>&, Material*, const String&, const MaterialParams::ParamData&, UINT32)> 
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

		auto& dataParams = mShader->getDataParams();
		for (auto& param : dataParams)
		{
			UINT32 arraySize = param.second.arraySize > 1 ? param.second.arraySize : 1;

			const MaterialParams::ParamData* paramData = nullptr;
			auto result = params->getParamData(param.first, MaterialParams::ParamType::Data, param.second.type, 0, &paramData);

			if (result != MaterialParams::GetParamResult::Success)
				continue;

			UINT32 elemsToCopy = std::min(arraySize, paramData->arraySize);

			auto& copyFunction = copyParamLookup[param.second.type];
			if (copyFunction != nullptr)
				copyFunction(params, this, param.first, *paramData, elemsToCopy);
			else
			{
				if(param.second.type == GPDT_STRUCT)
				{
					TMaterialParamStruct<false> curParam = getParamStruct(param.first);

					UINT32 structSize = params->getStructSize(*paramData);
					if (param.second.elementSize != structSize)
						continue;

					UINT8* structData = (UINT8*)bs_stack_alloc(structSize);
					for (UINT32 i = 0; i < elemsToCopy; i++)
					{
						params->getStructData(*paramData, structData, structSize, i);
						curParam.set(structData, structSize, i);
					}

					bs_stack_free(structData);
				}
			}

			for(UINT32 i = 0; i < arraySize; i++)
			{
				const bool isAnimated = params->isAnimated(*paramData, i);
				if(!isAnimated)
					continue;

				if(param.second.type == GPDT_FLOAT1)
				{
					TMaterialCurveParam<float, false> curParam = getParamFloatCurve(param.first);
					curParam.set(params->getCurveParam<float>(*paramData, i), i);
				}
				else if(param.second.type == GPDT_COLOR)
				{
					TMaterialColorGradientParam<false> curParam = getParamColorGradient(param.first);
					curParam.set(params->getColorGradientParam(*paramData, i), i);
				}
			}
		}

		auto& textureParams = mShader->getTextureParams();
		for (auto& param : textureParams)
		{
			const MaterialParams::ParamData* paramData = nullptr;
			auto result = params->getParamData(param.first, MaterialParams::ParamType::Texture, GPDT_UNKNOWN, 0, &paramData);

			if (result != MaterialParams::GetParamResult::Success)
				continue;

			MateralParamTextureType texType = params->getTextureType(*paramData);
			switch(texType)
			{
			default:
			case MateralParamTextureType::Normal: 
			{
				TMaterialParamTexture<false> curParam = getParamTexture(param.first);

				HTexture texture;
				TextureSurface surface;
				params->getTexture(*paramData, texture, surface);
				curParam.set(texture);
			}
				break;
			case MateralParamTextureType::LoadStore:
			{
				TMaterialParamLoadStoreTexture<false> curParam = getParamLoadStoreTexture(param.first);

				HTexture texture;
				TextureSurface surface;
				params->getLoadStoreTexture(*paramData, texture, surface);
				curParam.set(texture, surface);
			}
			break;
			case MateralParamTextureType::Sprite:
			{
				TMaterialParamSpriteTexture<false> curParam = getParamSpriteTexture(param.first);

				HSpriteTexture texture;
				params->getSpriteTexture(*paramData, texture);
				curParam.set(texture);
			}
			break;
			}
		}

		auto& bufferParams = mShader->getBufferParams();
		for (auto& param : bufferParams)
		{
			const MaterialParams::ParamData* paramData = nullptr;
			auto result = params->getParamData(param.first, MaterialParams::ParamType::Buffer, GPDT_UNKNOWN, 0, &paramData);

			if (result != MaterialParams::GetParamResult::Success)
				continue;

			TMaterialParamBuffer<false> curParam = getParamBuffer(param.first);

			SPtr<GpuBuffer> buffer;
			params->getBuffer(*paramData, buffer);
			curParam.set(buffer);
		}

		auto& samplerParams = mShader->getSamplerParams();
		for (auto& param : samplerParams)
		{
			const MaterialParams::ParamData* paramData = nullptr;
			auto result = params->getParamData(param.first, MaterialParams::ParamType::Sampler, GPDT_UNKNOWN, 0, &paramData);

			if (result != MaterialParams::GetParamResult::Success)
				continue;

			TMaterialParamSampState<false> curParam = getParamSamplerState(param.first);

			SPtr<SamplerState> samplerState;
			params->getSamplerState(*paramData, samplerState);
			curParam.set(samplerState);
		}
	}

	HMaterial Material::create()
	{
		SPtr<Material> materialPtr = MaterialManager::instance().create();

		return static_resource_cast<Material>(gResources()._createResourceHandle(materialPtr));
	}

	HMaterial Material::create(const HShader& shader)
	{
		SPtr<Material> materialPtr = MaterialManager::instance().create(shader);

		return static_resource_cast<Material>(gResources()._createResourceHandle(materialPtr));
	}

	RTTITypeBase* Material::getRTTIStatic()
	{
		return MaterialRTTI::instance();
	}

	RTTITypeBase* Material::getRTTI() const
	{
		return Material::getRTTIStatic();
	}

	namespace ct
	{
	Material::Material(const SPtr<Shader>& shader)
	{
		setShader(shader);
	}
	
	Material::Material(const SPtr<Shader>& shader, const ShaderVariation& variation)
	{
		setShader(shader, variation);
	}

	Material::Material(const SPtr<Shader>& shader, const Vector<SPtr<Technique>>& techniques,
		const SPtr<MaterialParams>& materialParams)
	{
		mShader = shader;
		mParams = materialParams;
		mTechniques = techniques;
	}

	void Material::setShader(const SPtr<Shader>& shader)
	{
		mShader = shader;

		initializeTechniques();
		_markCoreDirty(MaterialDirtyFlags::Shader);
	}
	
	void Material::setShader(const SPtr<Shader>& shader, const ShaderVariation& variation)
	{
		mShader = shader;

		initializeTechniques(false, variation);
		_markCoreDirty(MaterialDirtyFlags::Shader);
	}

	void Material::syncToCore(const CoreSyncData& data)
	{
		char* dataPtr = (char*)data.getBuffer();

		bool syncAllParams;
		dataPtr = rttiReadElem(syncAllParams, dataPtr);

		if(syncAllParams)
			mParams = nullptr;

		SPtr<Shader>* shader = (SPtr<Shader>*)dataPtr;

		mShader = *shader;
		shader->~SPtr<Shader>();
		dataPtr += sizeof(SPtr<Shader>);

		UINT32 numTechniques;
		dataPtr = rttiReadElem(numTechniques, dataPtr);

		mTechniques.resize(numTechniques);
		for(UINT32 i = 0; i < numTechniques; i++)
		{
			SPtr<Technique>* technique = (SPtr<Technique>*)dataPtr;
			mTechniques[i] = *technique;
			technique->~SPtr<Technique>();
			dataPtr += sizeof(SPtr<Technique>);
		}

		UINT32 paramsSize = 0;
		dataPtr = rttiReadElem(paramsSize, dataPtr);
		if (mParams == nullptr && mShader != nullptr)
			mParams = bs_shared_ptr_new<MaterialParams>(mShader);

		if(mParams != nullptr && paramsSize > 0)
			mParams->setSyncData((UINT8*)dataPtr, paramsSize);

		dataPtr += paramsSize;
	}

	SPtr<Material> Material::create(const SPtr<Shader>& shader)
	{
		Material* material = new (bs_alloc<Material>()) Material(shader);
		SPtr<Material> materialPtr = bs_shared_ptr<Material>(material);
		materialPtr->_setThisPtr(materialPtr);
		materialPtr->initialize();

		return materialPtr;
	}
	
	SPtr<Material> Material::create(const SPtr<Shader>& shader, const ShaderVariation& variation)
	{
		Material* material = new (bs_alloc<Material>()) Material(shader, variation);
		SPtr<Material> materialPtr = bs_shared_ptr<Material>(material);
		materialPtr->_setThisPtr(materialPtr);
		materialPtr->initialize();

		return materialPtr;
	}
	}
}
