//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Material/BsGpuParamsSet.h"
#include "Material/BsShader.h"
#include "Material/BsTechnique.h"
#include "Material/BsPass.h"
#include "RenderAPI/BsGpuProgram.h"
#include "RenderAPI/BsGpuPipelineState.h"
#include "Material/BsMaterialParams.h"
#include "RenderAPI/BsGpuParamDesc.h"
#include "RenderAPI/BsRenderAPI.h"
#include "RenderAPI/BsGpuParamBlockBuffer.h"
#include "Animation/BsAnimationCurve.h"
#include "Image/BsColorGradient.h"
#include "Image/BsSpriteTexture.h"

namespace bs
{
	/** Uniquely identifies a GPU parameter. */
	struct ValidParamKey
	{
		ValidParamKey(const String& name, const MaterialParams::ParamType& type)
			:name(name), type(type)
		{ }

		bool operator== (const ValidParamKey& rhs) const
		{
			return name == rhs.name && type == rhs.type;
		}

		bool operator!= (const ValidParamKey& rhs) const
		{
			return !(*this == rhs);
		}

		String name;
		MaterialParams::ParamType type;
	};
}

/** @cond STDLIB */

namespace std
{
	/** Hash value generator for ValidParamKey. */
	template<>
	struct hash<bs::ValidParamKey>
	{
		size_t operator()(const bs::ValidParamKey& key) const
		{
			size_t hash = 0;
			bs::hash_combine(hash, key.name);
			bs::hash_combine(hash, key.type);

			return hash;
		}
	};
}

/** @endcond */

namespace bs
{
	struct ShaderBlockDesc
	{
		String name;
		GpuBufferUsage usage;
		int size;
		bool external;
		UINT32 sequentialIdx;
		UINT32 set;
		UINT32 slot;
	};

	Vector<SPtr<GpuParamDesc>> getAllParamDescs(const SPtr<Technique>& technique)
	{
		Vector<SPtr<GpuParamDesc>> allParamDescs;

		// Make sure all gpu programs are fully loaded
		for (UINT32 i = 0; i < technique->getNumPasses(); i++)
		{
			SPtr<Pass> curPass = technique->getPass(i);

			const SPtr<GraphicsPipelineState>& graphicsPipeline = curPass->getGraphicsPipelineState();
			if(graphicsPipeline)
			{
				SPtr<GpuProgram> vertProgram = graphicsPipeline->getVertexProgram();
				if (vertProgram)
				{
					vertProgram->blockUntilCoreInitialized();
					allParamDescs.push_back(vertProgram->getParamDesc());
				}

				SPtr<GpuProgram> fragProgram = graphicsPipeline->getFragmentProgram();
				if (fragProgram)
				{
					fragProgram->blockUntilCoreInitialized();
					allParamDescs.push_back(fragProgram->getParamDesc());
				}

				SPtr<GpuProgram> geomProgram = graphicsPipeline->getGeometryProgram();
				if (geomProgram)
				{
					geomProgram->blockUntilCoreInitialized();
					allParamDescs.push_back(geomProgram->getParamDesc());
				}

				SPtr<GpuProgram> hullProgram = graphicsPipeline->getHullProgram();
				if (hullProgram)
				{
					hullProgram->blockUntilCoreInitialized();
					allParamDescs.push_back(hullProgram->getParamDesc());
				}

				SPtr<GpuProgram> domainProgram = graphicsPipeline->getDomainProgram();
				if (domainProgram)
				{
					domainProgram->blockUntilCoreInitialized();
					allParamDescs.push_back(domainProgram->getParamDesc());
				}
			}

			const SPtr<ComputePipelineState>& computePipeline = curPass->getComputePipelineState();
			if(computePipeline)
			{
				SPtr<GpuProgram> computeProgram = computePipeline->getProgram();
				if (computeProgram)
				{
					computeProgram->blockUntilCoreInitialized();
					allParamDescs.push_back(computeProgram->getParamDesc());
				}
				
			}
		}

		return allParamDescs;
	}

	Vector<SPtr<GpuParamDesc>> getAllParamDescs(const SPtr<ct::Technique>& technique)
	{
		Vector<SPtr<GpuParamDesc>> allParamDescs;

		// Make sure all gpu programs are fully loaded
		for (UINT32 i = 0; i < technique->getNumPasses(); i++)
		{
			SPtr<ct::Pass> curPass = technique->getPass(i);

			const SPtr<ct::GraphicsPipelineState>& graphicsPipeline = curPass->getGraphicsPipelineState();
			if(graphicsPipeline)
			{
				SPtr<ct::GpuProgram> vertProgram = graphicsPipeline->getVertexProgram();
				if (vertProgram)
					allParamDescs.push_back(vertProgram->getParamDesc());

				SPtr<ct::GpuProgram> fragProgram = graphicsPipeline->getFragmentProgram();
				if (fragProgram)
					allParamDescs.push_back(fragProgram->getParamDesc());

				SPtr<ct::GpuProgram> geomProgram = graphicsPipeline->getGeometryProgram();
				if (geomProgram)
					allParamDescs.push_back(geomProgram->getParamDesc());

				SPtr<ct::GpuProgram> hullProgram = graphicsPipeline->getHullProgram();
				if (hullProgram)
					allParamDescs.push_back(hullProgram->getParamDesc());

				SPtr<ct::GpuProgram> domainProgram = graphicsPipeline->getDomainProgram();
				if (domainProgram)
					allParamDescs.push_back(domainProgram->getParamDesc());
			}

			const SPtr<ct::ComputePipelineState>& computePipeline = curPass->getComputePipelineState();
			if(computePipeline)
			{
				SPtr<ct::GpuProgram> computeProgram = computePipeline->getProgram();
				if (computeProgram)
					allParamDescs.push_back(computeProgram->getParamDesc());
				
			}
		}

		return allParamDescs;
	}

	bool areParamsEqual(const GpuParamDataDesc& paramA, const GpuParamDataDesc& paramB, bool ignoreBufferOffsets)
	{
		bool equal = paramA.arraySize == paramB.arraySize && paramA.elementSize == paramB.elementSize
			&& paramA.type == paramB.type && paramA.arrayElementStride == paramB.arrayElementStride;

		if (!ignoreBufferOffsets)
			equal &= paramA.cpuMemOffset == paramB.cpuMemOffset && paramA.gpuMemOffset == paramB.gpuMemOffset;

		return equal;
	}

	Vector<ShaderBlockDesc> determineValidShareableParamBlocks(const Vector<SPtr<GpuParamDesc>>& paramDescs,
		const Map<String, SHADER_PARAM_BLOCK_DESC>& shaderDesc)
	{
		struct BlockInfo
		{
			BlockInfo() { }
			BlockInfo(const GpuParamBlockDesc* blockDesc, const SPtr<GpuParamDesc>& paramDesc, bool isValid = true)
				:blockDesc(blockDesc), paramDesc(paramDesc), isValid(isValid)
			{ }

			const GpuParamBlockDesc* blockDesc;
			SPtr<GpuParamDesc> paramDesc;
			bool isValid;
			UINT32 set;
			UINT32 slot;
		};

		// Make sure param blocks with the same name actually contain the same fields
		Map<String, BlockInfo> uniqueParamBlocks;

		for (auto iter = paramDescs.begin(); iter != paramDescs.end(); ++iter)
		{
			const GpuParamDesc& curDesc = **iter;
			for (auto blockIter = curDesc.paramBlocks.begin(); blockIter != curDesc.paramBlocks.end(); ++blockIter)
			{
				const GpuParamBlockDesc& curBlock = blockIter->second;

				if (!curBlock.isShareable) // Non-shareable buffers are handled differently, they're allowed same names
					continue;

				auto iterFind = uniqueParamBlocks.find(blockIter->first);
				if (iterFind == uniqueParamBlocks.end())
				{
					uniqueParamBlocks[blockIter->first] = BlockInfo(&curBlock, *iter);
					continue;
				}

				const GpuParamBlockDesc& otherBlock = *iterFind->second.blockDesc;

				// The block was already determined as invalid, no need to check further
				if (!iterFind->second.isValid)
					continue;

				String otherBlockName = otherBlock.name;
				SPtr<GpuParamDesc> otherDesc = iterFind->second.paramDesc;

				for (auto myParamIter = curDesc.params.begin(); myParamIter != curDesc.params.end(); ++myParamIter)
				{
					const GpuParamDataDesc& myParam = myParamIter->second;

					if (myParam.paramBlockSet != curBlock.set || myParam.paramBlockSlot != curBlock.slot)
						continue; // Param is in another block, so we will check it when its time for that block

					auto otherParamFind = otherDesc->params.find(myParamIter->first);

					// Cannot find other param, blocks aren't equal
					if (otherParamFind == otherDesc->params.end())
						break;

					const GpuParamDataDesc& otherParam = otherParamFind->second;

					if (!areParamsEqual(myParam, otherParam, false) || curBlock.name != otherBlockName)
						break;
				}

				// Note: Ignoring mismatched blocks for now, because glslang parser doesn't report dead uniform entries,
				// meaning identical blocks can have different sets of uniforms reported depending on which are unused.
				//if (!isBlockValid)
				//{
				//	LOGWRN("Found two param blocks with the same name but different contents: " + blockIter->first);
				//	uniqueParamBlocks[blockIter->first] = BlockInfo(&curBlock, nullptr, false);

				//	continue;
				//}
			}
		}

		Vector<ShaderBlockDesc> output;
		for (auto& entry : uniqueParamBlocks)
		{
			if (!entry.second.isValid)
				continue;

			const GpuParamBlockDesc& curBlock = *entry.second.blockDesc;

			ShaderBlockDesc shaderBlockDesc;
			shaderBlockDesc.external = false;
			shaderBlockDesc.usage = GBU_STATIC;
			shaderBlockDesc.size = curBlock.blockSize * sizeof(UINT32);
			shaderBlockDesc.name = entry.first;
			shaderBlockDesc.set = curBlock.set;
			shaderBlockDesc.slot = curBlock.slot;

			auto iterFind = shaderDesc.find(entry.first);
			if (iterFind != shaderDesc.end())
			{
				shaderBlockDesc.external = iterFind->second.shared || iterFind->second.rendererSemantic != StringID::NONE;
				shaderBlockDesc.usage = iterFind->second.usage;
			}

			output.push_back(shaderBlockDesc);
		}

		return output;
	}

	Map<String, const GpuParamDataDesc*> determineValidDataParameters(const Vector<SPtr<GpuParamDesc>>& paramDescs)
	{
		Map<String, const GpuParamDataDesc*> foundDataParams;
		Map<String, bool> validParams;

		for (auto iter = paramDescs.begin(); iter != paramDescs.end(); ++iter)
		{
			const GpuParamDesc& curDesc = **iter;

			// Check regular data params
			for (auto iter2 = curDesc.params.begin(); iter2 != curDesc.params.end(); ++iter2)
			{
				const GpuParamDataDesc& curParam = iter2->second;

				auto dataFindIter = validParams.find(iter2->first);
				if (dataFindIter == validParams.end())
				{
					validParams[iter2->first] = true;
					foundDataParams[iter2->first] = &curParam;
				}
				else
				{
					if (validParams[iter2->first])
					{
						auto dataFindIter2 = foundDataParams.find(iter2->first);

						const GpuParamDataDesc* otherParam = dataFindIter2->second;
						if (!areParamsEqual(curParam, *otherParam, true))
						{
							validParams[iter2->first] = false;
							foundDataParams.erase(dataFindIter2);
						}
					}
				}
			}
		}

		return foundDataParams;
	}

	Vector<const GpuParamObjectDesc*> determineValidObjectParameters(const Vector<SPtr<GpuParamDesc>>& paramDescs)
	{
		Vector<const GpuParamObjectDesc*> validParams;

		for (auto iter = paramDescs.begin(); iter != paramDescs.end(); ++iter)
		{
			const GpuParamDesc& curDesc = **iter;

			// Check sampler params
			for (auto iter2 = curDesc.samplers.begin(); iter2 != curDesc.samplers.end(); ++iter2)
			{
				validParams.push_back(&iter2->second);
			}

			// Check texture params
			for (auto iter2 = curDesc.textures.begin(); iter2 != curDesc.textures.end(); ++iter2)
			{
				validParams.push_back(&iter2->second);
			}

			// Check load-store texture params
			for (auto iter2 = curDesc.loadStoreTextures.begin(); iter2 != curDesc.loadStoreTextures.end(); ++iter2)
			{
				validParams.push_back(&iter2->second);
			}

			// Check buffer params
			for (auto iter2 = curDesc.buffers.begin(); iter2 != curDesc.buffers.end(); ++iter2)
			{
				validParams.push_back(&iter2->second);
			}
		}

		return validParams;
	}

	Map<String, String> determineParameterToBlockMapping(const Vector<SPtr<GpuParamDesc>>& paramDescs)
	{
		Map<String, String> paramToParamBlock;

		for (auto iter = paramDescs.begin(); iter != paramDescs.end(); ++iter)
		{
			const GpuParamDesc& curDesc = **iter;
			for (auto iter2 = curDesc.params.begin(); iter2 != curDesc.params.end(); ++iter2)
			{
				const GpuParamDataDesc& curParam = iter2->second;

				auto iterFind = paramToParamBlock.find(curParam.name);
				if (iterFind != paramToParamBlock.end())
					continue;

				for (auto iterBlock = curDesc.paramBlocks.begin(); iterBlock != curDesc.paramBlocks.end(); ++iterBlock)
				{
					if (iterBlock->second.set == curParam.paramBlockSet && iterBlock->second.slot == curParam.paramBlockSlot)
					{
						paramToParamBlock[curParam.name] = iterBlock->second.name;
						break;
					}
				}
			}
		}

		return paramToParamBlock;
	}

	UnorderedMap<ValidParamKey, String> determineValidParameters(const Vector<SPtr<GpuParamDesc>>& paramDescs,
		const Map<String, SHADER_DATA_PARAM_DESC>& dataParams,
		const Map<String, SHADER_OBJECT_PARAM_DESC>& textureParams,
		const Map<String, SHADER_OBJECT_PARAM_DESC>& bufferParams,
		const Map<String, SHADER_OBJECT_PARAM_DESC>& samplerParams)
	{
		UnorderedMap<ValidParamKey, String> validParams;

		Map<String, const GpuParamDataDesc*> validDataParameters = determineValidDataParameters(paramDescs);
		Vector<const GpuParamObjectDesc*> validObjectParameters = determineValidObjectParameters(paramDescs);
		Map<String, String> paramToParamBlockMap = determineParameterToBlockMapping(paramDescs);

		// Create data param mappings
		for (auto iter = dataParams.begin(); iter != dataParams.end(); ++iter)
		{
			auto findIter = validDataParameters.find(iter->second.gpuVariableName);

			// Not valid so we skip it
			if (findIter == validDataParameters.end())
				continue;

			if (findIter->second->type != iter->second.type && !(iter->second.type == GPDT_COLOR && findIter->second->type == GPDT_FLOAT4))
			{
				LOGWRN("Ignoring shader parameter \"" + iter->first + "\". Type doesn't match the one defined in the gpu program. "
					+ "Shader defined type: " + toString(iter->second.type) + " - Gpu program defined type: " + toString(findIter->second->type));
				continue;
			}

			auto findBlockIter = paramToParamBlockMap.find(iter->second.gpuVariableName);

			if (findBlockIter == paramToParamBlockMap.end())
				BS_EXCEPT(InternalErrorException, "Parameter doesn't exist in param to param block map but exists in valid param map.");

			ValidParamKey key(iter->second.gpuVariableName, MaterialParams::ParamType::Data);
			validParams.insert(std::make_pair(key, iter->first));
		}

		// Create object param mappings
		auto determineObjectMappings = [&](const Map<String, SHADER_OBJECT_PARAM_DESC>& params, MaterialParams::ParamType paramType)
		{
			for (auto iter = params.begin(); iter != params.end(); ++iter)
			{
				const Vector<String>& gpuVariableNames = iter->second.gpuVariableNames;
				for (auto iter2 = gpuVariableNames.begin(); iter2 != gpuVariableNames.end(); ++iter2)
				{
					for (auto iter3 = validObjectParameters.begin(); iter3 != validObjectParameters.end(); ++iter3)
					{
						if ((*iter3)->name == (*iter2))
						{
							ValidParamKey key(*iter2, paramType);
							validParams.insert(std::make_pair(key, iter->first));

							break;
						}
					}
				}
			}
		};

		determineObjectMappings(textureParams, MaterialParams::ParamType::Texture);
		determineObjectMappings(samplerParams, MaterialParams::ParamType::Sampler);
		determineObjectMappings(bufferParams, MaterialParams::ParamType::Buffer);

		return validParams;
	}

	template<bool Core>
	const UINT32 TGpuParamsSet<Core>::NUM_STAGES = 6;

	template<bool Core>
	TGpuParamsSet<Core>::TGpuParamsSet(const SPtr<TechniqueType>& technique, const ShaderType& shader,
		const SPtr<MaterialParamsType>& params)
		:mPassParams(technique->getNumPasses()), mParamVersion(0)
	{
		UINT32 numPasses = technique->getNumPasses();

		// Create GpuParams for each pass and shader stage
		for (UINT32 i = 0; i < numPasses; i++)
		{
			SPtr<PassType> curPass = technique->getPass(i);

			SPtr<GraphicsPipelineStateType> gfxPipeline = curPass->getGraphicsPipelineState();
			if(gfxPipeline != nullptr)
				mPassParams[i] = GpuParamsType::create(gfxPipeline);
			else
			{
				SPtr<ComputePipelineStateType> computePipeline = curPass->getComputePipelineState();
				mPassParams[i] = GpuParamsType::create(computePipeline);
			}
		}

		// Create and assign parameter block buffers
		Vector<SPtr<GpuParamDesc>> allParamDescs = getAllParamDescs(technique);

		//// Fill out various helper structures
		Vector<ShaderBlockDesc> paramBlockData = determineValidShareableParamBlocks(allParamDescs, shader->getParamBlocks());
		UnorderedMap<ValidParamKey, String> validParams = determineValidParameters(
			allParamDescs, 
			shader->getDataParams(), 
			shader->getTextureParams(), 
			shader->getBufferParams(), 
			shader->getSamplerParams());

		Map<String, ParamBlockPtrType> paramBlockBuffers;

		//// Create param blocks
		for (auto& paramBlock : paramBlockData)
		{
			ParamBlockPtrType newParamBlockBuffer;
			if (!paramBlock.external)
				newParamBlockBuffer = ParamBlockType::create(paramBlock.size, paramBlock.usage);

			paramBlock.sequentialIdx = (UINT32)mBlocks.size();

			paramBlockBuffers[paramBlock.name] = newParamBlockBuffer;
			mBlocks.push_back(BlockInfo(paramBlock.name, paramBlock.set, paramBlock.slot, newParamBlockBuffer, true));
		}

		//// Assign param block buffers and generate information about data parameters
		assert(numPasses < 64); // BlockInfo flags uses UINT64 for tracking usage
		for (UINT32 i = 0; i < numPasses; i++)
		{
			SPtr<GpuParamsType> paramPtr = mPassParams[i];
			for (UINT32 j = 0; j < NUM_STAGES; j++)
			{
				GpuProgramType progType = (GpuProgramType)j;

				// Assign shareable buffers
				for (auto& block : paramBlockData)
				{
					const String& paramBlockName = block.name;
					if (paramPtr->hasParamBlock(progType, paramBlockName))
					{
						ParamBlockPtrType blockBuffer = paramBlockBuffers[paramBlockName];

						paramPtr->setParamBlockBuffer(progType, paramBlockName, blockBuffer);
					}
				}

				// Create non-shareable ones (these are buffers defined by default by the RHI usually)
				SPtr<GpuParamDesc> desc = paramPtr->getParamDesc(progType);
				if (desc == nullptr)
					continue;

				for (auto iterBlockDesc = desc->paramBlocks.begin(); iterBlockDesc != desc->paramBlocks.end(); ++iterBlockDesc)
				{
					const GpuParamBlockDesc& blockDesc = iterBlockDesc->second;

					UINT32 globalBlockIdx = (UINT32)-1;
					if (!blockDesc.isShareable)
					{
						ParamBlockPtrType newParamBlockBuffer = ParamBlockType::create(blockDesc.blockSize * sizeof(UINT32));

						globalBlockIdx = (UINT32)mBlocks.size();

						paramPtr->setParamBlockBuffer(progType, iterBlockDesc->first, newParamBlockBuffer);
						mBlocks.emplace_back(iterBlockDesc->first, iterBlockDesc->second.set, 
							iterBlockDesc->second.slot, newParamBlockBuffer, false);
					}
					else
					{
						auto iterFind = std::find_if(paramBlockData.begin(), paramBlockData.end(), [&](const auto& x)
						{
							return x.name == iterBlockDesc->first;
						});

						if(iterFind != paramBlockData.end())
							globalBlockIdx = iterFind->sequentialIdx;
					}

					// If this parameter block is valid, create data/struct mappings for it
					if (globalBlockIdx == (UINT32)-1)
						continue;

					for(auto& dataParam : desc->params)
					{
						if (dataParam.second.paramBlockSet != blockDesc.set || dataParam.second.paramBlockSlot != blockDesc.slot)
							continue;

						ValidParamKey key(dataParam.first, MaterialParams::ParamType::Data);

						auto iterFind = validParams.find(key);
						if (iterFind == validParams.end())
							continue;

						UINT32 paramIdx = params->getParamIndex(iterFind->second);

						// Parameter shouldn't be in the valid parameter list if it cannot be found
						assert(paramIdx != (UINT32)-1);

						mDataParamInfos.push_back(DataParamInfo());
						DataParamInfo& paramInfo = mDataParamInfos.back();
						paramInfo.paramIdx = paramIdx;
						paramInfo.blockIdx = globalBlockIdx;
						paramInfo.offset = dataParam.second.cpuMemOffset;
					}
				}
			}
		}

		// Add buffers defined in shader but not actually used by GPU programs (so we can check if user is providing a
		// valid buffer name)
		auto& allParamBlocks = shader->getParamBlocks();
		for (auto& entry : allParamBlocks)
		{
			auto iterFind = std::find_if(mBlocks.begin(), mBlocks.end(), 
				[&](auto& x)
			{
				return x.name == entry.first;
			});

			if(iterFind == mBlocks.end())
			{
				mBlocks.push_back(BlockInfo(entry.first, 0, 0, nullptr, true));
				mBlocks.back().isUsed = false;
			}
		}

		// Generate information about object parameters
		bs_frame_mark();
		{
			FrameVector<ObjectParamInfo> objParamInfos;

			UINT32 offsetsSize = numPasses * NUM_STAGES * 4 * sizeof(UINT32);
			UINT32* offsets = (UINT32*)bs_frame_alloc(offsetsSize);
			memset(offsets, 0, offsetsSize);

			// First store all objects in temporary arrays since we don't know how many of them are
			UINT32 totalNumObjects = 0;
			UINT32* stageOffsets = offsets;
			for (UINT32 i = 0; i < numPasses; i++)
			{
				SPtr<GpuParamsType> paramPtr = mPassParams[i];
				for (UINT32 j = 0; j < NUM_STAGES; j++)
				{
					GpuProgramType progType = (GpuProgramType)j;

					auto processObjectParams = [&](const Map<String, GpuParamObjectDesc>& gpuParams, 
						UINT32 stageIdx, MaterialParams::ParamType paramType)
					{
						for (auto& param : gpuParams)
						{
							ValidParamKey key(param.first, paramType);

							auto iterFind = validParams.find(key);
							if (iterFind == validParams.end())
								continue;

							UINT32 paramIdx;
							auto result = params->getParamIndex(iterFind->second, paramType, GPDT_UNKNOWN, 0, paramIdx);

							// Parameter shouldn't be in the valid parameter list if it cannot be found
							assert(result == MaterialParams::GetParamResult::Success);

							objParamInfos.push_back(ObjectParamInfo());
							ObjectParamInfo& paramInfo = objParamInfos.back();
							paramInfo.paramIdx = paramIdx;
							paramInfo.slotIdx = param.second.slot;
							paramInfo.setIdx = param.second.set;

							stageOffsets[stageIdx]++;
							totalNumObjects++;
						}
					};

					SPtr<GpuParamDesc> desc = paramPtr->getParamDesc(progType);
					if(desc == nullptr)
					{
						stageOffsets += 4;
						continue;
					}

					processObjectParams(desc->textures, 0, MaterialParams::ParamType::Texture);
					processObjectParams(desc->loadStoreTextures, 1, MaterialParams::ParamType::Texture);
					processObjectParams(desc->buffers, 2, MaterialParams::ParamType::Buffer);
					processObjectParams(desc->samplers, 3, MaterialParams::ParamType::Sampler);

					stageOffsets += 4;
				}
			}

			// Transfer all objects into their permanent storage
			UINT32 numBlocks = (UINT32)mBlocks.size();
			UINT32 blockBindingsSize = numBlocks * numPasses * sizeof(PassBlockBindings);
			UINT32 objectParamInfosSize = totalNumObjects * sizeof(ObjectParamInfo) + numPasses * sizeof(PassParamInfo);
			mData = (UINT8*)bs_alloc(objectParamInfosSize + blockBindingsSize);
			UINT8* dataIter = mData;

			mPassParamInfos = (PassParamInfo*)dataIter;
			memset(mPassParamInfos, 0, objectParamInfosSize);
			dataIter += objectParamInfosSize;

			StageParamInfo* stageInfos = (StageParamInfo*)mPassParamInfos;

			ObjectParamInfo* objInfos = (ObjectParamInfo*)(mPassParamInfos + numPasses);
			memcpy(objInfos, objParamInfos.data(), totalNumObjects * sizeof(ObjectParamInfo));

			UINT32 objInfoOffset = 0;

			stageOffsets = offsets;
			for (UINT32 i = 0; i < numPasses; i++)
			{
				for (UINT32 j = 0; j < NUM_STAGES; j++)
				{
					StageParamInfo& stage = stageInfos[i * NUM_STAGES + j];

					if(stageOffsets[0] > 0)
					{
						UINT32 numEntries = stageOffsets[0];

						stage.textures = objInfos + objInfoOffset;
						stage.numTextures = numEntries;

						objInfoOffset += numEntries;
					}

					if (stageOffsets[1] > 0)
					{
						UINT32 numEntries = stageOffsets[1];

						stage.loadStoreTextures = objInfos + objInfoOffset;
						stage.numLoadStoreTextures = numEntries;

						objInfoOffset += numEntries;
					}

					if (stageOffsets[2] > 0)
					{
						UINT32 numEntries = stageOffsets[2];

						stage.buffers = objInfos + objInfoOffset;
						stage.numBuffers = numEntries;

						objInfoOffset += numEntries;
					}

					if (stageOffsets[3] > 0)
					{
						UINT32 numEntries = stageOffsets[3];

						stage.samplerStates = objInfos + objInfoOffset;
						stage.numSamplerStates = numEntries;

						objInfoOffset += numEntries;
					}

					stageOffsets += 4;
				}
			}

			// Determine on which passes & stages are buffers used on
			for (auto& block : mBlocks)
			{
				block.passData = (PassBlockBindings*)dataIter;
				dataIter += sizeof(PassBlockBindings) * numPasses;
			}

			for (auto& block : mBlocks)
			{
				for (UINT32 i = 0; i < numPasses; i++)
				{
					SPtr<GpuParamsType> paramPtr = mPassParams[i];
					for (UINT32 j = 0; j < NUM_STAGES; j++)
					{
						GpuProgramType progType = (GpuProgramType)j;

						SPtr<GpuParamDesc> curDesc = paramPtr->getParamDesc(progType);
						if (curDesc == nullptr)
						{
							block.passData[i].bindings[j].set = -1;
							block.passData[i].bindings[j].slot = -1;

							continue;
						}

						auto iterFind = curDesc->paramBlocks.find(block.name);
						if (iterFind == curDesc->paramBlocks.end())
						{
							block.passData[i].bindings[j].set = -1;
							block.passData[i].bindings[j].slot = -1;

							continue;
						}

						block.passData[i].bindings[j].set = iterFind->second.set;
						block.passData[i].bindings[j].slot = iterFind->second.slot;
					}
				}
			}

			bs_frame_free(offsets);
		}
		bs_frame_clear();
	}

	template<bool Core>
	TGpuParamsSet<Core>::~TGpuParamsSet()
	{
		// All allocations share the same memory, so we just clear it all at once
		bs_free(mData);
	}

	template<bool Core>
	SPtr<typename TGpuParamsSet<Core>::GpuParamsType> TGpuParamsSet<Core>::getGpuParams(UINT32 passIdx)
	{
		if (passIdx >= mPassParams.size())
			return nullptr;

		return mPassParams[passIdx];
	}

	template<bool Core>
	UINT32 TGpuParamsSet<Core>::getParamBlockBufferIndex(const String& name) const
	{
		for (UINT32 i = 0; i < (UINT32)mBlocks.size(); i++)
		{
			const BlockInfo& block = mBlocks[i];
			if (block.name == name)
				return i;
		}

		return -1;
	}

	template<bool Core>
	void TGpuParamsSet<Core>::setParamBlockBuffer(UINT32 index, const ParamBlockPtrType& paramBlock,
												  bool ignoreInUpdate)
	{
		BlockInfo& blockInfo = mBlocks[index];
		if (!blockInfo.shareable)
		{
			LOGERR("Cannot set parameter block buffer with the name \"" + blockInfo.name + 
				"\". Buffer is not assignable. ");
			return;
		}

		if (!blockInfo.isUsed)
			return;

		blockInfo.allowUpdate = !ignoreInUpdate;

		if (blockInfo.buffer != paramBlock)
		{
			blockInfo.buffer = paramBlock;

			UINT32 numPasses = (UINT32)mPassParams.size();
			for (UINT32 j = 0; j < numPasses; j++)
			{
				SPtr<GpuParamsType> paramPtr = mPassParams[j];
				for (UINT32 i = 0; i < NUM_STAGES; i++)
				{
					GpuProgramType progType = (GpuProgramType)i;

					const BlockBinding& binding = blockInfo.passData[j].bindings[progType];

					if (binding.slot != (UINT32)-1)
						paramPtr->setParamBlockBuffer(binding.set, binding.slot, paramBlock);
				}
			}
		}
	}

	template<bool Core>
	void TGpuParamsSet<Core>::setParamBlockBuffer(const String& name, const ParamBlockPtrType& paramBlock, 
		bool ignoreInUpdate)
	{
		UINT32 bufferIdx = getParamBlockBufferIndex(name);
		if(bufferIdx == (UINT32)-1)
		{
			LOGERR("Cannot set parameter block buffer with the name \"" + name + "\". Buffer name not found. ");
			return;
		}

		setParamBlockBuffer(bufferIdx, paramBlock, ignoreInUpdate);
	}

	template<bool Core>
	void TGpuParamsSet<Core>::update(const SPtr<MaterialParamsType>& params, float t, bool updateAll)
	{
		// Note: Instead of iterating over every single parameter, it might be more efficient for @p params to keep
		// a ring buffer and a version number. Then we could just iterate over the ring buffer and only access dirty
		// parameters. If the version number is too high (larger than ring buffer can store), then we force update for all.

		// Update data params
		for(auto& paramInfo : mDataParamInfos)
		{
			ParamBlockPtrType paramBlock = mBlocks[paramInfo.blockIdx].buffer;
			if (paramBlock == nullptr || !mBlocks[paramInfo.blockIdx].allowUpdate)
				continue;

			const MaterialParams::ParamData* materialParamInfo = params->getParamData(paramInfo.paramIdx);
			UINT32 arraySize = materialParamInfo->arraySize == 0 ? 1 : materialParamInfo->arraySize;
			
			bool isAnimated = false;
			for(UINT32 i = 0; i < arraySize; i++)
			{
				isAnimated = params->isAnimated(*materialParamInfo, i);
				if(isAnimated)
					break;
			}

			if (materialParamInfo->version <= mParamVersion && !updateAll && !isAnimated)
				continue;

			const GpuParamDataTypeInfo& typeInfo = GpuParams::PARAM_SIZES.lookup[(int)materialParamInfo->dataType];
			UINT32 paramSize = typeInfo.numColumns * typeInfo.numRows * typeInfo.baseTypeSize;

			UINT8* data = params->getData(materialParamInfo->index);
			if(!isAnimated)
			{
				const bool transposeMatrices = ct::RenderAPI::instance().getAPIInfo().isFlagSet(RenderAPIFeatureFlag::ColumnMajorMatrices);
				if (transposeMatrices)
				{
					auto writeTransposed = [&](auto& temp)
					{
						for (UINT32 i = 0; i < arraySize; i++)
						{
							UINT32 arrayOffset = i * paramSize;
							memcpy(&temp, data + arrayOffset, paramSize);
							auto transposed = temp.transpose();

							paramBlock->write(paramInfo.offset * sizeof(UINT32) + arrayOffset, &transposed, paramSize);
						}
					};

					switch (materialParamInfo->dataType)
					{
					case GPDT_MATRIX_2X2:
					{
						MatrixNxM<2, 2> matrix;
						writeTransposed(matrix);
					}
					break;
					case GPDT_MATRIX_2X3:
					{
						MatrixNxM<2, 3> matrix;
						writeTransposed(matrix);
					}
					break;
					case GPDT_MATRIX_2X4:
					{
						MatrixNxM<2, 4> matrix;
						writeTransposed(matrix);
					}
					break;
					case GPDT_MATRIX_3X2:
					{
						MatrixNxM<3, 2> matrix;
						writeTransposed(matrix);
					}
					break;
					case GPDT_MATRIX_3X3:
					{
						Matrix3 matrix;
						writeTransposed(matrix);
					}
					break;
					case GPDT_MATRIX_3X4:
					{
						MatrixNxM<3, 4> matrix;
						writeTransposed(matrix);
					}
					break;
					case GPDT_MATRIX_4X2:
					{
						MatrixNxM<4, 2> matrix;
						writeTransposed(matrix);
					}
					break;
					case GPDT_MATRIX_4X3:
					{
						MatrixNxM<4, 3> matrix;
						writeTransposed(matrix);
					}
					break;
					case GPDT_MATRIX_4X4:
					{
						Matrix4 matrix;
						writeTransposed(matrix);
					}
					break;
					default:
					{
						paramBlock->write(paramInfo.offset * sizeof(UINT32), data, paramSize * arraySize);
						break;
					}
					}
				}
				else
					paramBlock->write(paramInfo.offset * sizeof(UINT32), data, paramSize * arraySize);
			}
			else // Animated
			{
				if(materialParamInfo->dataType == GPDT_FLOAT1)
				{
					assert(paramSize == sizeof(float));

					for (UINT32 i = 0; i < arraySize; i++)
					{
						UINT32 arrayOffset = i * paramSize;
						UINT32 writeOffset = paramInfo.offset * sizeof(UINT32) + arrayOffset;

						float value;
						if(params->isAnimated(*materialParamInfo, i))
						{
							const TAnimationCurve<float>& curve = params->template getCurveParam<float>(*materialParamInfo, i);

							value = curve.evaluate(t, true);
						}
						else
							memcpy(&value, data + arrayOffset, paramSize);

						paramBlock->write(writeOffset, &value, paramSize);
					}
				}
				else if(materialParamInfo->dataType == GPDT_FLOAT4)
				{
					assert(paramSize == sizeof(Rect2));
					
					typename TSpriteTextureType<Core>::Type spriteTexture = 
						params->getOwningSpriteTexture(*materialParamInfo);

					UINT32 writeOffset = paramInfo.offset * sizeof(UINT32);
					Rect2 uv = Rect2(0.0f, 0.0f, 1.0f, 1.0f);
					if(spriteTexture != nullptr)
						uv = spriteTexture->evaluate(t);

					paramBlock->write(writeOffset, &uv, paramSize);

					// Only the first array element receives sprite UVs, the rest are treated as normal
					if(arraySize > 1)
					{
						writeOffset = paramInfo.offset * sizeof(UINT32) + paramSize;
						paramBlock->write(writeOffset, data + paramSize, paramSize * (arraySize - 1));
					}
				}
				else if(materialParamInfo->dataType == GPDT_COLOR)
				{
					for (UINT32 i = 0; i < arraySize; i++)
					{
						assert(paramSize == sizeof(Color));

						UINT32 arrayOffset = i * paramSize;
						UINT32 writeOffset = paramInfo.offset * sizeof(UINT32) + arrayOffset;

						Color value;
						if(params->isAnimated(*materialParamInfo, i))
						{
							const ColorGradient& gradient = params->getColorGradientParam(*materialParamInfo, i);

							const float wrappedT = Math::repeat(t, gradient.getDuration());
							value.setAsRGBA(gradient.evaluate(wrappedT));
						}
						else
							memcpy(&value, data + arrayOffset, paramSize);

						paramBlock->write(writeOffset, &value, paramSize);
					}
				}
			}
		}

		// Update object params
		const auto numPasses = (UINT32)mPassParams.size();

		for(UINT32 i = 0; i < numPasses; i++)
		{
			SPtr<GpuParamsType> paramPtr = mPassParams[i];

			for(UINT32 j = 0; j < NUM_STAGES; j++)
			{
				const StageParamInfo& stageInfo = mPassParamInfos[i].stages[j];

				for(UINT32 k = 0; k < stageInfo.numTextures; k++)
				{
					const ObjectParamInfo& paramInfo = stageInfo.textures[k];

					const MaterialParams::ParamData* materialParamInfo = params->getParamData(paramInfo.paramIdx);
					if (materialParamInfo->version <= mParamVersion && !updateAll)
						continue;

					TextureSurface surface;
					TextureType texture;
					params->getTexture(*materialParamInfo, texture, surface);

					paramPtr->setTexture(paramInfo.setIdx, paramInfo.slotIdx, texture, surface);
				}

				for (UINT32 k = 0; k < stageInfo.numLoadStoreTextures; k++)
				{
					const ObjectParamInfo& paramInfo = stageInfo.loadStoreTextures[k];

					const MaterialParams::ParamData* materialParamInfo = params->getParamData(paramInfo.paramIdx);
					if (materialParamInfo->version <= mParamVersion && !updateAll)
						continue;

					TextureSurface surface;
					TextureType texture;
					params->getLoadStoreTexture(*materialParamInfo, texture, surface);

					paramPtr->setLoadStoreTexture(paramInfo.setIdx, paramInfo.slotIdx, texture, surface);
				}

				for (UINT32 k = 0; k < stageInfo.numBuffers; k++)
				{
					const ObjectParamInfo& paramInfo = stageInfo.buffers[k];

					const MaterialParams::ParamData* materialParamInfo = params->getParamData(paramInfo.paramIdx);
					if (materialParamInfo->version <= mParamVersion && !updateAll)
						continue;

					BufferType buffer;
					params->getBuffer(*materialParamInfo, buffer);

					paramPtr->setBuffer(paramInfo.setIdx, paramInfo.slotIdx, buffer);
				}

				for (UINT32 k = 0; k < stageInfo.numSamplerStates; k++)
				{
					const ObjectParamInfo& paramInfo = stageInfo.samplerStates[k];

					const MaterialParams::ParamData* materialParamInfo = params->getParamData(paramInfo.paramIdx);
					if (materialParamInfo->version <= mParamVersion && !updateAll)
						continue;

					SamplerStateType samplerState;
					params->getSamplerState(*materialParamInfo, samplerState);

					paramPtr->setSamplerState(paramInfo.setIdx, paramInfo.slotIdx, samplerState);
				}
			}

			paramPtr->_markCoreDirty();
		}

		mParamVersion = params->getParamVersion();
	}

	template class TGpuParamsSet <false>;
	template class TGpuParamsSet <true>;
}