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

using namespace bs;

/** Uniquely identifies a GPU parameter. */
struct ValidParamKey
{
	ValidParamKey(const String& name, const MaterialParams::ParamType& type)
		: Name(name), Type(type)
	{}

	bool operator==(const ValidParamKey& rhs) const
	{
		return Name == rhs.Name && Type == rhs.Type;
	}

	bool operator!=(const ValidParamKey& rhs) const
	{
		return !(*this == rhs);
	}

	String Name;
	MaterialParams::ParamType Type;
};

/** @cond STDLIB */

namespace std
{
/** Hash value generator for ValidParamKey. */
template <>
struct hash<ValidParamKey>
{
	size_t operator()(const ValidParamKey& key) const
	{
		size_t hash = 0;
		bs::bs_hash_combine(hash, key.Name);
		bs::bs_hash_combine(hash, key.Type);

		return hash;
	}
};
} // namespace std

/** @endcond */

using namespace bs;

struct ShaderBlockDesc
{
	String Name;
	GpuBufferUsage Usage;
	int Size;
	bool External;
	u32 SequentialIdx;
	u32 Set;
	u32 Slot;
};

Vector<SPtr<GpuParamDesc>> getAllParamDescs(const SPtr<Technique>& technique)
{
	Vector<SPtr<GpuParamDesc>> allParamDescs;

	// Make sure all gpu programs are fully loaded
	for(u32 i = 0; i < technique->GetNumPasses(); i++)
	{
		SPtr<Pass> curPass = technique->GetPass(i);

		const SPtr<GraphicsPipelineState>& graphicsPipeline = curPass->GetGraphicsPipelineState();
		if(graphicsPipeline)
		{
			SPtr<GpuProgram> vertProgram = graphicsPipeline->GetVertexProgram();
			if(vertProgram)
			{
				vertProgram->BlockUntilCoreInitialized();
				allParamDescs.push_back(vertProgram->GetParamDesc());
			}

			SPtr<GpuProgram> fragProgram = graphicsPipeline->GetFragmentProgram();
			if(fragProgram)
			{
				fragProgram->BlockUntilCoreInitialized();
				allParamDescs.push_back(fragProgram->GetParamDesc());
			}

			SPtr<GpuProgram> geomProgram = graphicsPipeline->GetGeometryProgram();
			if(geomProgram)
			{
				geomProgram->BlockUntilCoreInitialized();
				allParamDescs.push_back(geomProgram->GetParamDesc());
			}

			SPtr<GpuProgram> hullProgram = graphicsPipeline->GetHullProgram();
			if(hullProgram)
			{
				hullProgram->BlockUntilCoreInitialized();
				allParamDescs.push_back(hullProgram->GetParamDesc());
			}

			SPtr<GpuProgram> domainProgram = graphicsPipeline->GetDomainProgram();
			if(domainProgram)
			{
				domainProgram->BlockUntilCoreInitialized();
				allParamDescs.push_back(domainProgram->GetParamDesc());
			}
		}

		const SPtr<ComputePipelineState>& computePipeline = curPass->GetComputePipelineState();
		if(computePipeline)
		{
			SPtr<GpuProgram> computeProgram = computePipeline->GetProgram();
			if(computeProgram)
			{
				computeProgram->BlockUntilCoreInitialized();
				allParamDescs.push_back(computeProgram->GetParamDesc());
			}
		}
	}

	return allParamDescs;
}

Vector<SPtr<GpuParamDesc>> getAllParamDescs(const SPtr<ct::Technique>& technique)
{
	Vector<SPtr<GpuParamDesc>> allParamDescs;

	// Make sure all gpu programs are fully loaded
	for(u32 i = 0; i < technique->GetNumPasses(); i++)
	{
		SPtr<ct::Pass> curPass = technique->GetPass(i);

		const SPtr<ct::GraphicsPipelineState>& graphicsPipeline = curPass->GetGraphicsPipelineState();
		if(graphicsPipeline)
		{
			SPtr<ct::GpuProgram> vertProgram = graphicsPipeline->GetVertexProgram();
			if(vertProgram)
				allParamDescs.push_back(vertProgram->GetParamDesc());

			SPtr<ct::GpuProgram> fragProgram = graphicsPipeline->GetFragmentProgram();
			if(fragProgram)
				allParamDescs.push_back(fragProgram->GetParamDesc());

			SPtr<ct::GpuProgram> geomProgram = graphicsPipeline->GetGeometryProgram();
			if(geomProgram)
				allParamDescs.push_back(geomProgram->GetParamDesc());

			SPtr<ct::GpuProgram> hullProgram = graphicsPipeline->GetHullProgram();
			if(hullProgram)
				allParamDescs.push_back(hullProgram->GetParamDesc());

			SPtr<ct::GpuProgram> domainProgram = graphicsPipeline->GetDomainProgram();
			if(domainProgram)
				allParamDescs.push_back(domainProgram->GetParamDesc());
		}

		const SPtr<ct::ComputePipelineState>& computePipeline = curPass->GetComputePipelineState();
		if(computePipeline)
		{
			SPtr<ct::GpuProgram> computeProgram = computePipeline->GetProgram();
			if(computeProgram)
				allParamDescs.push_back(computeProgram->GetParamDesc());
		}
	}

	return allParamDescs;
}

bool areParamsEqual(const GpuParamDataDesc& paramA, const GpuParamDataDesc& paramB, bool ignoreBufferOffsets)
{
	bool equal = paramA.ArraySize == paramB.ArraySize && paramA.ElementSize == paramB.ElementSize && paramA.Type == paramB.Type && paramA.ArrayElementStride == paramB.ArrayElementStride;

	if(!ignoreBufferOffsets)
		equal &= paramA.CpuMemOffset == paramB.CpuMemOffset && paramA.GpuMemOffset == paramB.GpuMemOffset;

	return equal;
}

Vector<ShaderBlockDesc> determineValidShareableParamBlocks(const Vector<SPtr<GpuParamDesc>>& paramDescs, const Map<String, SHADER_PARAM_BLOCK_DESC>& shaderDesc)
{
	struct BlockInfo
	{
		BlockInfo() {}

		BlockInfo(const GpuParamBlockDesc* blockDesc, const SPtr<GpuParamDesc>& paramDesc, bool isValid = true)
			: BlockDesc(blockDesc), ParamDesc(paramDesc), IsValid(isValid)
		{}

		const GpuParamBlockDesc* BlockDesc;
		SPtr<GpuParamDesc> ParamDesc;
		bool IsValid;
		u32 Set;
		u32 Slot;
	};

	// Make sure param blocks with the same name actually contain the same fields
	Map<String, BlockInfo> uniqueParamBlocks;

	for(auto iter = paramDescs.begin(); iter != paramDescs.end(); ++iter)
	{
		const GpuParamDesc& curDesc = **iter;
		for(auto blockIter = curDesc.ParamBlocks.begin(); blockIter != curDesc.ParamBlocks.end(); ++blockIter)
		{
			const GpuParamBlockDesc& curBlock = blockIter->second;

			if(!curBlock.IsShareable) // Non-shareable buffers are handled differently, they're allowed same names
				continue;

			auto iterFind = uniqueParamBlocks.find(blockIter->first);
			if(iterFind == uniqueParamBlocks.end())
			{
				uniqueParamBlocks[blockIter->first] = BlockInfo(&curBlock, *iter);
				continue;
			}

			const GpuParamBlockDesc& otherBlock = *iterFind->second.BlockDesc;

			// The block was already determined as invalid, no need to check further
			if(!iterFind->second.IsValid)
				continue;

			String otherBlockName = otherBlock.Name;
			SPtr<GpuParamDesc> otherDesc = iterFind->second.ParamDesc;

			for(auto myParamIter = curDesc.Params.begin(); myParamIter != curDesc.Params.end(); ++myParamIter)
			{
				const GpuParamDataDesc& myParam = myParamIter->second;

				if(myParam.ParamBlockSet != curBlock.Set || myParam.ParamBlockSlot != curBlock.Slot)
					continue; // Param is in another block, so we will check it when its time for that block

				auto otherParamFind = otherDesc->Params.find(myParamIter->first);

				// Cannot find other param, blocks aren't equal
				if(otherParamFind == otherDesc->Params.end())
					break;

				const GpuParamDataDesc& otherParam = otherParamFind->second;

				if(!areParamsEqual(myParam, otherParam, false) || curBlock.Name != otherBlockName)
					break;
			}

			// Note: Ignoring mismatched blocks for now, because glslang parser doesn't report dead uniform entries,
			// meaning identical blocks can have different sets of uniforms reported depending on which are unused.
			// if (!isBlockValid)
			//{
			//	LOGWRN("Found two param blocks with the same name but different contents: " + blockIter->first);
			//	uniqueParamBlocks[blockIter->first] = BlockInfo(&curBlock, nullptr, false);

			//	continue;
			//}
		}
	}

	Vector<ShaderBlockDesc> output;
	for(auto& entry : uniqueParamBlocks)
	{
		if(!entry.second.IsValid)
			continue;

		const GpuParamBlockDesc& curBlock = *entry.second.BlockDesc;

		ShaderBlockDesc shaderBlockDesc;
		shaderBlockDesc.External = false;
		shaderBlockDesc.Usage = GBU_STATIC;
		shaderBlockDesc.Size = curBlock.BlockSize * sizeof(u32);
		shaderBlockDesc.Name = entry.first;
		shaderBlockDesc.Set = curBlock.Set;
		shaderBlockDesc.Slot = curBlock.Slot;

		auto iterFind = shaderDesc.find(entry.first);
		if(iterFind != shaderDesc.end())
		{
			shaderBlockDesc.External = iterFind->second.Shared || iterFind->second.RendererSemantic != StringID::kNone;
			shaderBlockDesc.Usage = iterFind->second.Usage;
		}

		output.push_back(shaderBlockDesc);
	}

	return output;
}

Map<String, const GpuParamDataDesc*> determineValidDataParameters(const Vector<SPtr<GpuParamDesc>>& paramDescs)
{
	Map<String, const GpuParamDataDesc*> foundDataParams;
	Map<String, bool> validParams;

	for(auto iter = paramDescs.begin(); iter != paramDescs.end(); ++iter)
	{
		const GpuParamDesc& curDesc = **iter;

		// Check regular data params
		for(auto iter2 = curDesc.Params.begin(); iter2 != curDesc.Params.end(); ++iter2)
		{
			const GpuParamDataDesc& curParam = iter2->second;

			auto dataFindIter = validParams.find(iter2->first);
			if(dataFindIter == validParams.end())
			{
				validParams[iter2->first] = true;
				foundDataParams[iter2->first] = &curParam;
			}
			else
			{
				if(validParams[iter2->first])
				{
					auto dataFindIter2 = foundDataParams.find(iter2->first);

					const GpuParamDataDesc* otherParam = dataFindIter2->second;
					if(!areParamsEqual(curParam, *otherParam, true))
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

	for(auto iter = paramDescs.begin(); iter != paramDescs.end(); ++iter)
	{
		const GpuParamDesc& curDesc = **iter;

		// Check sampler params
		for(auto iter2 = curDesc.Samplers.begin(); iter2 != curDesc.Samplers.end(); ++iter2)
		{
			validParams.push_back(&iter2->second);
		}

		// Check texture params
		for(auto iter2 = curDesc.Textures.begin(); iter2 != curDesc.Textures.end(); ++iter2)
		{
			validParams.push_back(&iter2->second);
		}

		// Check load-store texture params
		for(auto iter2 = curDesc.LoadStoreTextures.begin(); iter2 != curDesc.LoadStoreTextures.end(); ++iter2)
		{
			validParams.push_back(&iter2->second);
		}

		// Check buffer params
		for(auto iter2 = curDesc.Buffers.begin(); iter2 != curDesc.Buffers.end(); ++iter2)
		{
			validParams.push_back(&iter2->second);
		}
	}

	return validParams;
}

Map<String, String> determineParameterToBlockMapping(const Vector<SPtr<GpuParamDesc>>& paramDescs)
{
	Map<String, String> paramToParamBlock;

	for(auto iter = paramDescs.begin(); iter != paramDescs.end(); ++iter)
	{
		const GpuParamDesc& curDesc = **iter;
		for(auto iter2 = curDesc.Params.begin(); iter2 != curDesc.Params.end(); ++iter2)
		{
			const GpuParamDataDesc& curParam = iter2->second;

			auto iterFind = paramToParamBlock.find(curParam.Name);
			if(iterFind != paramToParamBlock.end())
				continue;

			for(auto iterBlock = curDesc.ParamBlocks.begin(); iterBlock != curDesc.ParamBlocks.end(); ++iterBlock)
			{
				if(iterBlock->second.Set == curParam.ParamBlockSet && iterBlock->second.Slot == curParam.ParamBlockSlot)
				{
					paramToParamBlock[curParam.Name] = iterBlock->second.Name;
					break;
				}
			}
		}
	}

	return paramToParamBlock;
}

UnorderedMap<ValidParamKey, String> determineValidParameters(const Vector<SPtr<GpuParamDesc>>& paramDescs, const Map<String, SHADER_DATA_PARAM_DESC>& dataParams, const Map<String, SHADER_OBJECT_PARAM_DESC>& textureParams, const Map<String, SHADER_OBJECT_PARAM_DESC>& bufferParams, const Map<String, SHADER_OBJECT_PARAM_DESC>& samplerParams)
{
	UnorderedMap<ValidParamKey, String> validParams;

	Map<String, const GpuParamDataDesc*> validDataParameters = determineValidDataParameters(paramDescs);
	Vector<const GpuParamObjectDesc*> validObjectParameters = determineValidObjectParameters(paramDescs);
	Map<String, String> paramToParamBlockMap = determineParameterToBlockMapping(paramDescs);

	// Create data param mappings
	for(auto iter = dataParams.begin(); iter != dataParams.end(); ++iter)
	{
		auto findIter = validDataParameters.find(iter->second.GpuVariableName);

		// Not valid so we skip it
		if(findIter == validDataParameters.end())
			continue;

		if(findIter->second->Type != iter->second.Type &&
		   !(iter->second.Type == GPDT_COLOR && (findIter->second->Type == GPDT_FLOAT4 || findIter->second->Type == GPDT_FLOAT3)))
		{
			BS_LOG(Warning, Material, "Ignoring shader parameter \"{0}\". Type doesn't match the one defined in the "
									  "GPU program. Shader defined type: {1} - Gpu program defined type: {2}",
				   iter->first, iter->second.Type, findIter->second->Type);
			continue;
		}

		auto findBlockIter = paramToParamBlockMap.find(iter->second.GpuVariableName);

		if(findBlockIter == paramToParamBlockMap.end())
			BS_EXCEPT(InternalErrorException, "Parameter doesn't exist in param to param block map but exists in valid param map.");

		ValidParamKey key(iter->second.GpuVariableName, MaterialParams::ParamType::Data);
		validParams.insert(std::make_pair(key, iter->first));
	}

	// Create object param mappings
	auto determineObjectMappings = [&](const Map<String, SHADER_OBJECT_PARAM_DESC>& params, MaterialParams::ParamType paramType)
	{
		for(auto iter = params.begin(); iter != params.end(); ++iter)
		{
			const Vector<String>& gpuVariableNames = iter->second.GpuVariableNames;
			for(auto iter2 = gpuVariableNames.begin(); iter2 != gpuVariableNames.end(); ++iter2)
			{
				for(auto iter3 = validObjectParameters.begin(); iter3 != validObjectParameters.end(); ++iter3)
				{
					if((*iter3)->Name == (*iter2))
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

template <bool Core>
const u32 TGpuParamsSet<Core>::kNumStages = 6;

template <bool Core>
TGpuParamsSet<Core>::TGpuParamsSet(const SPtr<TechniqueType>& technique, const ShaderType& shader, const SPtr<MaterialParamsType>& params)
	: mPassParams(technique->GetNumPasses()), mParamVersion(0)
{
	u32 numPasses = technique->GetNumPasses();

	// Create GpuParams for each pass and shader stage
	for(u32 i = 0; i < numPasses; i++)
	{
		SPtr<PassType> curPass = technique->GetPass(i);

		SPtr<GraphicsPipelineStateType> gfxPipeline = curPass->GetGraphicsPipelineState();
		if(gfxPipeline != nullptr)
			mPassParams[i] = GpuParamsType::Create(gfxPipeline);
		else
		{
			SPtr<ComputePipelineStateType> computePipeline = curPass->GetComputePipelineState();
			mPassParams[i] = GpuParamsType::Create(computePipeline);
		}
	}

	// Create and assign parameter block buffers
	Vector<SPtr<GpuParamDesc>> allParamDescs = getAllParamDescs(technique);

	//// Fill out various helper structures
	Vector<ShaderBlockDesc> paramBlockData = determineValidShareableParamBlocks(allParamDescs, shader->GetParamBlocks());
	UnorderedMap<ValidParamKey, String> validParams = determineValidParameters(
		allParamDescs,
		shader->GetDataParams(),
		shader->GetTextureParams(),
		shader->GetBufferParams(),
		shader->GetSamplerParams());

	Map<String, ParamBlockPtrType> paramBlockBuffers;

	//// Create param blocks
	for(auto& paramBlock : paramBlockData)
	{
		ParamBlockPtrType newParamBlockBuffer;
		if(!paramBlock.External)
			newParamBlockBuffer = ParamBlockType::Create(paramBlock.Size, paramBlock.Usage);

		paramBlock.SequentialIdx = (u32)mBlocks.size();

		paramBlockBuffers[paramBlock.Name] = newParamBlockBuffer;
		mBlocks.push_back(BlockInfo(paramBlock.Name, paramBlock.Set, paramBlock.Slot, newParamBlockBuffer, true));
	}

	//// Assign param block buffers and generate information about data parameters
	assert(numPasses < 64); // BlockInfo flags uses u64 for tracking usage
	for(u32 i = 0; i < numPasses; i++)
	{
		SPtr<GpuParamsType> paramPtr = mPassParams[i];
		for(u32 j = 0; j < kNumStages; j++)
		{
			GpuProgramType progType = (GpuProgramType)j;

			// Assign shareable buffers
			for(auto& block : paramBlockData)
			{
				const String& paramBlockName = block.Name;
				if(paramPtr->HasParamBlock(progType, paramBlockName))
				{
					ParamBlockPtrType blockBuffer = paramBlockBuffers[paramBlockName];

					paramPtr->SetParamBlockBuffer(progType, paramBlockName, blockBuffer);
				}
			}

			// Create non-shareable ones (these are buffers defined by default by the RHI usually)
			SPtr<GpuParamDesc> desc = paramPtr->GetParamDesc(progType);
			if(desc == nullptr)
				continue;

			for(auto iterBlockDesc = desc->ParamBlocks.begin(); iterBlockDesc != desc->ParamBlocks.end(); ++iterBlockDesc)
			{
				const GpuParamBlockDesc& blockDesc = iterBlockDesc->second;

				u32 globalBlockIdx = (u32)-1;
				if(!blockDesc.IsShareable)
				{
					ParamBlockPtrType newParamBlockBuffer = ParamBlockType::Create(blockDesc.BlockSize * sizeof(u32));

					globalBlockIdx = (u32)mBlocks.size();

					paramPtr->SetParamBlockBuffer(progType, iterBlockDesc->first, newParamBlockBuffer);
					mBlocks.emplace_back(iterBlockDesc->first, iterBlockDesc->second.Set, iterBlockDesc->second.Slot, newParamBlockBuffer, false);
				}
				else
				{
					auto iterFind = std::find_if(paramBlockData.begin(), paramBlockData.end(), [&](const auto& x)
												 { return x.Name == iterBlockDesc->first; });

					if(iterFind != paramBlockData.end())
						globalBlockIdx = iterFind->SequentialIdx;
				}

				// If this parameter block is valid, create data/struct mappings for it
				if(globalBlockIdx == (u32)-1)
					continue;

				for(auto& dataParam : desc->Params)
				{
					if(dataParam.second.ParamBlockSet != blockDesc.Set || dataParam.second.ParamBlockSlot != blockDesc.Slot)
						continue;

					ValidParamKey key(dataParam.first, MaterialParams::ParamType::Data);

					auto iterFind = validParams.find(key);
					if(iterFind == validParams.end())
						continue;

					u32 paramIdx = params->GetParamIndex(iterFind->second);

					// Parameter shouldn't be in the valid parameter list if it cannot be found
					assert(paramIdx != (u32)-1);

					mDataParamInfos.push_back(DataParamInfo());
					DataParamInfo& paramInfo = mDataParamInfos.back();
					paramInfo.ParamIdx = paramIdx;
					paramInfo.BlockIdx = globalBlockIdx;
					paramInfo.Offset = dataParam.second.CpuMemOffset;
					paramInfo.ArrayStride = dataParam.second.ArrayElementStride;
				}
			}
		}
	}

	// Add buffers defined in shader but not actually used by GPU programs (so we can check if user is providing a
	// valid buffer name)
	auto& allParamBlocks = shader->GetParamBlocks();
	for(auto& entry : allParamBlocks)
	{
		auto iterFind = std::find_if(mBlocks.begin(), mBlocks.end(), [&](auto& x)
									 { return x.Name == entry.first; });

		if(iterFind == mBlocks.end())
		{
			mBlocks.push_back(BlockInfo(entry.first, 0, 0, nullptr, true));
			mBlocks.back().IsUsed = false;
		}
	}

	// Generate information about object parameters
	bs_frame_mark();
	{
		FrameVector<ObjectParamInfo> objParamInfos;

		u32 offsetsSize = numPasses * kNumStages * 4 * sizeof(u32);
		u32* offsets = (u32*)bs_frame_alloc(offsetsSize);
		memset(offsets, 0, offsetsSize);

		// First store all objects in temporary arrays since we don't know how many of them are
		u32 totalNumObjects = 0;
		u32* stageOffsets = offsets;
		for(u32 i = 0; i < numPasses; i++)
		{
			SPtr<GpuParamsType> paramPtr = mPassParams[i];
			for(u32 j = 0; j < kNumStages; j++)
			{
				GpuProgramType progType = (GpuProgramType)j;

				auto processObjectParams = [&](const Map<String, GpuParamObjectDesc>& gpuParams,
											   u32 stageIdx, MaterialParams::ParamType paramType)
				{
					for(auto& param : gpuParams)
					{
						ValidParamKey key(param.first, paramType);

						auto iterFind = validParams.find(key);
						if(iterFind == validParams.end())
							continue;

						u32 paramIdx;
						auto result = params->GetParamIndex(iterFind->second, paramType, GPDT_UNKNOWN, 0, paramIdx);

						// Parameter shouldn't be in the valid parameter list if it cannot be found
						assert(result == MaterialParams::GetParamResult::Success);

						objParamInfos.push_back(ObjectParamInfo());
						ObjectParamInfo& paramInfo = objParamInfos.back();
						paramInfo.ParamIdx = paramIdx;
						paramInfo.SlotIdx = param.second.Slot;
						paramInfo.SetIdx = param.second.Set;

						stageOffsets[stageIdx]++;
						totalNumObjects++;
					}
				};

				SPtr<GpuParamDesc> desc = paramPtr->GetParamDesc(progType);
				if(desc == nullptr)
				{
					stageOffsets += 4;
					continue;
				}

				processObjectParams(desc->Textures, 0, MaterialParams::ParamType::Texture);
				processObjectParams(desc->LoadStoreTextures, 1, MaterialParams::ParamType::Texture);
				processObjectParams(desc->Buffers, 2, MaterialParams::ParamType::Buffer);
				processObjectParams(desc->Samplers, 3, MaterialParams::ParamType::Sampler);

				stageOffsets += 4;
			}
		}

		// Transfer all objects into their permanent storage
		u32 numBlocks = (u32)mBlocks.size();
		u32 blockBindingsSize = numBlocks * numPasses * sizeof(PassBlockBindings);
		u32 objectParamInfosSize = totalNumObjects * sizeof(ObjectParamInfo) + numPasses * sizeof(PassParamInfo);
		mData = (u8*)B3DAllocate(objectParamInfosSize + blockBindingsSize);
		u8* dataIter = mData;

		mPassParamInfos = (PassParamInfo*)dataIter;
		memset(mPassParamInfos, 0, objectParamInfosSize);
		dataIter += objectParamInfosSize;

		StageParamInfo* stageInfos = (StageParamInfo*)mPassParamInfos;

		ObjectParamInfo* objInfos = (ObjectParamInfo*)(mPassParamInfos + numPasses);
		memcpy(objInfos, objParamInfos.data(), totalNumObjects * sizeof(ObjectParamInfo));

		u32 objInfoOffset = 0;

		stageOffsets = offsets;
		for(u32 i = 0; i < numPasses; i++)
		{
			for(u32 j = 0; j < kNumStages; j++)
			{
				StageParamInfo& stage = stageInfos[i * kNumStages + j];

				if(stageOffsets[0] > 0)
				{
					u32 numEntries = stageOffsets[0];

					stage.Textures = objInfos + objInfoOffset;
					stage.NumTextures = numEntries;

					objInfoOffset += numEntries;
				}

				if(stageOffsets[1] > 0)
				{
					u32 numEntries = stageOffsets[1];

					stage.LoadStoreTextures = objInfos + objInfoOffset;
					stage.NumLoadStoreTextures = numEntries;

					objInfoOffset += numEntries;
				}

				if(stageOffsets[2] > 0)
				{
					u32 numEntries = stageOffsets[2];

					stage.Buffers = objInfos + objInfoOffset;
					stage.NumBuffers = numEntries;

					objInfoOffset += numEntries;
				}

				if(stageOffsets[3] > 0)
				{
					u32 numEntries = stageOffsets[3];

					stage.SamplerStates = objInfos + objInfoOffset;
					stage.NumSamplerStates = numEntries;

					objInfoOffset += numEntries;
				}

				stageOffsets += 4;
			}
		}

		// Determine on which passes & stages are buffers used on
		for(auto& block : mBlocks)
		{
			block.PassData = (PassBlockBindings*)dataIter;
			dataIter += sizeof(PassBlockBindings) * numPasses;
		}

		for(auto& block : mBlocks)
		{
			for(u32 i = 0; i < numPasses; i++)
			{
				SPtr<GpuParamsType> paramPtr = mPassParams[i];
				for(u32 j = 0; j < kNumStages; j++)
				{
					GpuProgramType progType = (GpuProgramType)j;

					SPtr<GpuParamDesc> curDesc = paramPtr->GetParamDesc(progType);
					if(curDesc == nullptr)
					{
						block.PassData[i].Bindings[j].Set = -1;
						block.PassData[i].Bindings[j].Slot = -1;

						continue;
					}

					auto iterFind = curDesc->ParamBlocks.find(block.Name);
					if(iterFind == curDesc->ParamBlocks.end())
					{
						block.PassData[i].Bindings[j].Set = -1;
						block.PassData[i].Bindings[j].Slot = -1;

						continue;
					}

					block.PassData[i].Bindings[j].Set = iterFind->second.Set;
					block.PassData[i].Bindings[j].Slot = iterFind->second.Slot;
				}
			}
		}

		bs_frame_free(offsets);
	}
	bs_frame_clear();
}

template <bool Core>
TGpuParamsSet<Core>::~TGpuParamsSet()
{
	// All allocations share the same memory, so we just clear it all at once
	B3DFree(mData);
}

template <bool Core>
SPtr<typename TGpuParamsSet<Core>::GpuParamsType> TGpuParamsSet<Core>::GetGpuParams(u32 passIdx)
{
	if(passIdx >= mPassParams.size())
		return nullptr;

	return mPassParams[passIdx];
}

template <bool Core>
u32 TGpuParamsSet<Core>::GetParamBlockBufferIndex(const String& name) const
{
	for(u32 i = 0; i < (u32)mBlocks.size(); i++)
	{
		const BlockInfo& block = mBlocks[i];
		if(block.Name == name)
			return i;
	}

	return -1;
}

template <bool Core>
void TGpuParamsSet<Core>::SetParamBlockBuffer(u32 index, const ParamBlockPtrType& paramBlock, bool ignoreInUpdate)
{
	BlockInfo& blockInfo = mBlocks[index];
	if(!blockInfo.Shareable)
	{
		BS_LOG(Error, RenderBackend, "Cannot set parameter block buffer with the name \"{0}\". "
									 "Buffer is not assignable. ",
			   blockInfo.Name);
		return;
	}

	if(!blockInfo.IsUsed)
		return;

	blockInfo.AllowUpdate = !ignoreInUpdate;

	if(blockInfo.Buffer != paramBlock)
	{
		blockInfo.Buffer = paramBlock;

		u32 numPasses = (u32)mPassParams.size();
		for(u32 j = 0; j < numPasses; j++)
		{
			SPtr<GpuParamsType> paramPtr = mPassParams[j];
			for(u32 i = 0; i < kNumStages; i++)
			{
				GpuProgramType progType = (GpuProgramType)i;

				const BlockBinding& binding = blockInfo.PassData[j].Bindings[progType];

				if(binding.Slot != (u32)-1)
					paramPtr->SetParamBlockBuffer(binding.Set, binding.Slot, paramBlock);
			}
		}
	}
}

template <bool Core>
void TGpuParamsSet<Core>::SetParamBlockBuffer(const String& name, const ParamBlockPtrType& paramBlock, bool ignoreInUpdate)
{
	u32 bufferIdx = GetParamBlockBufferIndex(name);
	if(bufferIdx == (u32)-1)
	{
		BS_LOG(Error, RenderBackend, "Cannot set parameter block buffer with the name \"{0}\". Buffer name not found. ", name);
		return;
	}

	SetParamBlockBuffer(bufferIdx, paramBlock, ignoreInUpdate);
}

template <bool Core>
void TGpuParamsSet<Core>::Update(const SPtr<MaterialParamsType>& params, float t, bool updateAll)
{
	// Note: Instead of iterating over every single parameter, it might be more efficient for @p params to keep
	// a ring buffer and a version number. Then we could just iterate over the ring buffer and only access dirty
	// parameters. If the version number is too high (larger than ring buffer can store), then we force update for all.

	// Update data params
	for(auto& paramInfo : mDataParamInfos)
	{
		ParamBlockPtrType paramBlock = mBlocks[paramInfo.BlockIdx].Buffer;
		if(paramBlock == nullptr || !mBlocks[paramInfo.BlockIdx].AllowUpdate)
			continue;

		const MaterialParams::ParamData* materialParamInfo = params->GetParamData(paramInfo.ParamIdx);
		u32 arraySize = materialParamInfo->ArraySize == 0 ? 1 : materialParamInfo->ArraySize;

		bool isAnimated = false;
		for(u32 i = 0; i < arraySize; i++)
		{
			isAnimated = params->IsAnimated(*materialParamInfo, i);
			if(isAnimated)
				break;
		}

		if(materialParamInfo->Version <= mParamVersion && !updateAll && !isAnimated)
			continue;

		if(materialParamInfo->DataType != GPDT_STRUCT)
		{
			const GpuParamDataTypeInfo& typeInfo = GpuParams::kParamSizes.Lookup[(int)materialParamInfo->DataType];

			u32 paramSize;
			if(materialParamInfo->DataType != GPDT_COLOR)
				paramSize = typeInfo.NumColumns * typeInfo.NumRows * typeInfo.BaseTypeSize;
			else
				paramSize = paramInfo.ArrayStride * typeInfo.BaseTypeSize;

			u8* data = params->GetData(materialParamInfo->Index);
			if(!isAnimated)
			{
				const bool transposeMatrices = ct::GetRenderBackendCapabilities().Conventions.MatrixOrder == Conventions::MatrixOrder::ColumnMajor;
				if(transposeMatrices)
				{
					auto writeTransposed = [&paramInfo, &paramSize, &arraySize, &paramBlock, data](auto& temp)
					{
						for(u32 i = 0; i < arraySize; i++)
						{
							u32 readOffset = i * paramSize;
							memcpy(&temp, data + readOffset, paramSize);
							auto transposed = temp.Transpose();

							u32 writeOffset = (paramInfo.Offset + paramInfo.ArrayStride * i) * sizeof(u32);
							paramBlock->Write(writeOffset, &transposed, paramSize);
						}
					};

					switch(materialParamInfo->DataType)
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
							for(u32 i = 0; i < arraySize; i++)
							{
								u32 arrayOffset = i * paramSize;
								u32 writeOffset = (paramInfo.Offset + paramInfo.ArrayStride * i) * sizeof(u32);
								paramBlock->Write(writeOffset, data + arrayOffset, paramSize);
							}
							break;
						}
					}
				}
				else
				{
					for(u32 i = 0; i < arraySize; i++)
					{
						u32 readOffset = i * paramSize;
						u32 writeOffset = (paramInfo.Offset + paramInfo.ArrayStride * i) * sizeof(u32);
						paramBlock->Write(writeOffset, data + readOffset, paramSize);
					}
				}
			}
			else // Animated
			{
				if(materialParamInfo->DataType == GPDT_FLOAT1)
				{
					assert(paramSize == sizeof(float));

					for(u32 i = 0; i < arraySize; i++)
					{
						u32 readOffset = i * paramSize;
						u32 writeOffset = (paramInfo.Offset + paramInfo.ArrayStride * i) * sizeof(u32);

						float value;
						if(params->IsAnimated(*materialParamInfo, i))
						{
							const TAnimationCurve<float>& curve = params->template GetCurveParam<float>(*materialParamInfo, i);

							value = curve.Evaluate(t, true);
						}
						else
							memcpy(&value, data + readOffset, paramSize);

						paramBlock->Write(writeOffset, &value, paramSize);
					}
				}
				else if(materialParamInfo->DataType == GPDT_FLOAT4)
				{
					assert(paramSize == sizeof(Rect2));

					CoreVariantHandleType<SpriteTexture, Core> spriteTexture =
						params->GetOwningSpriteTexture(*materialParamInfo);

					u32 writeOffset = paramInfo.Offset * sizeof(u32);
					Rect2 uv = Rect2(0.0f, 0.0f, 1.0f, 1.0f);
					if(spriteTexture != nullptr)
						uv = spriteTexture->Evaluate(t);

					paramBlock->Write(writeOffset, &uv, paramSize);

					// Only the first array element receives sprite UVs, the rest are treated as normal
					for(u32 i = 1; i < arraySize; i++)
					{
						u32 readOffset = i * paramSize;
						writeOffset = (paramInfo.Offset + paramInfo.ArrayStride * i) * sizeof(u32);

						paramBlock->Write(writeOffset, data + readOffset, paramSize);
					}
				}
				else if(materialParamInfo->DataType == GPDT_COLOR)
				{
					for(u32 i = 0; i < arraySize; i++)
					{
						assert(paramSize == sizeof(Color));

						u32 readOffset = i * paramSize;
						u32 writeOffset = (paramInfo.Offset + paramInfo.ArrayStride * i) * sizeof(u32);

						Color value;
						if(params->IsAnimated(*materialParamInfo, i))
						{
							const ColorGradientHDR& gradient = params->GetColorGradientParam(*materialParamInfo, i);

							const float wrappedT = Math::Repeat(t, gradient.GetDuration());
							value = gradient.Evaluate(wrappedT);
						}
						else
							memcpy(&value, data + readOffset, paramSize);

						paramBlock->Write(writeOffset, &value, paramSize);
					}
				}
			}
		}
		else
		{
			u32 paramSize = params->GetStructSize(*materialParamInfo);
			void* paramData = bs_stack_alloc(paramSize);
			for(u32 i = 0; i < arraySize; i++)
			{
				params->GetStructData(*materialParamInfo, paramData, paramSize, i);

				u32 writeOffset = (paramInfo.Offset + paramInfo.ArrayStride * i) * sizeof(u32);
				paramBlock->Write(writeOffset, paramData, paramSize);
			}
			bs_stack_free(paramData);
		}
	}

	// Update object params
	const auto numPasses = (u32)mPassParams.size();

	for(u32 i = 0; i < numPasses; i++)
	{
		SPtr<GpuParamsType> paramPtr = mPassParams[i];

		for(u32 j = 0; j < kNumStages; j++)
		{
			const StageParamInfo& stageInfo = mPassParamInfos[i].Stages[j];

			for(u32 k = 0; k < stageInfo.NumTextures; k++)
			{
				const ObjectParamInfo& paramInfo = stageInfo.Textures[k];

				const MaterialParams::ParamData* materialParamInfo = params->GetParamData(paramInfo.ParamIdx);
				if(materialParamInfo->Version <= mParamVersion && !updateAll)
					continue;

				TextureSurface surface;
				TextureType texture;
				params->GetTexture(*materialParamInfo, texture, surface);

				paramPtr->SetTexture(paramInfo.SetIdx, paramInfo.SlotIdx, texture, surface);
			}

			for(u32 k = 0; k < stageInfo.NumLoadStoreTextures; k++)
			{
				const ObjectParamInfo& paramInfo = stageInfo.LoadStoreTextures[k];

				const MaterialParams::ParamData* materialParamInfo = params->GetParamData(paramInfo.ParamIdx);
				if(materialParamInfo->Version <= mParamVersion && !updateAll)
					continue;

				TextureSurface surface;
				TextureType texture;
				params->GetLoadStoreTexture(*materialParamInfo, texture, surface);

				paramPtr->SetLoadStoreTexture(paramInfo.SetIdx, paramInfo.SlotIdx, texture, surface);
			}

			for(u32 k = 0; k < stageInfo.NumBuffers; k++)
			{
				const ObjectParamInfo& paramInfo = stageInfo.Buffers[k];

				const MaterialParams::ParamData* materialParamInfo = params->GetParamData(paramInfo.ParamIdx);
				if(materialParamInfo->Version <= mParamVersion && !updateAll)
					continue;

				BufferType buffer;
				params->GetBuffer(*materialParamInfo, buffer);

				paramPtr->SetBuffer(paramInfo.SetIdx, paramInfo.SlotIdx, buffer);
			}

			for(u32 k = 0; k < stageInfo.NumSamplerStates; k++)
			{
				const ObjectParamInfo& paramInfo = stageInfo.SamplerStates[k];

				const MaterialParams::ParamData* materialParamInfo = params->GetParamData(paramInfo.ParamIdx);
				if(materialParamInfo->Version <= mParamVersion && !updateAll)
					continue;

				SamplerStateType samplerState;
				params->GetSamplerState(*materialParamInfo, samplerState);

				paramPtr->SetSamplerState(paramInfo.SetIdx, paramInfo.SlotIdx, samplerState);
			}
		}

		paramPtr->MarkCoreDirtyInternal();
	}

	mParamVersion = params->GetParamVersion();
}

template class TGpuParamsSet<false>;
template class TGpuParamsSet<true>;
