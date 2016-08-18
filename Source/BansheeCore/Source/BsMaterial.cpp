//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsMaterial.h"
#include "BsException.h"
#include "BsShader.h"
#include "BsTechnique.h"
#include "BsPass.h"
#include "BsRenderAPI.h"
#include "BsHardwareBufferManager.h"
#include "BsGpuProgram.h"
#include "BsGpuParamBlockBuffer.h"
#include "BsGpuParamDesc.h"
#include "BsMaterialRTTI.h"
#include "BsMaterialManager.h"
#include "BsDebug.h"
#include "BsResources.h"
#include "BsFrameAlloc.h"
#include "BsMatrixNxM.h"
#include "BsVectorNI.h"
#include "BsMemorySerializer.h"
#include "BsMaterialParams.h"

namespace BansheeEngine
{
	struct ShaderBlockDesc
	{
		String name;
		GpuParamBlockUsage usage;
		int size;
		bool create;
	};

	enum MaterialLoadFlags
	{
		Load_None	= 0,
		Load_Shader	= 1,
		Load_All	= 2
	};

	const UINT32 PassParameters::NUM_PARAMS = 6;
	const UINT32 PassParametersCore::NUM_PARAMS = 6;
	
	SPtr<PassParametersCore> convertParamsToCore(const SPtr<PassParameters>& passParams)
	{
		SPtr<PassParametersCore> passParameters = bs_shared_ptr_new<PassParametersCore>();

		if (passParams->mVertParams != nullptr)
			passParameters->mVertParams = passParams->mVertParams->getCore();
		else
			passParameters->mVertParams = nullptr;

		if (passParams->mFragParams != nullptr)
			passParameters->mFragParams = passParams->mFragParams->getCore();
		else
			passParameters->mFragParams = nullptr;

		if (passParams->mGeomParams != nullptr)
			passParameters->mGeomParams = passParams->mGeomParams->getCore();
		else
			passParameters->mGeomParams = nullptr;

		if (passParams->mHullParams != nullptr)
			passParameters->mHullParams = passParams->mHullParams->getCore();
		else
			passParameters->mHullParams = nullptr;

		if (passParams->mDomainParams != nullptr)
			passParameters->mDomainParams = passParams->mDomainParams->getCore();
		else
			passParameters->mDomainParams = nullptr;

		if (passParams->mComputeParams != nullptr)
			passParameters->mComputeParams = passParams->mComputeParams->getCore();
		else
			passParameters->mComputeParams = nullptr;

		return passParameters;
	}

	bool areParamsEqual(const GpuParamDataDesc& paramA, const GpuParamDataDesc& paramB, bool ignoreBufferOffsets)
	{
		bool equal = paramA.arraySize == paramB.arraySize && paramA.elementSize == paramB.elementSize
			&& paramA.type == paramB.type && paramA.arrayElementStride == paramB.arrayElementStride;

		if (!ignoreBufferOffsets)
			equal &= paramA.cpuMemOffset == paramB.cpuMemOffset && paramA.gpuMemOffset == paramB.gpuMemOffset;

		return equal;
	}

	Set<String> determineValidShareableParamBlocks(const Vector<SPtr<GpuParamDesc>>& paramDescs)
	{
		// Make sure param blocks with the same name actually are the same
		Map<String, std::pair<String, SPtr<GpuParamDesc>>> uniqueParamBlocks;
		Map<String, bool> validParamBlocks;

		for (auto iter = paramDescs.begin(); iter != paramDescs.end(); ++iter)
		{
			const GpuParamDesc& curDesc = **iter;
			for (auto blockIter = curDesc.paramBlocks.begin(); blockIter != curDesc.paramBlocks.end(); ++blockIter)
			{
				bool isBlockValid = true;
				const GpuParamBlockDesc& curBlock = blockIter->second;

				if (!curBlock.isShareable) // Non-shareable buffers are handled differently, they're allowed same names
					continue;

				auto iterFind = uniqueParamBlocks.find(blockIter->first);
				if (iterFind == uniqueParamBlocks.end())
				{
					uniqueParamBlocks[blockIter->first] = std::make_pair(blockIter->first, *iter);
					validParamBlocks[blockIter->first] = true;
					continue;
				}

				String otherBlockName = iterFind->second.first;
				SPtr<GpuParamDesc> otherDesc = iterFind->second.second;

				for (auto myParamIter = curDesc.params.begin(); myParamIter != curDesc.params.end(); ++myParamIter)
				{
					const GpuParamDataDesc& myParam = myParamIter->second;

					if (myParam.paramBlockSlot != curBlock.slot)
						continue; // Param is in another block, so we will check it when its time for that block

					auto otherParamFind = otherDesc->params.find(myParamIter->first);

					// Cannot find other param, blocks aren't equal
					if (otherParamFind == otherDesc->params.end())
					{
						isBlockValid = false;
						break;
					}

					const GpuParamDataDesc& otherParam = otherParamFind->second;

					if (!areParamsEqual(myParam, otherParam, false) || curBlock.name != otherBlockName)
					{
						isBlockValid = false;
						break;
					}
				}

				if (!isBlockValid)
				{
					if (validParamBlocks[blockIter->first])
					{
						LOGWRN("Found two param blocks with the same name but different contents: " + blockIter->first);
						validParamBlocks[blockIter->first] = false;
					}
				}
			}
		}

		Set<String> validParamBlocksReturn;
		for (auto iter = validParamBlocks.begin(); iter != validParamBlocks.end(); ++iter)
		{
			if (iter->second)
				validParamBlocksReturn.insert(iter->first);
		}

		return validParamBlocksReturn;
	}

	Vector<ShaderBlockDesc> determineShaderBlockData(const Set<String>& paramBlocks, const Vector<SPtr<GpuParamDesc>>& paramDescs,
		const Map<String, SHADER_PARAM_BLOCK_DESC>& shaderDesc)
	{
		Vector<ShaderBlockDesc> output;
		for (auto iter = paramBlocks.begin(); iter != paramBlocks.end(); ++iter)
		{
			ShaderBlockDesc shaderBlockDesc;
			shaderBlockDesc.create = true;
			shaderBlockDesc.usage = GPBU_STATIC;
			shaderBlockDesc.size = 0;
			shaderBlockDesc.name = *iter;

			auto iterFind = shaderDesc.find(*iter);
			if (iterFind != shaderDesc.end())
			{
				shaderBlockDesc.create = !iterFind->second.shared && iterFind->second.rendererSemantic == StringID::NONE;
				shaderBlockDesc.usage = iterFind->second.usage;
			}

			for (auto iter2 = paramDescs.begin(); iter2 != paramDescs.end(); ++iter2)
			{
				auto findParamBlockDesc = (*iter2)->paramBlocks.find(*iter);

				if (findParamBlockDesc != (*iter2)->paramBlocks.end())
				{
					shaderBlockDesc.size = findParamBlockDesc->second.blockSize * sizeof(UINT32);
					break;
				}
			}

			output.push_back(shaderBlockDesc);
		}

		return output;
	}

	template<class T>
	bool isShaderValid(const T& shader) { return false; }

	template<>
	bool isShaderValid(const HShader& shader) { return shader.isLoaded(); }

	template<>
	bool isShaderValid(const SPtr<ShaderCore>& shader) { return shader != nullptr; }

	Vector<SPtr<GpuParamDesc>> MaterialBase::getAllParamDescs(const SPtr<Technique>& technique)
	{
		Vector<SPtr<GpuParamDesc>> allParamDescs;

		// Make sure all gpu programs are fully loaded
		for (UINT32 i = 0; i < technique->getNumPasses(); i++)
		{
			SPtr<Pass> curPass = technique->getPass(i);

			SPtr<GpuProgram> vertProgram = curPass->getVertexProgram();
			if (vertProgram)
			{
				vertProgram->blockUntilCoreInitialized();
				allParamDescs.push_back(vertProgram->getParamDesc());
			}

			SPtr<GpuProgram> fragProgram = curPass->getFragmentProgram();
			if (fragProgram)
			{
				fragProgram->blockUntilCoreInitialized();
				allParamDescs.push_back(fragProgram->getParamDesc());
			}

			SPtr<GpuProgram> geomProgram = curPass->getGeometryProgram();
			if (geomProgram)
			{
				geomProgram->blockUntilCoreInitialized();
				allParamDescs.push_back(geomProgram->getParamDesc());
			}

			SPtr<GpuProgram> hullProgram = curPass->getHullProgram();
			if (hullProgram)
			{
				hullProgram->blockUntilCoreInitialized();
				allParamDescs.push_back(hullProgram->getParamDesc());
			}

			SPtr<GpuProgram> domainProgram = curPass->getDomainProgram();
			if (domainProgram)
			{
				domainProgram->blockUntilCoreInitialized();
				allParamDescs.push_back(domainProgram->getParamDesc());
			}

			SPtr<GpuProgram> computeProgram = curPass->getComputeProgram();
			if (computeProgram)
			{
				computeProgram->blockUntilCoreInitialized();
				allParamDescs.push_back(computeProgram->getParamDesc());
			}
		}

		return allParamDescs;
	}

	Vector<SPtr<GpuParamDesc>> MaterialBase::getAllParamDescs(const SPtr<TechniqueCore>& technique)
	{
		Vector<SPtr<GpuParamDesc>> allParamDescs;

		// Make sure all gpu programs are fully loaded
		for (UINT32 i = 0; i < technique->getNumPasses(); i++)
		{
			SPtr<PassCore> curPass = technique->getPass(i);

			SPtr<GpuProgramCore> vertProgram = curPass->getVertexProgram();
			if (vertProgram)
				allParamDescs.push_back(vertProgram->getParamDesc());

			SPtr<GpuProgramCore> fragProgram = curPass->getFragmentProgram();
			if (fragProgram)
				allParamDescs.push_back(fragProgram->getParamDesc());

			SPtr<GpuProgramCore> geomProgram = curPass->getGeometryProgram();
			if (geomProgram)
				allParamDescs.push_back(geomProgram->getParamDesc());

			SPtr<GpuProgramCore> hullProgram = curPass->getHullProgram();
			if (hullProgram)
				allParamDescs.push_back(hullProgram->getParamDesc());

			SPtr<GpuProgramCore> domainProgram = curPass->getDomainProgram();
			if (domainProgram)
				allParamDescs.push_back(domainProgram->getParamDesc());

			SPtr<GpuProgramCore> computeProgram = curPass->getComputeProgram();
			if (computeProgram)
				allParamDescs.push_back(computeProgram->getParamDesc());
		}

		return allParamDescs;
	}

	template<bool Core>
	UINT32 TMaterial<Core>::findTechnique(const StringID& tag)
	{
		for(UINT32 i = 0; i < (UINT32)mTechniques.size(); i++)
		{
			if (mTechniques[i]->hasTag(tag))
				return i;
		}

		return -1;
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

		SPtr<Vector<TGpuParamStruct<Core>>> gpuParams;
		return TMaterialParamStruct<Core>(name, mParams, gpuParams);
	}

	template<bool Core>
	TMaterialParamTexture<Core> TMaterial<Core>::getParamTexture(const String& name) const
	{
		throwIfNotInitialized();

		SPtr<Vector<TGpuParamTexture<Core>>> gpuParams;

		if (mShader != nullptr)
		{
			const auto& params = mShader->getTextureParams();
			auto iterFind = params.find(name);
			if (iterFind != params.end())
			{
				for (auto iter = mParametersPerPass.begin(); iter != mParametersPerPass.end(); ++iter)
				{
					SPtr<PassParamsType> params = *iter;
					for (UINT32 i = 0; i < PassParamsType::NUM_PARAMS; i++)
					{
						GpuParamsType& paramPtr = params->getParamByIdx(i);
						if (paramPtr)
						{
							for (auto& gpuVarName : iterFind->second.gpuVariableNames)
							{
								if (paramPtr->hasTexture(gpuVarName))
								{
									if (gpuParams == nullptr)
										gpuParams = bs_shared_ptr_new<Vector<TGpuParamTexture<Core>>>();

									gpuParams->push_back(TGpuParamTexture<Core>());
									paramPtr->getTextureParam(gpuVarName, gpuParams->back());

									break;
								}
							}
						}
					}
				}
			}
		}

		return TMaterialParamTexture<Core>(name, mParams, gpuParams);
	}

	template<bool Core>
	TMaterialParamLoadStoreTexture<Core> TMaterial<Core>::getParamLoadStoreTexture(const String& name) const
	{
		throwIfNotInitialized();

		SPtr<Vector<TGpuParamLoadStoreTexture<Core>>> gpuParams;

		if (mShader != nullptr)
		{
			const auto& params = mShader->getTextureParams();
			auto iterFind = params.find(name);
			if (iterFind != params.end())
			{
				for (auto iter = mParametersPerPass.begin(); iter != mParametersPerPass.end(); ++iter)
				{
					SPtr<PassParamsType> params = *iter;
					for (UINT32 i = 0; i < PassParamsType::NUM_PARAMS; i++)
					{
						GpuParamsType& paramPtr = params->getParamByIdx(i);
						if (paramPtr)
						{
							for (auto& gpuVarName : iterFind->second.gpuVariableNames)
							{
								if (paramPtr->hasLoadStoreTexture(gpuVarName))
								{
									if (gpuParams == nullptr)
										gpuParams = bs_shared_ptr_new<Vector<TGpuParamLoadStoreTexture<Core>>>();

									gpuParams->push_back(TGpuParamLoadStoreTexture<Core>());
									paramPtr->getLoadStoreTextureParam(gpuVarName, gpuParams->back());

									break;
								}
							}
						}
					}
				}
			}
		}

		return TMaterialParamLoadStoreTexture<Core>(name, mParams, gpuParams);
	}

	template<bool Core>
	TMaterialParamBuffer<Core> TMaterial<Core>::getParamBuffer(const String& name) const
	{
		throwIfNotInitialized();

		SPtr<Vector<TGpuParamBuffer<Core>>> gpuParams;

		if (mShader != nullptr)
		{
			const auto& params = mShader->getBufferParams();
			auto iterFind = params.find(name);
			if (iterFind != params.end())
			{
				for (auto iter = mParametersPerPass.begin(); iter != mParametersPerPass.end(); ++iter)
				{
					SPtr<PassParamsType> params = *iter;
					for (UINT32 i = 0; i < PassParamsType::NUM_PARAMS; i++)
					{
						GpuParamsType& paramPtr = params->getParamByIdx(i);
						if (paramPtr)
						{
							for (auto& gpuVarName : iterFind->second.gpuVariableNames)
							{
								if (paramPtr->hasBuffer(gpuVarName))
								{
									if (gpuParams == nullptr)
										gpuParams = bs_shared_ptr_new<Vector<TGpuParamBuffer<Core>>>();

									gpuParams->push_back(TGpuParamBuffer<Core>());
									paramPtr->getBufferParam(gpuVarName, gpuParams->back());

									break;
								}
							}
						}
					}
				}
			}
		}

		return TMaterialParamBuffer<Core>(name, mParams, gpuParams);
	}

	template<bool Core>
	TMaterialParamSampState<Core> TMaterial<Core>::getParamSamplerState(const String& name) const
	{
		throwIfNotInitialized();

		SPtr<Vector<TGpuParamSampState<Core>>> gpuParams;

		if (mShader != nullptr)
		{
			const auto& params = mShader->getSamplerParams();
			auto iterFind = params.find(name);
			if (iterFind != params.end())
			{
				for (auto iter = mParametersPerPass.begin(); iter != mParametersPerPass.end(); ++iter)
				{
					SPtr<PassParamsType> params = *iter;
					for (UINT32 i = 0; i < PassParamsType::NUM_PARAMS; i++)
					{
						GpuParamsType& paramPtr = params->getParamByIdx(i);
						if (paramPtr)
						{
							for (auto& gpuVarName : iterFind->second.gpuVariableNames)
							{
								if (paramPtr->hasSamplerState(gpuVarName))
								{
									if(gpuParams == nullptr)
										gpuParams = bs_shared_ptr_new<Vector<TGpuParamSampState<Core>>>();

									gpuParams->push_back(TGpuParamSampState<Core>());
									paramPtr->getSamplerStateParam(gpuVarName, gpuParams->back());

									break;
								}
							}
						}
					}
				}
			}
		}

		return TMaterialParamSampState<Core>(name, mParams, gpuParams);
	}

	template<bool Core>
	void TMaterial<Core>::setParamBlockBuffer(const String& name, const ParamBlockPtrType& paramBlock)
	{
		for (auto iter = mParametersPerPass.begin(); iter != mParametersPerPass.end(); ++iter)
		{
			SPtr<PassParamsType> params = *iter;

			for (UINT32 i = 0; i < PassParamsType::NUM_PARAMS; i++)
			{
				GpuParamsType& paramPtr = params->getParamByIdx(i);
				if (paramPtr)
				{
					if (paramPtr->hasParamBlock(name))
						paramPtr->setParamBlockBuffer(name, paramBlock);
				}
			}
		}
	}

	template<bool Core>
	void TMaterial<Core>::initializeTechniques()
	{
		mTechniques.clear();
		mParametersPerPass.clear();

		if (isShaderValid(mShader))
		{
			mParams = bs_shared_ptr_new<MaterialParamsType>(mShader);
			mTechniques = mShader->getCompatibleTechniques();

			if (mTechniques.size() == 0)
				return;

			Vector<SPtr<GpuParamDesc>> allParamDescs = getAllParamDescs(mTechniques[0]);

			// Fill out various helper structures
			Set<String> validShareableParamBlocks = determineValidShareableParamBlocks(allParamDescs);
			Vector<ShaderBlockDesc> paramBlockData = determineShaderBlockData(validShareableParamBlocks, allParamDescs, mShader->getParamBlocks());

			Map<String, ParamBlockPtrType> paramBlockBuffers;

			// Create param blocks
			for (auto& paramBlock : paramBlockData)
			{
				ParamBlockPtrType newParamBlockBuffer;
				if (paramBlock.create)
					newParamBlockBuffer = ParamBlockType::create(paramBlock.size, paramBlock.usage);

				paramBlockBuffers[paramBlock.name] = newParamBlockBuffer;
			}

			for (UINT32 i = 0; i < mTechniques[0]->getNumPasses(); i++)
			{
				SPtr<PassType> curPass = mTechniques[0]->getPass(i);
				SPtr<PassParamsType> params = SPtr<PassParamsType>(new PassParamsType());

				GpuProgramType vertProgram = curPass->getVertexProgram();
				if (vertProgram)
					params->mVertParams = vertProgram->createParameters();

				GpuProgramType fragProgram = curPass->getFragmentProgram();
				if (fragProgram)
					params->mFragParams = fragProgram->createParameters();

				GpuProgramType geomProgram = curPass->getGeometryProgram();
				if (geomProgram)
					params->mGeomParams = geomProgram->createParameters();

				GpuProgramType hullProgram = curPass->getHullProgram();
				if (hullProgram)
					params->mHullParams = hullProgram->createParameters();

				GpuProgramType domainProgram = curPass->getDomainProgram();
				if (domainProgram)
					params->mDomainParams = domainProgram->createParameters();

				GpuProgramType computeProgram = curPass->getComputeProgram();
				if (computeProgram)
					params->mComputeParams = computeProgram->createParameters();

				mParametersPerPass.push_back(params);
			}

			// Assign param block buffers
			for (auto iter = mParametersPerPass.begin(); iter != mParametersPerPass.end(); ++iter)
			{
				SPtr<PassParamsType> params = *iter;

				for (UINT32 i = 0; i < PassParamsType::NUM_PARAMS; i++)
				{
					GpuParamsType& paramPtr = params->getParamByIdx(i);
					if (paramPtr)
					{
						// Assign shareable buffers
						for (auto& block : paramBlockData)
						{
							const String& paramBlockName = block.name;
							if (paramPtr->hasParamBlock(paramBlockName))
							{
								ParamBlockPtrType blockBuffer = paramBlockBuffers[paramBlockName];

								paramPtr->setParamBlockBuffer(paramBlockName, blockBuffer);
							}
						}

						// Create non-shareable ones
						const GpuParamDesc& desc = paramPtr->getParamDesc();
						for (auto iterBlockDesc = desc.paramBlocks.begin(); iterBlockDesc != desc.paramBlocks.end(); ++iterBlockDesc)
						{
							if (!iterBlockDesc->second.isShareable)
							{
								ParamBlockPtrType newParamBlockBuffer = ParamBlockType::create(iterBlockDesc->second.blockSize * sizeof(UINT32));

								paramPtr->setParamBlockBuffer(iterBlockDesc->first, newParamBlockBuffer);
							}
						}
					}
				}
			}

			// Assign default parameters
			initDefaultParameters();
		}

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

		SPtr<Vector<TGpuDataParam<T, Core>>> gpuParams;

		if (mShader != nullptr)
		{
			const auto& params = mShader->getDataParams();
			auto iterFind = params.find(name);
			if (iterFind != params.end())
			{
				const String& gpuVarName = iterFind->second.gpuVariableName;
				gpuParams = bs_shared_ptr_new<Vector<TGpuDataParam<T, Core>>>();

				for (auto iter = mParametersPerPass.begin(); iter != mParametersPerPass.end(); ++iter)
				{
					SPtr<PassParamsType> params = *iter;

					for (UINT32 i = 0; i < PassParamsType::NUM_PARAMS; i++)
					{
						GpuParamsType& paramPtr = params->getParamByIdx(i);
						if (paramPtr)
						{
							if (paramPtr->hasParam(gpuVarName))
							{
								gpuParams->push_back(TGpuDataParam<T, Core>());
								paramPtr->template getParam<T>(gpuVarName, gpuParams->back());
							}
						}
					}
				}
			}
		}

		output = TMaterialDataParam<T, Core>(name, mParams, gpuParams);
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

	MaterialCore::MaterialCore(const SPtr<ShaderCore>& shader)
	{
		setShader(shader);
	}

	MaterialCore::MaterialCore(const SPtr<ShaderCore>& shader, const Vector<SPtr<TechniqueCore>>& techniques,
		const SPtr<MaterialParamsCore>& materialParams, const Vector<SPtr<PassParametersCore>>& passParams)
	{
		mShader = shader;
		mParams = materialParams;
		mTechniques = techniques;

		UINT32 numPassParams = (UINT32)passParams.size();
		mParametersPerPass.resize(numPassParams);
		for (UINT32 i = 0; i < numPassParams; i++)
			mParametersPerPass[i] = passParams[i];
	}

	void MaterialCore::setShader(const SPtr<ShaderCore>& shader)
	{
		mShader = shader;

		initializeTechniques();

		_markCoreDirty();
	}

	void MaterialCore::syncToCore(const CoreSyncData& data)
	{
		char* dataPtr = (char*)data.getBuffer();

		mParametersPerPass.clear();

		UINT32 numPasses = 0;
		dataPtr = rttiReadElem(numPasses, dataPtr);

		for (UINT32 i = 0; i < numPasses; i++)
		{
			SPtr<PassParametersCore>* passParameters = (SPtr<PassParametersCore>*)dataPtr;

			mParametersPerPass.push_back(*passParameters);

			passParameters->~SPtr<PassParametersCore>();
			dataPtr += sizeof(SPtr<PassParametersCore>);
		}

		SPtr<ShaderCore>* shader = (SPtr<ShaderCore>*)dataPtr;
		mShader = *shader;
		shader->~SPtr<ShaderCore>();
		dataPtr += sizeof(SPtr<ShaderCore>);

		UINT32 numTechniques;
		dataPtr = rttiReadElem(numTechniques, dataPtr);

		mTechniques.resize(numTechniques);
		for(UINT32 i = 0; i < numTechniques; i++)
		{
			SPtr<TechniqueCore>* technique = (SPtr<TechniqueCore>*)dataPtr;
			mTechniques[i] = *technique;
			technique->~SPtr<TechniqueCore>();
			dataPtr += sizeof(SPtr<TechniqueCore>);
		}

		UINT32 paramsSize = 0;
		dataPtr = rttiReadElem(paramsSize, dataPtr);
		if (mParams != nullptr)
			mParams->setSyncData((UINT8*)dataPtr, paramsSize);

		dataPtr += paramsSize;
	}

	SPtr<MaterialCore> MaterialCore::create(const SPtr<ShaderCore>& shader)
	{
		MaterialCore* material = new (bs_alloc<MaterialCore>()) MaterialCore(shader);
		SPtr<MaterialCore> materialPtr = bs_shared_ptr<MaterialCore>(material);
		materialPtr->_setThisPtr(materialPtr);
		materialPtr->initialize();

		return materialPtr;
	}

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
		_markResourcesDirty();

		initializeIfLoaded();
	}

	void Material::_markCoreDirty()
	{
		markCoreDirty();
	}

	void Material::_markDependenciesDirty()
	{
		markDependenciesDirty();
	}

	void Material::_markResourcesDirty()
	{
		markListenerResourcesDirty();
	}

	SPtr<MaterialCore> Material::getCore() const
	{
		return std::static_pointer_cast<MaterialCore>(mCoreSpecific);
	}

	SPtr<CoreObjectCore> Material::createCore() const
	{
		MaterialCore* material = nullptr;

		SPtr<ShaderCore> shader;
		if (mShader.isLoaded())
		{
			shader = mShader->getCore();

			Vector<SPtr<TechniqueCore>> techniques(mTechniques.size());
			for (UINT32 i = 0; i < (UINT32)mTechniques.size(); i++)
				techniques[i] = mTechniques[i]->getCore();
			
			Vector<SPtr<PassParametersCore>> passParams;
			for (auto& passParam : mParametersPerPass)
				passParams.push_back(convertParamsToCore(passParam));

			SPtr<MaterialParamsCore> materialParams = bs_shared_ptr_new<MaterialParamsCore>(shader, mParams);

			material = new (bs_alloc<MaterialCore>()) MaterialCore(shader, techniques, materialParams, passParams);
		}
		
		if (material == nullptr)
			material = new (bs_alloc<MaterialCore>()) MaterialCore(shader);

		SPtr<MaterialCore> materialPtr = bs_shared_ptr<MaterialCore>(material);
		materialPtr->_setThisPtr(materialPtr);

		return materialPtr;
	}

	CoreSyncData Material::syncToCore(FrameAlloc* allocator)
	{
		UINT32 numPasses = (UINT32)mParametersPerPass.size();

		UINT32 paramsSize = 0;
		if (mParams != nullptr)
			mParams->getSyncData(nullptr, paramsSize);

		UINT32 size = sizeof(UINT32) * 3 + numPasses * sizeof(SPtr<PassParametersCore>)
			+ sizeof(SPtr<ShaderCore>) + sizeof(SPtr<TechniqueCore>) + paramsSize;

		UINT8* buffer = allocator->alloc(size);
		char* dataPtr = (char*)buffer;
		dataPtr = rttiWriteElem(numPasses, dataPtr);

		for (UINT32 i = 0; i < numPasses; i++)
		{
			SPtr<PassParametersCore>* passParameters = new (dataPtr) SPtr<PassParametersCore>();
			*passParameters = convertParamsToCore(mParametersPerPass[i]);

			dataPtr += sizeof(SPtr<PassParametersCore>);
		}

		SPtr<ShaderCore>* shader = new (dataPtr)SPtr<ShaderCore>();
		if (mShader.isLoaded(false))
			*shader = mShader->getCore();
		else
			*shader = nullptr;

		dataPtr += sizeof(SPtr<ShaderCore>);

		UINT32 numTechniques = (UINT32)mTechniques.size();
		dataPtr = rttiWriteElem(numTechniques, dataPtr);

		for(UINT32 i = 0; i < numTechniques; i++)
		{
			SPtr<TechniqueCore>* technique = new (dataPtr)SPtr<TechniqueCore>();
				*technique = mTechniques[i]->getCore();

			dataPtr += sizeof(SPtr<TechniqueCore>);
		}

		dataPtr = rttiWriteElem(paramsSize, dataPtr);
		if (mParams != nullptr)
			mParams->getSyncData((UINT8*)dataPtr, paramsSize);

		dataPtr += paramsSize;

		return CoreSyncData(buffer, size);
	}

	void Material::getCoreDependencies(Vector<CoreObject*>& dependencies)
	{
		if (mShader.isLoaded())
			dependencies.push_back(mShader.get());

		for (auto& params : mParametersPerPass)
		{
			for (UINT32 i = 0; i < PassParameters::NUM_PARAMS; i++)
			{
				SPtr<GpuParams> gpuParams = params->getParamByIdx(i);
				if (gpuParams != nullptr)
					dependencies.push_back(gpuParams.get());
			}
		}
	}

	void Material::getListenerResources(Vector<HResource>& resources)
	{
		if (mShader != nullptr)
			resources.push_back(mShader);
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
		initializeIfLoaded();
	}

	void Material::notifyResourceChanged(const HResource& resource)
	{
		mLoadFlags = Load_None;
		initializeIfLoaded();
	}

	HMaterial Material::clone()
	{
		UINT32 bufferSize = 0;

		MemorySerializer serializer;
		UINT8* buffer = serializer.encode(this, bufferSize, (void*(*)(UINT32))&bs_alloc);

		SPtr<Material> cloneObj = std::static_pointer_cast<Material>(serializer.decode(buffer, bufferSize));
		bs_free(buffer);

		return static_resource_cast<Material>(gResources()._createResourceHandle(cloneObj));
	}

	template<class T>
	void copyParam(const SPtr<MaterialParams>& from, Material* to, const String& name, UINT32 index, UINT32 arraySize)
	{
		TMaterialDataParam<T, false> param;
		to->getParam(name, param);

		T paramData;
		for (UINT32 i = 0; i < arraySize; i++)
		{
			from->getDataParam(index, i, paramData);
			param.set(paramData, i);
		}
	}

	void Material::setParams(const SPtr<MaterialParams>& params)
	{
		if (params == nullptr)
			return;

		std::function<void(const SPtr<MaterialParams>&, Material*, const String&, UINT32, UINT32)> copyParamLookup[GPDT_COUNT];

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
				copyFunction(params, this, param.first, paramData->index, elemsToCopy);
			else
			{
				if(param.second.type == GPDT_STRUCT)
				{
					TMaterialParamStruct<false> curParam = getParamStruct(param.first);

					UINT32 structSize = params->getStructSize(paramData->index);
					if (param.second.elementSize != structSize)
						continue;

					UINT8* structData = (UINT8*)bs_stack_alloc(structSize);
					for (UINT32 i = 0; i < elemsToCopy; i++)
					{
						params->getStructData(paramData->index + i, structData, structSize);
						curParam.set(structData, structSize, i);
					}

					bs_stack_free(structData);
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

			bool isLoadStore = params->getIsTextureLoadStore(paramData->index);
			if(!isLoadStore)
			{
				TMaterialParamTexture<false> curParam = getParamTexture(param.first);

				HTexture texture;
				params->getTexture(paramData->index, texture);
				curParam.set(texture);
			}
			else
			{
				TMaterialParamLoadStoreTexture<false> curParam = getParamLoadStoreTexture(param.first);

				HTexture texture;
				TextureSurface surface;
				params->getLoadStoreTexture(paramData->index, texture, surface);
				curParam.set(texture, surface);
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
			params->getBuffer(paramData->index, buffer);
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
			params->getSamplerState(paramData->index, samplerState);
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
}