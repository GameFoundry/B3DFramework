//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsGpuParamsSet.h"
#include "BsShader.h"
#include "BsTechnique.h"
#include "BsPass.h"
#include "BsGpuProgram.h"
#include "BsGpuParamDesc.h"
#include "BsGpuParamBlockBuffer.h"

namespace BansheeEngine
{
	struct ShaderBlockDesc
	{
		String name;
		GpuParamBlockUsage usage;
		int size;
		bool external;
	};

	Vector<SPtr<GpuParamDesc>> getAllParamDescs(const SPtr<Technique>& technique)
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

	Vector<SPtr<GpuParamDesc>> getAllParamDescs(const SPtr<TechniqueCore>& technique)
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
			BlockInfo(const String& name, const SPtr<GpuParamDesc>& paramDesc, bool isValid = true)
				:name(name), paramDesc(paramDesc), isValid(isValid)
			{ }

			String name;
			SPtr<GpuParamDesc> paramDesc;
			bool isValid;
		};

		// Make sure param blocks with the same name actually contain the same fields
		Map<String, BlockInfo> uniqueParamBlocks;

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
					uniqueParamBlocks[blockIter->first] = BlockInfo(blockIter->first, *iter);
					continue;
				}

				// The block was already determined as invalid, no need to check further
				if (!iterFind->second.isValid)
					continue;

				String otherBlockName = iterFind->second.name;
				SPtr<GpuParamDesc> otherDesc = iterFind->second.paramDesc;

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
					LOGWRN("Found two param blocks with the same name but different contents: " + blockIter->first);
					uniqueParamBlocks[blockIter->first] = BlockInfo(blockIter->first, nullptr, false);
				}
			}
		}

		Vector<ShaderBlockDesc> output;
		for (auto& entry : uniqueParamBlocks)
		{
			if (!entry.second.isValid)
				continue;

			ShaderBlockDesc shaderBlockDesc;
			shaderBlockDesc.external = false;
			shaderBlockDesc.usage = GPBU_STATIC;
			shaderBlockDesc.size = 0;
			shaderBlockDesc.name = entry.first;

			auto iterFind = shaderDesc.find(entry.first);
			if (iterFind != shaderDesc.end())
			{
				shaderBlockDesc.external = iterFind->second.shared || iterFind->second.rendererSemantic != StringID::NONE;
				shaderBlockDesc.usage = iterFind->second.usage;
			}

			for (auto iter2 = paramDescs.begin(); iter2 != paramDescs.end(); ++iter2)
			{
				auto findParamBlockDesc = (*iter2)->paramBlocks.find(entry.first);

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

	template<bool Core>
	const UINT32 TGpuParamsSet<Core>::NUM_PARAMS = 6;

	template<bool Core>
	TGpuParamsSet<Core>::TGpuParamsSet(const SPtr<TechniqueType>& technique, const ShaderType& shader)
		:mPassParams(technique->getNumPasses())
	{
		UINT32 numPasses = technique->getNumPasses();

		// Create GpuParams for each pass and shader stage
		for (UINT32 i = 0; i < numPasses; i++)
		{
			SPtr<PassType> curPass = technique->getPass(i);

			GpuProgramPtrType vertProgram = curPass->getVertexProgram();
			if (vertProgram)
				mPassParams[i].vertex = vertProgram->createParameters();

			GpuProgramPtrType fragProgram = curPass->getFragmentProgram();
			if (fragProgram)
				mPassParams[i].fragment = fragProgram->createParameters();

			GpuProgramPtrType geomProgram = curPass->getGeometryProgram();
			if (geomProgram)
				mPassParams[i].geometry = geomProgram->createParameters();

			GpuProgramPtrType hullProgram = curPass->getHullProgram();
			if (hullProgram)
				mPassParams[i].hull = hullProgram->createParameters();

			GpuProgramPtrType domainProgram = curPass->getDomainProgram();
			if (domainProgram)
				mPassParams[i].domain = domainProgram->createParameters();

			GpuProgramPtrType computeProgram = curPass->getComputeProgram();
			if (computeProgram)
				mPassParams[i].compute = computeProgram->createParameters();
		}

		// Create and assign parameter block buffers
		Vector<SPtr<GpuParamDesc>> allParamDescs = getAllParamDescs(technique);

		//// Fill out various helper structures
		Vector<ShaderBlockDesc> paramBlockData = determineValidShareableParamBlocks(allParamDescs, shader->getParamBlocks());

		Map<String, ParamBlockPtrType> paramBlockBuffers;

		//// Create param blocks
		for (auto& paramBlock : paramBlockData)
		{
			ParamBlockPtrType newParamBlockBuffer;
			if (!paramBlock.external)
				newParamBlockBuffer = ParamBlockType::create(paramBlock.size, paramBlock.usage);

			paramBlockBuffers[paramBlock.name] = newParamBlockBuffer;
		}

		//// Assign param block buffers
		for (UINT32 i = 0; i < numPasses; i++)
		{
			for (UINT32 j = 0; j < NUM_PARAMS; j++)
			{
				GpuParamsType paramPtr = getParamByIdx(j, i);
				if (paramPtr != nullptr)
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

					// Create non-shareable ones (these are buffers defined by default by the RHI usually)
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
	}

	template<bool Core>
	typename TGpuParamsSet<Core>::GpuParamsType TGpuParamsSet<Core>::getGpuParams(GpuProgramType type, UINT32 passIdx)
	{
		if (passIdx >= mPassParams.size())
			return nullptr;

		switch (type)
		{
		case GPT_VERTEX_PROGRAM:
			return mPassParams[passIdx].vertex;
		case GPT_FRAGMENT_PROGRAM:
			return mPassParams[passIdx].fragment;
		case GPT_GEOMETRY_PROGRAM:
			return mPassParams[passIdx].geometry;
		case GPT_HULL_PROGRAM:
			return mPassParams[passIdx].hull;
		case GPT_DOMAIN_PROGRAM:
			return mPassParams[passIdx].domain;
		case GPT_COMPUTE_PROGRAM:
			return mPassParams[passIdx].compute;
		}

		return nullptr;
	}

	template<bool Core>
	void TGpuParamsSet<Core>::setParamBlockBuffer(const String& name, const ParamBlockPtrType& paramBlock)
	{
		UINT32 numPasses = (UINT32)mPassParams.size();
		for (UINT32 j = 0; j < numPasses; j++)
		{
			for (UINT32 i = 0; i < NUM_PARAMS; i++)
			{
				GpuParamsType paramPtr = getParamByIdx(i);
				if (paramPtr != nullptr)
				{
					if (paramPtr->hasParamBlock(name))
						paramPtr->setParamBlockBuffer(name, paramBlock);
				}
			}
		}
	}

	template class TGpuParamsSet <false>;
	template class TGpuParamsSet <true>;
}