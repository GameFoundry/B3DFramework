//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Material/B3DMaterialParameterAdapter.h"

#include "B3DApplication.h"
#include "Material/B3DShader.h"
#include "Material/B3DVariation.h"
#include "Material/B3DPass.h"
#include "RenderAPI/B3DGpuProgram.h"
#include "RenderAPI/B3DGpuPipelineState.h"
#include "RenderAPI/B3DGpuPipelineParameterLayout.h"
#include "Material/B3DMaterialParameters.h"
#include "RenderAPI/B3DGpuProgramParameterDescription.h"
#include "Animation/B3DAnimationCurve.h"
#include "Image/B3DColorGradient.h"
#include "Image/B3DSpriteTexture.h"
#include "RenderAPI/B3DGpuCommandBuffer.h"
#include "RenderAPI/B3DGpuDevice.h"
#include "RenderAPI/B3DGpuDeviceCapabilities.h"
#include "RenderAPI/B3DGpuParameterSetPool.h"
#include "Renderer/B3DRenderer.h"

using namespace b3d;

/** Uniquely identifies a GPU parameter. */
struct ValidParamKey
{
	ValidParamKey(const String& name, const MaterialParameters::ParamType& type)
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
	MaterialParameters::ParamType Type;
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
		b3d::B3DCombineHash(hash, key.Name);
		b3d::B3DCombineHash(hash, key.Type);

		return hash;
	}
};
} // namespace std

/** @endcond */

using namespace b3d;

struct ShaderUniformBuffer
{
	String Name;
	GpuBufferFlags Flags;
	int Size;
	bool External;
	u32 SequentialIdx;
	u32 Set;
	u32 Slot;
};

Array<SPtr<GpuProgramParameterDescription>, GPT_COUNT> GatherParameterDescriptions(const SPtr<Pass>& pass)
{
	Array<SPtr<GpuProgramParameterDescription>, GPT_COUNT> parameterDescriptions;

	const SPtr<GpuGraphicsPipelineState>& graphicsPipeline = pass->GetGraphicsPipelineState();
	if(graphicsPipeline)
	{
		SPtr<GpuProgram> vertProgram = graphicsPipeline->GetVertexProgram();
		if(vertProgram)
			parameterDescriptions[GPT_VERTEX_PROGRAM] = vertProgram->GetParameterDescription();

		SPtr<GpuProgram> fragProgram = graphicsPipeline->GetFragmentProgram();
		if(fragProgram)
			parameterDescriptions[GPT_FRAGMENT_PROGRAM] = fragProgram->GetParameterDescription();

		SPtr<GpuProgram> geomProgram = graphicsPipeline->GetGeometryProgram();
		if(geomProgram)
			parameterDescriptions[GPT_GEOMETRY_PROGRAM] = geomProgram->GetParameterDescription();

		SPtr<GpuProgram> hullProgram = graphicsPipeline->GetHullProgram();
		if(hullProgram)
			parameterDescriptions[GPT_HULL_PROGRAM] = hullProgram->GetParameterDescription();

		SPtr<GpuProgram> domainProgram = graphicsPipeline->GetDomainProgram();
		if(domainProgram)
			parameterDescriptions[GPT_DOMAIN_PROGRAM] = domainProgram->GetParameterDescription();
	}

	const SPtr<GpuComputePipelineState>& computePipeline = pass->GetComputePipelineState();
	if(computePipeline)
	{
		SPtr<GpuProgram> computeProgram = computePipeline->GetProgram();
		if(computeProgram)
			parameterDescriptions[GPT_COMPUTE_PROGRAM] = computeProgram->GetParameterDescription();
	}

	return parameterDescriptions;
}

Array<SPtr<GpuProgramParameterDescription>, GPT_COUNT> GatherParameterDescriptions(const SPtr<render::Pass>& pass)
{
	Array<SPtr<GpuProgramParameterDescription>, GPT_COUNT> parameterDescriptions;

	// Make sure all gpu programs are fully loaded
	const SPtr<GpuGraphicsPipelineState>& graphicsPipeline = pass->GetGraphicsPipelineState();
	if(graphicsPipeline)
	{
		SPtr<GpuProgram> vertProgram = graphicsPipeline->GetVertexProgram();
		if(vertProgram)
			parameterDescriptions[GPT_VERTEX_PROGRAM] = vertProgram->GetParameterDescription();

		SPtr<GpuProgram> fragProgram = graphicsPipeline->GetFragmentProgram();
		if(fragProgram)
			parameterDescriptions[GPT_FRAGMENT_PROGRAM] = fragProgram->GetParameterDescription();

		SPtr<GpuProgram> geomProgram = graphicsPipeline->GetGeometryProgram();
		if(geomProgram)
			parameterDescriptions[GPT_GEOMETRY_PROGRAM] = geomProgram->GetParameterDescription();

		SPtr<GpuProgram> hullProgram = graphicsPipeline->GetHullProgram();
		if(hullProgram)
			parameterDescriptions[GPT_HULL_PROGRAM] = hullProgram->GetParameterDescription();

		SPtr<GpuProgram> domainProgram = graphicsPipeline->GetDomainProgram();
		if(domainProgram)
			parameterDescriptions[GPT_DOMAIN_PROGRAM] = domainProgram->GetParameterDescription();
	}

	const SPtr<GpuComputePipelineState>& computePipeline = pass->GetComputePipelineState();
	if(computePipeline)
	{
		SPtr<GpuProgram> computeProgram = computePipeline->GetProgram();
		if(computeProgram)
			parameterDescriptions[GPT_COMPUTE_PROGRAM] = computeProgram->GetParameterDescription();
	}

	return parameterDescriptions;
}

bool AreParamsEqual(const GpuUniformBufferMemberInformation& paramA, const GpuUniformBufferMemberInformation& paramB, bool ignoreBufferOffsets)
{
	bool equal = paramA.ArraySize == paramB.ArraySize && paramA.ElementSize == paramB.ElementSize && paramA.Type == paramB.Type && paramA.ArrayElementStride == paramB.ArrayElementStride;

	if(!ignoreBufferOffsets)
		equal &= paramA.CpuOffset == paramB.CpuOffset && paramA.GpuOffset == paramB.GpuOffset;

	return equal;
}

Vector<ShaderUniformBuffer> DetermineValidShareableUniformBuffers(const Vector<SPtr<GpuProgramParameterDescription>>& paramDescs, const Map<String, ShaderUniformBufferInformation>& shaderUniformInformation)
{
	struct UniformBufferInfo
	{
		UniformBufferInfo() {}

		UniformBufferInfo(const GpuUniformBufferInformation* uniformBufferDescriptor, const SPtr<GpuProgramParameterDescription>& paramDesc, bool isValid = true)
			: BufferInformation(uniformBufferDescriptor), ParamDesc(paramDesc), IsValid(isValid)
		{}

		const GpuUniformBufferInformation* BufferInformation;
		SPtr<GpuProgramParameterDescription> ParamDesc;
		bool IsValid;
		u32 Set;
		u32 Slot;
	};

	// Make sure uniform buffers with the same name actually contain the same fields
	Map<String, UniformBufferInfo> uniqueUniformBuffers;

	for(auto iter = paramDescs.begin(); iter != paramDescs.end(); ++iter)
	{
		const GpuProgramParameterDescription& curDesc = **iter;
		for(auto bufferIter = curDesc.UniformBuffers.begin(); bufferIter != curDesc.UniformBuffers.end(); ++bufferIter)
		{
			const GpuUniformBufferInformation& curBlock = bufferIter->second;

			if(!curBlock.IsShareable) // Non-shareable buffers are handled differently, they're allowed same names
				continue;

			auto iterFind = uniqueUniformBuffers.find(bufferIter->first);
			if(iterFind == uniqueUniformBuffers.end())
			{
				uniqueUniformBuffers[bufferIter->first] = UniformBufferInfo(&curBlock, *iter);
				continue;
			}

			const GpuUniformBufferInformation& otherBlock = *iterFind->second.BufferInformation;

			// The buffer was already determined as invalid, no need to check further
			if(!iterFind->second.IsValid)
				continue;

			String otherBlockName = otherBlock.Name;
			SPtr<GpuProgramParameterDescription> otherDesc = iterFind->second.ParamDesc;

			for(auto myParamIter = curDesc.UniformBufferMembers.begin(); myParamIter != curDesc.UniformBufferMembers.end(); ++myParamIter)
			{
				const GpuUniformBufferMemberInformation& myParam = myParamIter->second;

				if(myParam.ParentUniformBufferSet != curBlock.Set || myParam.ParentUniformBufferSlot != curBlock.Slot)
					continue; // Param is in another buffer, so we will check it when its time for that buffer

				auto otherParamFind = otherDesc->UniformBufferMembers.find(myParamIter->first);

				// Cannot find other param, buffers aren't equal
				if(otherParamFind == otherDesc->UniformBufferMembers.end())
					break;

				const GpuUniformBufferMemberInformation& otherParam = otherParamFind->second;

				if(!AreParamsEqual(myParam, otherParam, false) || curBlock.Name != otherBlockName)
					break;
			}

			// Note: Ignoring mismatched buffers for now, because glslang parser doesn't report dead uniform entries,
			// meaning identical buffers can have different sets of uniforms reported depending on which are unused.
			// if (!isBlockValid)
			//{
			//	LOGWRN("Found two uniform buffers with the same name but different contents: " + blockIter->first);
			//	uniqueParamBlocks[blockIter->first] = BlockInfo(&curBlock, nullptr, false);

			//	continue;
			//}
		}
	}

	Vector<ShaderUniformBuffer> output;
	for(auto& entry : uniqueUniformBuffers)
	{
		if(!entry.second.IsValid)
			continue;

		const GpuUniformBufferInformation& curBlock = *entry.second.BufferInformation;

		ShaderUniformBuffer shaderBlockDesc;
		shaderBlockDesc.External = false;
		shaderBlockDesc.Flags = GpuBufferFlag::StoreOnGPU;
		shaderBlockDesc.Size = curBlock.Size * sizeof(u32);
		shaderBlockDesc.Name = entry.first;
		shaderBlockDesc.Set = curBlock.Set;
		shaderBlockDesc.Slot = curBlock.Slot;

		auto iterFind = shaderUniformInformation.find(entry.first);
		if(iterFind != shaderUniformInformation.end())
		{
			shaderBlockDesc.External = iterFind->second.Shared || iterFind->second.RendererSemantic != StringID::kNone;
			shaderBlockDesc.Flags = iterFind->second.Flags;
		}

		output.push_back(shaderBlockDesc);
	}

	return output;
}

Map<String, const GpuUniformBufferMemberInformation*> DetermineValidDataParameters(const Vector<SPtr<GpuProgramParameterDescription>>& paramDescs)
{
	Map<String, const GpuUniformBufferMemberInformation*> foundDataParams;
	Map<String, bool> validParams;

	for(auto iter = paramDescs.begin(); iter != paramDescs.end(); ++iter)
	{
		const GpuProgramParameterDescription& curDesc = **iter;

		// Check regular data params
		for(auto iter2 = curDesc.UniformBufferMembers.begin(); iter2 != curDesc.UniformBufferMembers.end(); ++iter2)
		{
			const GpuUniformBufferMemberInformation& curParam = iter2->second;

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

					const GpuUniformBufferMemberInformation* otherParam = dataFindIter2->second;
					if(!AreParamsEqual(curParam, *otherParam, true))
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

Vector<const GpuObjectParameterInformation*> DetermineValidObjectParameters(const Vector<SPtr<GpuProgramParameterDescription>>& paramDescs)
{
	Vector<const GpuObjectParameterInformation*> validParams;

	for(auto iter = paramDescs.begin(); iter != paramDescs.end(); ++iter)
	{
		const GpuProgramParameterDescription& curDesc = **iter;

		// Check sampler params
		for(auto iter2 = curDesc.Samplers.begin(); iter2 != curDesc.Samplers.end(); ++iter2)
		{
			validParams.push_back(&iter2->second);
		}

		// Check texture params
		for(auto iter2 = curDesc.SampledTextures.begin(); iter2 != curDesc.SampledTextures.end(); ++iter2)
		{
			validParams.push_back(&iter2->second);
		}

		// Check load-store texture params
		for(auto iter2 = curDesc.StorageTextures.begin(); iter2 != curDesc.StorageTextures.end(); ++iter2)
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

Map<String, String> DetermineMemberToUniformBufferMapping(const Vector<SPtr<GpuProgramParameterDescription>>& memberDescriptors)
{
	Map<String, String> memberToUniformBuffer;

	for(auto iter = memberDescriptors.begin(); iter != memberDescriptors.end(); ++iter)
	{
		const GpuProgramParameterDescription& curDesc = **iter;
		for(auto iter2 = curDesc.UniformBufferMembers.begin(); iter2 != curDesc.UniformBufferMembers.end(); ++iter2)
		{
			const GpuUniformBufferMemberInformation& curParam = iter2->second;

			auto iterFind = memberToUniformBuffer.find(curParam.Name);
			if(iterFind != memberToUniformBuffer.end())
				continue;

			for(auto iterBlock = curDesc.UniformBuffers.begin(); iterBlock != curDesc.UniformBuffers.end(); ++iterBlock)
			{
				if(iterBlock->second.Set == curParam.ParentUniformBufferSet && iterBlock->second.Slot == curParam.ParentUniformBufferSlot)
				{
					memberToUniformBuffer[curParam.Name] = iterBlock->second.Name;
					break;
				}
			}
		}
	}

	return memberToUniformBuffer;
}

UnorderedMap<ValidParamKey, String> DetermineValidParameters(const Vector<SPtr<GpuProgramParameterDescription>>& paramDescs, const Map<String, ShaderDataParameterInformation>& dataParams, const Map<String, ShaderObjectParameterInformation>& textureParams, const Map<String, ShaderObjectParameterInformation>& bufferParams, const Map<String, ShaderObjectParameterInformation>& samplerParams)
{
	UnorderedMap<ValidParamKey, String> validParams;

	Map<String, const GpuUniformBufferMemberInformation*> validDataParameters = DetermineValidDataParameters(paramDescs);
	Vector<const GpuObjectParameterInformation*> validObjectParameters = DetermineValidObjectParameters(paramDescs);
	Map<String, String> memberToUniformBuffer = DetermineMemberToUniformBufferMapping(paramDescs);

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
			B3D_LOG(Warning, LogMaterial, "Ignoring shader parameter \"{0}\". Type doesn't match the one defined in the "
									  "GPU program. Shader defined type: {1} - Gpu program defined type: {2}",
				   iter->first, iter->second.Type, findIter->second->Type);
			continue;
		}

		auto findBlockIter = memberToUniformBuffer.find(iter->second.GpuVariableName);

		if(!B3D_ENSURE_LOG(findBlockIter != memberToUniformBuffer.end(), "Parameter doesn't exist in param to uniform buffer map but exists in valid param map."))
			continue;

		ValidParamKey key(iter->second.GpuVariableName, MaterialParameters::ParamType::Data);
		validParams.insert(std::make_pair(key, iter->first));
	}

	// Create object param mappings
	auto fnDetermineObjectMappings = [&](const Map<String, ShaderObjectParameterInformation>& params, MaterialParameters::ParamType paramType)
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

	fnDetermineObjectMappings(textureParams, MaterialParameters::ParamType::Texture);
	fnDetermineObjectMappings(samplerParams, MaterialParameters::ParamType::Sampler);
	fnDetermineObjectMappings(bufferParams, MaterialParameters::ParamType::Buffer);

	return validParams;
}


template<class T>
SPtr<T> CreateGpuBuffer(const GpuBufferCreateInformation& gpuBufferCreateInformation)
{
	return nullptr;
}

template<>
SPtr<GpuBuffer> CreateGpuBuffer(const GpuBufferCreateInformation& gpuBufferCreateInformation)
{
	return GpuBuffer::Create(gpuBufferCreateInformation);
}

template<>
SPtr<render::GpuBuffer> CreateGpuBuffer(const GpuBufferCreateInformation& gpuBufferCreateInformation)
{
	const SPtr<GpuDevice>& device = GetApplication().GetPrimaryGpuDevice();
	return device->CreateGpuBuffer(gpuBufferCreateInformation);
}

template <bool IsRenderProxy>
SPtr<CoreVariantType<GpuParameterSet, IsRenderProxy>> CreateGpuParameterSet(const SPtr<GpuPipelineParameterSetLayout>& parameterSetLayout, u32 setIndex)
{
	return nullptr;
}

template <>
SPtr<GpuParameterSet> CreateGpuParameterSet<false>(const SPtr<GpuPipelineParameterSetLayout>& parameterSetLayout, u32 setIndex)
{
	return GpuParameterSet::Create(parameterSetLayout, setIndex);
}

template <>
SPtr<render::GpuParameterSet> CreateGpuParameterSet<true>(const SPtr<GpuPipelineParameterSetLayout>& parameterSetLayout, u32 setIndex)
{
	GpuParameterSetPool& pool = render::GetRenderer()->GetParameterSetPool();
	return pool.Create(parameterSetLayout, setIndex);
}

template <bool IsRenderProxy>
const u32 TMaterialParameterAdapter<IsRenderProxy>::kNumStages = 6;

template <bool IsRenderProxy>
TMaterialParameterAdapter<IsRenderProxy>::TMaterialParameterAdapter(const SPtr<VariationType>& variation, const ShaderType& shader, const SPtr<MaterialParametersType>& materialParameters)
	: mGpuParametersPerPass(variation->GetPassCount()), mParamVersion(0)
{
	const u32 passCount = variation->GetPassCount();

	// Create GpuParameterSet for each pass and descriptor set
	Vector<SPtr<GpuProgramParameterDescription>> allParameterDescriptions;
	Vector<Array<SPtr<GpuProgramParameterDescription>, GPT_COUNT>> parameterDescriptionsPerPass;
	for(u32 passIndex = 0; passIndex < passCount; passIndex++)
	{
		SPtr<PassType> curPass = variation->GetPass(passIndex);

		SPtr<GpuPipelineParameterLayout> parameterLayout;
		SPtr<GpuGraphicsPipelineState> gfxPipeline = curPass->GetGraphicsPipelineState();
		if(gfxPipeline != nullptr)
			parameterLayout = gfxPipeline->GetParameterLayout();
		else
		{
			SPtr<GpuComputePipelineState> computePipeline = curPass->GetComputePipelineState();
			parameterLayout = computePipeline->GetParameterLayout();
		}

		// Create one GpuParameterSet for each descriptor set that has resources
		const u32 setCount = parameterLayout->GetSetCount();
		mGpuParametersPerPass[passIndex].Resize(setCount);
		for(u32 setIndex = 0; setIndex < setCount; setIndex++)
		{
			const SPtr<GpuPipelineParameterSetLayout>& layoutSet = parameterLayout->GetSet(setIndex);

			const u32 resourceCount = layoutSet->GetResourceCount();
			if(resourceCount > 0)
				mGpuParametersPerPass[passIndex][setIndex] = CreateGpuParameterSet<IsRenderProxy>(layoutSet, setIndex);
		}

		parameterDescriptionsPerPass.push_back(GatherParameterDescriptions(curPass));
		for(const auto& entry : parameterDescriptionsPerPass.back())
		{
			if(entry)
				allParameterDescriptions.push_back(entry);
		}
	}

	// Create and assign uniform buffers
	//// Fill out various helper structures
	Vector<ShaderUniformBuffer> uniformBufferData = DetermineValidShareableUniformBuffers(allParameterDescriptions, shader->GetUniformBuffers());
	UnorderedMap<ValidParamKey, String> validParams = DetermineValidParameters(
		allParameterDescriptions,
		shader->GetDataParameters(),
		shader->GetTextureParameters(),
		shader->GetBufferParameters(),
		shader->GetSamplerParameters());

	Map<String, UniformBufferPointerType> uniformBuffers;

	//// Create uniform buffers
	for(auto& uniformBufferData : uniformBufferData)
	{
		UniformBufferPointerType newUniformBuffer;
		if(!uniformBufferData.External)
			newUniformBuffer = CreateGpuBuffer<UniformBufferType>(GpuBufferCreateInformation::CreateUniform(uniformBufferData.Size, uniformBufferData.Flags));

		uniformBufferData.SequentialIdx = (u32)mUniformBuffers.size();

		uniformBuffers[uniformBufferData.Name] = newUniformBuffer;
		mUniformBuffers.push_back(UniformBufferInfo(uniformBufferData.Name, uniformBufferData.Set, uniformBufferData.Slot, newUniformBuffer, 0, true));
	}

	//// Assign uniform buffers and generate information about data parameters
	B3D_ASSERT(passCount < 64); // BlockInfo flags uses u64 for tracking usage
	for(u32 passIndex = 0; passIndex < passCount; passIndex++)
	{
		TInlineArray<SPtr<GpuParametersType>, 4>& gpuParametersForPass = mGpuParametersPerPass[passIndex];
		const Array<SPtr<GpuProgramParameterDescription>, GPT_COUNT>& parameterDescriptionsForPass = parameterDescriptionsPerPass[passIndex];

		for(u32 j = 0; j < kNumStages; j++)
		{
			// Assign shareable buffers to all GpuParameterSet objects that have them
			for(auto& buffer : uniformBufferData)
			{
				const String& uniformBufferName = buffer.Name;
				UniformBufferPointerType uniformBuffer = uniformBuffers[uniformBufferName];

				for(u32 setIndex = 0; setIndex < gpuParametersForPass.Size(); setIndex++)
				{
					SPtr<GpuParametersType>& gpuParameters = gpuParametersForPass[setIndex];
					if(gpuParameters && gpuParameters->HasUniformBuffer(uniformBufferName))
						gpuParameters->SetUniformBuffer(uniformBufferName, uniformBuffer);
				}
			}

			// Create non-shareable ones (these are buffers defined by default by the RHI usually)
			SPtr<GpuProgramParameterDescription> desc = parameterDescriptionsForPass[j];
			if(desc == nullptr)
				continue;

			for(auto iterBlockDesc = desc->UniformBuffers.begin(); iterBlockDesc != desc->UniformBuffers.end(); ++iterBlockDesc)
			{
				const GpuUniformBufferInformation& uniformBufferInformation = iterBlockDesc->second;

				u32 globalBlockIdx = ~0u;
				if(!uniformBufferInformation.IsShareable)
				{
					UniformBufferPointerType newUniformBuffer = CreateGpuBuffer<UniformBufferType>(GpuBufferCreateInformation::CreateUniform(uniformBufferInformation.Size * sizeof(u32)));

					globalBlockIdx = (u32)mUniformBuffers.size();

					// Set non-shareable buffer on the GpuParameterSet for the correct set
					u32 bufferSet = uniformBufferInformation.Set;
					if(bufferSet < gpuParametersForPass.Size() && gpuParametersForPass[bufferSet])
						gpuParametersForPass[bufferSet]->SetUniformBuffer(iterBlockDesc->first, newUniformBuffer);
					mUniformBuffers.emplace_back(iterBlockDesc->first, iterBlockDesc->second.Set, iterBlockDesc->second.Slot, newUniformBuffer, 0, false);
				}
				else
				{
					auto iterFind = std::find_if(uniformBufferData.begin(), uniformBufferData.end(), [&](const auto& x)
												 { return x.Name == iterBlockDesc->first; });

					if(iterFind != uniformBufferData.end())
						globalBlockIdx = iterFind->SequentialIdx;
				}

				// If this uniform buffer is valid, create data/struct mappings for it
				if(globalBlockIdx == ~0u)
					continue;

				for(auto& dataParam : desc->UniformBufferMembers)
				{
					if(dataParam.second.ParentUniformBufferSet != uniformBufferInformation.Set || dataParam.second.ParentUniformBufferSlot != uniformBufferInformation.Slot)
						continue;

					ValidParamKey key(dataParam.first, MaterialParameters::ParamType::Data);

					auto iterFind = validParams.find(key);
					if(iterFind == validParams.end())
						continue;

					u32 paramIdx = materialParameters->GetParamIndex(iterFind->second);

					// Parameter shouldn't be in the valid parameter list if it cannot be found
					B3D_ASSERT(paramIdx != (u32)-1);

					mDataParamInfos.push_back(DataParamInfo());
					DataParamInfo& paramInfo = mDataParamInfos.back();
					paramInfo.ParameterIndex = paramIdx;
					paramInfo.UniformBufferIndex = globalBlockIdx;
					paramInfo.Offset = dataParam.second.CpuOffset;
					paramInfo.ArrayStride = dataParam.second.ArrayElementStride;
				}
			}
		}
	}

	// Add buffers defined in shader but not actually used by GPU programs (so we can check if user is providing a
	// valid buffer name)
	auto& allUniformBuffers = shader->GetUniformBuffers();
	for(auto& entry : allUniformBuffers)
	{
		auto iterFind = std::find_if(mUniformBuffers.begin(), mUniformBuffers.end(), [&](auto& x)
									 { return x.Name == entry.first; });

		if(iterFind == mUniformBuffers.end())
		{
			mUniformBuffers.push_back(UniformBufferInfo(entry.first, 0, 0, nullptr, 0, true));
			mUniformBuffers.back().IsUsed = false;
		}
	}

	// Generate information about object parameters
	B3DMarkAllocatorFrame();
	{
		FrameVector<ObjectParamInfo> objParamInfos;

		u32 offsetsSize = passCount * kNumStages * 4 * sizeof(u32);
		u32* offsets = (u32*)B3DFrameAllocate(offsetsSize);
		memset(offsets, 0, offsetsSize);

		// First store all objects in temporary arrays since we don't know how many of them are
		u32 totalNumObjects = 0;
		u32* stageOffsets = offsets;
		for(u32 i = 0; i < passCount; i++)
		{
			const Array<SPtr<GpuProgramParameterDescription>, GPT_COUNT>& parameterDescriptionsForPass = parameterDescriptionsPerPass[i];
			for(u32 j = 0; j < kNumStages; j++)
			{
				auto fnProcessObjectParams = [&](const Map<String, GpuObjectParameterInformation>& gpuParams,
											   u32 stageIndex, MaterialParameters::ParamType paramType)
				{
					for(auto& param : gpuParams)
					{
						ValidParamKey key(param.first, paramType);

						auto iterFind = validParams.find(key);
						if(iterFind == validParams.end())
							continue;

						u32 paramIndex;
						auto result = materialParameters->GetParamIndex(iterFind->second, paramType, GPDT_UNKNOWN, 0, paramIndex);

						// Parameter shouldn't be in the valid parameter list if it cannot be found
						B3D_ASSERT(result == MaterialParameters::GetParamResult::Success);

						objParamInfos.push_back(ObjectParamInfo());
						ObjectParamInfo& paramInfo = objParamInfos.back();
						paramInfo.ParameterIndex = paramIndex;
						paramInfo.SlotIndex = param.second.Slot;
						paramInfo.SetIndex = param.second.Set;

						stageOffsets[stageIndex]++;
						totalNumObjects++;
					}
				};

				SPtr<GpuProgramParameterDescription> desc = parameterDescriptionsForPass[j];
				if(desc == nullptr)
				{
					stageOffsets += 4;
					continue;
				}

				fnProcessObjectParams(desc->SampledTextures, 0, MaterialParameters::ParamType::Texture);
				fnProcessObjectParams(desc->StorageTextures, 1, MaterialParameters::ParamType::Texture);
				fnProcessObjectParams(desc->Buffers, 2, MaterialParameters::ParamType::Buffer);
				fnProcessObjectParams(desc->Samplers, 3, MaterialParameters::ParamType::Sampler);

				stageOffsets += 4;
			}
		}

		// Transfer all objects into their permanent storage
		u32 bufferCount = (u32)mUniformBuffers.size();
		u32 uniformBufferBindingsSize = bufferCount * passCount * sizeof(PassUniformBufferBindings);
		u32 objectParamInfosSize = totalNumObjects * sizeof(ObjectParamInfo) + passCount * sizeof(PassParamInfo);
		mData = (u8*)B3DAllocate(objectParamInfosSize + uniformBufferBindingsSize);
		u8* dataIter = mData;

		mPassParamInfos = (PassParamInfo*)dataIter;
		memset(mPassParamInfos, 0, objectParamInfosSize);
		dataIter += objectParamInfosSize;

		StageParamInfo* stageInfos = (StageParamInfo*)mPassParamInfos;

		ObjectParamInfo* objInfos = (ObjectParamInfo*)(mPassParamInfos + passCount);
		memcpy(objInfos, objParamInfos.data(), totalNumObjects * sizeof(ObjectParamInfo));

		u32 objInfoOffset = 0;

		stageOffsets = offsets;
		for(u32 i = 0; i < passCount; i++)
		{
			for(u32 j = 0; j < kNumStages; j++)
			{
				StageParamInfo& stage = stageInfos[i * kNumStages + j];

				if(stageOffsets[0] > 0)
				{
					u32 numEntries = stageOffsets[0];

					stage.SampledTextures = objInfos + objInfoOffset;
					stage.SampledTextureCount = numEntries;

					objInfoOffset += numEntries;
				}

				if(stageOffsets[1] > 0)
				{
					u32 numEntries = stageOffsets[1];

					stage.StorageTextures = objInfos + objInfoOffset;
					stage.StorageTextureCount = numEntries;

					objInfoOffset += numEntries;
				}

				if(stageOffsets[2] > 0)
				{
					u32 numEntries = stageOffsets[2];

					stage.Buffers = objInfos + objInfoOffset;
					stage.BufferCount = numEntries;

					objInfoOffset += numEntries;
				}

				if(stageOffsets[3] > 0)
				{
					u32 numEntries = stageOffsets[3];

					stage.SamplerStates = objInfos + objInfoOffset;
					stage.SamplerStateCount = numEntries;

					objInfoOffset += numEntries;
				}

				stageOffsets += 4;
			}
		}

		// Determine on which passes & stages are buffers used on
		for(auto& uniformBuffer : mUniformBuffers)
		{
			uniformBuffer.PassData = (PassUniformBufferBindings*)dataIter;
			dataIter += sizeof(PassUniformBufferBindings) * passCount;
		}

		for(auto& uniformBuffer : mUniformBuffers)
		{
			for(u32 i = 0; i < passCount; i++)
			{
				const Array<SPtr<GpuProgramParameterDescription>, GPT_COUNT>& parameterDescriptionsForPass = parameterDescriptionsPerPass[i];
				for(u32 j = 0; j < kNumStages; j++)
				{
					SPtr<GpuProgramParameterDescription> curDesc = parameterDescriptionsForPass[j];
					if(curDesc == nullptr)
					{
						uniformBuffer.PassData[i].Bindings[j].Set = -1;
						uniformBuffer.PassData[i].Bindings[j].Slot = -1;

						continue;
					}

					auto iterFind = curDesc->UniformBuffers.find(uniformBuffer.Name);
					if(iterFind == curDesc->UniformBuffers.end())
					{
						uniformBuffer.PassData[i].Bindings[j].Set = -1;
						uniformBuffer.PassData[i].Bindings[j].Slot = -1;

						continue;
					}

					uniformBuffer.PassData[i].Bindings[j].Set = iterFind->second.Set;
					uniformBuffer.PassData[i].Bindings[j].Slot = iterFind->second.Slot;
				}
			}
		}

		B3DFrameFree(offsets);
	}
	B3DClearAllocatorFrame();
}

template <bool IsRenderProxy>
TMaterialParameterAdapter<IsRenderProxy>::~TMaterialParameterAdapter()
{
	// All allocations share the same memory, so we just clear it all at once
	B3DFree(mData);
}

template <bool IsRenderProxy>
SPtr<typename TMaterialParameterAdapter<IsRenderProxy>::GpuParametersType> TMaterialParameterAdapter<IsRenderProxy>::GetGpuParameterSet(u32 passIndex, u32 setIndex)
{
	if(passIndex >= mGpuParametersPerPass.size())
		return nullptr;

	if(setIndex >= mGpuParametersPerPass[passIndex].Size())
		return nullptr;

	return mGpuParametersPerPass[passIndex][setIndex];
}

template <bool IsRenderProxy>
u32 TMaterialParameterAdapter<IsRenderProxy>::GetUniformBufferIndex(const String& name) const
{
	for(u32 i = 0; i < (u32)mUniformBuffers.size(); i++)
	{
		const UniformBufferInfo& uniformBuffer = mUniformBuffers[i];
		if(uniformBuffer.Name == name)
			return i;
	}

	return -1;
}

template <bool IsRenderProxy>
void TMaterialParameterAdapter<IsRenderProxy>::SetUniformBuffer(u32 index, const UniformBufferPointerType& buffer, bool ignoreInUpdate)
{
	UniformBufferInfo& uniformBufferInfo = mUniformBuffers[index];
	if(!uniformBufferInfo.Shareable)
	{
		B3D_LOG(Error, LogRenderBackend, "Cannot set uniform buffer with the name \"{0}\". Buffer is not assignable. ", uniformBufferInfo.Name);
		return;
	}

	if(!uniformBufferInfo.IsUsed)
		return;

	uniformBufferInfo.AllowUpdate = !ignoreInUpdate;

	if(uniformBufferInfo.Buffer != buffer)
	{
		uniformBufferInfo.Buffer = buffer;

		const u32 passCount = (u32)mGpuParametersPerPass.size();
		for(u32 passIndex = 0; passIndex < passCount; passIndex++)
		{
			TInlineArray<SPtr<GpuParametersType>, 4>& gpuParametersForPass = mGpuParametersPerPass[passIndex];
			for(u32 stageIndex = 0; stageIndex < kNumStages; stageIndex++)
			{
				GpuProgramType gpuProgramType = (GpuProgramType)stageIndex;

				const UniformBufferBinding& binding = uniformBufferInfo.PassData[passIndex].Bindings[gpuProgramType];

				if(binding.Slot != ~0u && binding.Set < gpuParametersForPass.Size())
				{
					SPtr<GpuParametersType>& gpuParameters = gpuParametersForPass[binding.Set];
					if(gpuParameters)
						gpuParameters->SetUniformBuffer(binding.Slot, buffer);
				}
			}
		}
	}
}

template <bool IsRenderProxy>
void TMaterialParameterAdapter<IsRenderProxy>::SetUniformBuffer(const String& name, const UniformBufferPointerType& buffer, bool ignoreInUpdate)
{
	const u32 bufferIndex = GetUniformBufferIndex(name);
	if(bufferIndex == (u32)-1)
	{
		B3D_LOG(Error, LogRenderBackend, "Cannot set uniform buffer with the name \"{0}\". Buffer name not found. ", name);
		return;
	}

	SetUniformBuffer(bufferIndex, buffer, ignoreInUpdate);
}

template <bool IsRenderProxy>
u32 TMaterialParameterAdapter<IsRenderProxy>::GetSetCount(u32 passIndex)
{
	if(passIndex >= (u32)mGpuParametersPerPass.size())
		return 0;

	return (u32)mGpuParametersPerPass[passIndex].size();
}

template <bool IsRenderProxy>
void TMaterialParameterAdapter<IsRenderProxy>::Update(const MaterialType& material, float t, bool updateAll)
{
	// Note: Instead of iterating over every single parameter, it might be more efficient for @p params to keep
	// a ring buffer and a version number. Then we could just iterate over the ring buffer and only access dirty
	// parameters. If the version number is too high (larger than ring buffer can store), then we force update for all.

	const SPtr<GpuDevice>& device = GetApplication().GetPrimaryGpuDevice();
	const GpuBackendConventions& gpuBackendConventions = device->GetCapabilities().Conventions;
	const SPtr<MaterialParametersType>& materialParameters = material->GetMaterialParameters();

	// Parameters are sorted by buffer, so we map when buffer changes.
	TGpuBufferMappedScope<IsRenderProxy> mappedScope;
	SPtr<render::GpuBuffer> stagingBuffer;  // Only used for render proxy when buffer doesn't support direct mapping
	const UniformBufferInfo* currentUniformBufferInfo = nullptr;
	void* bufferMemory = nullptr;
	u32 curentBlockIndex = ~0u;

	// Helper to finalize previous buffer (flush staging + queue copy if needed)
	auto fnFinalizePreviousBuffer = [&stagingBuffer, &currentUniformBufferInfo, &mappedScope]()
	{
		if constexpr(IsRenderProxy)
		{
			if(stagingBuffer && currentUniformBufferInfo)
			{
				mappedScope.Unmap();

				GpuDevice& gpuDevice = currentUniformBufferInfo->Buffer->GetDevice();
				const SPtr<render::GpuCommandBuffer>& commandBuffer = gpuDevice.GetOrCreateTransferCommandBuffer();
				commandBuffer->CopyBufferToBuffer(stagingBuffer, currentUniformBufferInfo->Buffer, 0, currentUniformBufferInfo->SuballocationByteOffset, currentUniformBufferInfo->Buffer->GetSuballocationSize());
				stagingBuffer = nullptr;
			}
		}
	};

	// Pre-processing: Determine which buffers need full update due to staging.
	// When using staging buffers, only dirty parameters are written, but the entire staging buffer
	// is copied to the uniform buffer. This would overwrite valid data with uninitialized garbage.
	// Solution: If a buffer uses staging and has any dirty parameter, write ALL parameters in that buffer.
	TInlineArray<bool, 8> bufferNeedsFullUpdate;
	if constexpr(IsRenderProxy)
	{
		bufferNeedsFullUpdate.Resize((u32)mUniformBuffers.size(), false);

			for(const auto& paramInfo : mDataParamInfos)
		{
			const UniformBufferInfo& uniformBufferInfo = mUniformBuffers[paramInfo.UniformBufferIndex];
			if(uniformBufferInfo.Buffer == nullptr || !uniformBufferInfo.AllowUpdate)
				continue;

			// Check if buffer uses staging (non-mapped memory means staging is required)
			if(uniformBufferInfo.Buffer->GetMappedMemory() != nullptr)
				continue;

			const MaterialParameters::ParamData* materialParamInfo = materialParameters->GetParamData(paramInfo.ParameterIndex);
			const u32 arraySize = materialParamInfo->ArraySize == 0 ? 1 : materialParamInfo->ArraySize;

			bool isAnimated = false;
			for(u32 i = 0; i < arraySize; i++)
			{
				isAnimated = materialParameters->IsAnimated(*materialParamInfo, i);
				if(isAnimated)
					break;
			}

			// If this parameter is dirty, mark the buffer for full update
			if(materialParamInfo->Version > mParamVersion || updateAll || isAnimated)
				bufferNeedsFullUpdate[paramInfo.UniformBufferIndex] = true;
		}
	}

	// Update data params
	for(auto& paramInfo : mDataParamInfos)
	{
		const UniformBufferInfo& uniformBufferInfo = mUniformBuffers[paramInfo.UniformBufferIndex];
		if(uniformBufferInfo.Buffer == nullptr || !uniformBufferInfo.AllowUpdate)
			continue;

		const MaterialParameters::ParamData* materialParamInfo = materialParameters->GetParamData(paramInfo.ParameterIndex);
		u32 arraySize = materialParamInfo->ArraySize == 0 ? 1 : materialParamInfo->ArraySize;

		bool isAnimated = false;
		for(u32 i = 0; i < arraySize; i++)
		{
			isAnimated = materialParameters->IsAnimated(*materialParamInfo, i);
			if(isAnimated)
				break;
		}

		// Don't skip if buffer needs full update due to staging
		bool forceUpdate = false;
		if constexpr(IsRenderProxy)
			forceUpdate = bufferNeedsFullUpdate[paramInfo.UniformBufferIndex];

		if(!forceUpdate && materialParamInfo->Version <= mParamVersion && !updateAll && !isAnimated)
			continue;

		// Map new buffer when block changes
		if(paramInfo.UniformBufferIndex != curentBlockIndex)
		{
			fnFinalizePreviousBuffer();

			// Check if buffer supports direct CPU mapping (render proxy only)
			if constexpr(IsRenderProxy)
			{
				bool useStaging = false;
				useStaging = (uniformBufferInfo.Buffer->GetMappedMemory() == nullptr);

				if(useStaging)
				{
					stagingBuffer = render::GpuBufferUtility::CreateStaging(uniformBufferInfo.Buffer, false);
					mappedScope = stagingBuffer->Map(GpuMapOption::Write);
				}
				else
					mappedScope = uniformBufferInfo.Buffer->Map(GpuMapOption::Write);
			}
			else
				mappedScope = uniformBufferInfo.Buffer->Map(GpuMapOption::Write);

			bufferMemory = mappedScope.GetMappedMemory();
			currentUniformBufferInfo = &uniformBufferInfo;
			curentBlockIndex = paramInfo.UniformBufferIndex;
		}

		if(materialParamInfo->DataType != GPDT_STRUCT)
		{
			const GpuDataParameterTypeInformation& typeInfo = GpuParameterSet::kParamSizes.Lookup[(int)materialParamInfo->DataType];

			u32 paramSize;
			if(materialParamInfo->DataType != GPDT_COLOR)
				paramSize = typeInfo.NumColumns * typeInfo.NumRows * typeInfo.BaseTypeSize;
			else
				paramSize = paramInfo.ArrayStride * typeInfo.BaseTypeSize;

			u8* data = materialParameters->GetData(materialParamInfo->Index);
			if(!isAnimated)
			{
				const bool transposeMatrices = gpuBackendConventions.MatrixOrder == GpuBackendConventions::MatrixOrder::ColumnMajor;
				if(transposeMatrices)
				{
					auto fnWriteTransposed = [&paramInfo, &paramSize, &arraySize, &uniformBufferInfo, bufferMemory, data](auto& temp)
					{
						for(u32 i = 0; i < arraySize; i++)
						{
							u32 readOffset = i * paramSize;
							memcpy(&temp, data + readOffset, paramSize);
							auto transposed = temp.Transpose();

							u32 writeOffset = uniformBufferInfo.SuballocationByteOffset + (paramInfo.Offset + paramInfo.ArrayStride * i) * sizeof(u32);
							memcpy(static_cast<u8*>(bufferMemory) + writeOffset, &transposed, paramSize);
						}
					};

					switch(materialParamInfo->DataType)
					{
					case GPDT_MATRIX_2X2:
						{
							MatrixNxM<2, 2> matrix;
							fnWriteTransposed(matrix);
						}
						break;
					case GPDT_MATRIX_2X3:
						{
							MatrixNxM<2, 3> matrix;
							fnWriteTransposed(matrix);
						}
						break;
					case GPDT_MATRIX_2X4:
						{
							MatrixNxM<2, 4> matrix;
							fnWriteTransposed(matrix);
						}
						break;
					case GPDT_MATRIX_3X2:
						{
							MatrixNxM<3, 2> matrix;
							fnWriteTransposed(matrix);
						}
						break;
					case GPDT_MATRIX_3X3:
						{
							Matrix3 matrix;
							fnWriteTransposed(matrix);
						}
						break;
					case GPDT_MATRIX_3X4:
						{
							MatrixNxM<3, 4> matrix;
							fnWriteTransposed(matrix);
						}
						break;
					case GPDT_MATRIX_4X2:
						{
							MatrixNxM<4, 2> matrix;
							fnWriteTransposed(matrix);
						}
						break;
					case GPDT_MATRIX_4X3:
						{
							MatrixNxM<4, 3> matrix;
							fnWriteTransposed(matrix);
						}
						break;
					case GPDT_MATRIX_4X4:
						{
							Matrix4 matrix;
							fnWriteTransposed(matrix);
						}
						break;
					default:
						{
							for(u32 i = 0; i < arraySize; i++)
							{
								u32 arrayOffset = i * paramSize;
								u32 writeOffset = uniformBufferInfo.SuballocationByteOffset + (paramInfo.Offset + paramInfo.ArrayStride * i) * sizeof(u32);
								memcpy(static_cast<u8*>(bufferMemory) + writeOffset, data + arrayOffset, paramSize);
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
						u32 writeOffset = uniformBufferInfo.SuballocationByteOffset + (paramInfo.Offset + paramInfo.ArrayStride * i) * sizeof(u32);
						memcpy(static_cast<u8*>(bufferMemory) + writeOffset, data + readOffset, paramSize);
					}
				}
			}
			else // Animated
			{
				if(materialParamInfo->DataType == GPDT_FLOAT1)
				{
					B3D_ASSERT(paramSize == sizeof(float));

					for(u32 i = 0; i < arraySize; i++)
					{
						u32 readOffset = i * paramSize;
						u32 writeOffset = uniformBufferInfo.SuballocationByteOffset + (paramInfo.Offset + paramInfo.ArrayStride * i) * sizeof(u32);

						float value;
						if(materialParameters->IsAnimated(*materialParamInfo, i))
						{
							const TAnimationCurve<float>& curve = materialParameters->template GetCurveParam<float>(*materialParamInfo, i);

							value = curve.Evaluate(t, true);
						}
						else
							memcpy(&value, data + readOffset, paramSize);

						memcpy(static_cast<u8*>(bufferMemory) + writeOffset, &value, paramSize);
					}
				}
				else if(materialParamInfo->DataType == GPDT_FLOAT4)
				{
					B3D_ASSERT(paramSize == sizeof(Area2));

					CoreVariantHandleType<SpriteImage, IsRenderProxy> spriteImage =
						materialParameters->GetOwningSpriteImage(*materialParamInfo);

					u32 writeOffset = uniformBufferInfo.SuballocationByteOffset + paramInfo.Offset * sizeof(u32);
					Area2 uv = Area2(0.0f, 0.0f, 1.0f, 1.0f);
					if(spriteImage != nullptr)
						uv = spriteImage->EvaluateAnimation(spriteImage->GetDefaultAllocatedImage(), t);

					memcpy(static_cast<u8*>(bufferMemory) + writeOffset, &uv, paramSize);

					// Only the first array element receives sprite UVs, the rest are treated as normal
					for(u32 i = 1; i < arraySize; i++)
					{
						u32 readOffset = i * paramSize;
						writeOffset = uniformBufferInfo.SuballocationByteOffset + (paramInfo.Offset + paramInfo.ArrayStride * i) * sizeof(u32);

						memcpy(static_cast<u8*>(bufferMemory) + writeOffset, data + readOffset, paramSize);
					}
				}
				else if(materialParamInfo->DataType == GPDT_COLOR)
				{
					for(u32 i = 0; i < arraySize; i++)
					{
						B3D_ASSERT(paramSize == sizeof(Color));

						u32 readOffset = i * paramSize;
						u32 writeOffset = uniformBufferInfo.SuballocationByteOffset + (paramInfo.Offset + paramInfo.ArrayStride * i) * sizeof(u32);

						Color value;
						if(materialParameters->IsAnimated(*materialParamInfo, i))
						{
							const ColorGradientHDR& gradient = materialParameters->GetColorGradientParam(*materialParamInfo, i);

							const float wrappedT = Math::Repeat(t, gradient.GetDuration());
							value = gradient.Evaluate(wrappedT);
						}
						else
							memcpy(&value, data + readOffset, paramSize);

						memcpy(static_cast<u8*>(bufferMemory) + writeOffset, &value, paramSize);
					}
				}
			}
		}
		else
		{
			u32 paramSize = materialParameters->GetStructSize(*materialParamInfo);
			void* paramData = B3DStackAllocate(paramSize);
			for(u32 i = 0; i < arraySize; i++)
			{
				materialParameters->GetStructData(*materialParamInfo, paramData, paramSize, i);

				u32 writeOffset = uniformBufferInfo.SuballocationByteOffset + (paramInfo.Offset + paramInfo.ArrayStride * i) * sizeof(u32);
				memcpy(static_cast<u8*>(bufferMemory) + writeOffset, paramData, paramSize);
			}
			B3DStackFree(paramData);
		}
	}

	// Finalize last buffer (flush staging + queue copy if needed)
	fnFinalizePreviousBuffer();

	// Update object params
	const auto passCount = (u32)mGpuParametersPerPass.size();

	for(u32 passIndex = 0; passIndex < passCount; passIndex++)
	{
		TInlineArray<SPtr<GpuParametersType>, 4>& gpuParametersForPass = mGpuParametersPerPass[passIndex];

		for(u32 stageIndex = 0; stageIndex < kNumStages; stageIndex++)
		{
			const StageParamInfo& stageInfo = mPassParamInfos[passIndex].Stages[stageIndex];

			for(u32 k = 0; k < stageInfo.SampledTextureCount; k++)
			{
				const ObjectParamInfo& paramInfo = stageInfo.SampledTextures[k];

				const MaterialParameters::ParamData* materialParamInfo = materialParameters->GetParamData(paramInfo.ParameterIndex);
				if(materialParamInfo->Version <= mParamVersion && !updateAll)
					continue;

				TextureSurface surface;
				TextureType texture;
				materialParameters->GetTexture(*materialParamInfo, texture, surface);

				if(paramInfo.SetIndex < gpuParametersForPass.Size())
				{
					SPtr<GpuParametersType>& gpuParameters = gpuParametersForPass[paramInfo.SetIndex];
					if(gpuParameters)
						gpuParameters->SetSampledTexture(paramInfo.SlotIndex, texture, surface, 0);
				}
			}

			for(u32 k = 0; k < stageInfo.StorageTextureCount; k++)
			{
				const ObjectParamInfo& paramInfo = stageInfo.StorageTextures[k];

				const MaterialParameters::ParamData* materialParamInfo = materialParameters->GetParamData(paramInfo.ParameterIndex);
				if(materialParamInfo->Version <= mParamVersion && !updateAll)
					continue;

				TextureSurface surface;
				TextureType texture;
				materialParameters->GetStorageTexture(*materialParamInfo, texture, surface);

				if(paramInfo.SetIndex < gpuParametersForPass.Size())
				{
					SPtr<GpuParametersType>& gpuParameters = gpuParametersForPass[paramInfo.SetIndex];
					if(gpuParameters)
						gpuParameters->SetStorageTexture(paramInfo.SlotIndex, texture, surface, 0);
				}
			}

			for(u32 k = 0; k < stageInfo.BufferCount; k++)
			{
				const ObjectParamInfo& paramInfo = stageInfo.Buffers[k];

				const MaterialParameters::ParamData* materialParamInfo = materialParameters->GetParamData(paramInfo.ParameterIndex);
				if(materialParamInfo->Version <= mParamVersion && !updateAll)
					continue;

				BufferType buffer;
				materialParameters->GetBuffer(*materialParamInfo, buffer);

				if(paramInfo.SetIndex < gpuParametersForPass.Size())
				{
					SPtr<GpuParametersType>& gpuParameters = gpuParametersForPass[paramInfo.SetIndex];
					if(gpuParameters)
						gpuParameters->SetStorageBuffer(paramInfo.SlotIndex, buffer, 0);
				}
			}

			for(u32 k = 0; k < stageInfo.SamplerStateCount; k++)
			{
				const ObjectParamInfo& paramInfo = stageInfo.SamplerStates[k];

				const MaterialParameters::ParamData* materialParamInfo = materialParameters->GetParamData(paramInfo.ParameterIndex);
				if(materialParamInfo->Version <= mParamVersion && !updateAll)
					continue;

				SPtr<SamplerState> samplerState;
				materialParameters->GetSamplerState(*materialParamInfo, samplerState);

				if(paramInfo.SetIndex < gpuParametersForPass.Size())
				{
					SPtr<GpuParametersType>& gpuParameters = gpuParametersForPass[paramInfo.SetIndex];
					if(gpuParameters)
						gpuParameters->SetSamplerState(paramInfo.SlotIndex, samplerState, 0);
				}
			}
		}

		// Mark all GpuParameterSet objects in this pass as dirty
		for(u32 setIndex = 0; setIndex < gpuParametersForPass.Size(); setIndex++)
		{
			SPtr<GpuParametersType>& gpuParameters = gpuParametersForPass[setIndex];
			if(gpuParameters)
				gpuParameters->MarkRenderProxyDataDirtyInternal();
		}
	}

	mParamVersion = materialParameters->GetParamVersion();
}

template class TMaterialParameterAdapter<false>;
template class TMaterialParameterAdapter<true>;

namespace b3d::render
{
	void MaterialParameterAdapter::SetUniformBuffer(u32 index, const GpuBufferSuballocation& suballocation, bool ignoreInUpdate)
	{
		UniformBufferInfo& uniformBufferInfo = mUniformBuffers[index];
		if(!uniformBufferInfo.Shareable)
		{
			B3D_LOG(Error, LogRenderBackend, "Cannot set uniform buffer with the name \"{0}\". Buffer is not assignable. ", uniformBufferInfo.Name);
			return;
		}

		if(!uniformBufferInfo.IsUsed)
			return;

		uniformBufferInfo.AllowUpdate = !ignoreInUpdate;

		if(uniformBufferInfo.Buffer != suballocation.GetBuffer() || uniformBufferInfo.SuballocationByteOffset != suballocation.GetSuballocationOffset())
		{
			uniformBufferInfo.Buffer = suballocation.GetBuffer();
			uniformBufferInfo.SuballocationByteOffset = suballocation.GetSuballocationOffset();;

			const u32 passCount = (u32)mGpuParametersPerPass.size();
			for(u32 passIndex = 0; passIndex < passCount; passIndex++)
			{
				TInlineArray<SPtr<GpuParameterSet>, 4>& gpuParametersForPass = mGpuParametersPerPass[passIndex];
				for(u32 stageIndex = 0; stageIndex < kNumStages; stageIndex++)
				{
					GpuProgramType gpuProgramType = (GpuProgramType)stageIndex;

					const UniformBufferBinding& binding = uniformBufferInfo.PassData[passIndex].Bindings[gpuProgramType];

					if(binding.Slot != ~0u && binding.Set < gpuParametersForPass.Size())
					{
						SPtr<GpuParameterSet>& gpuParameters = gpuParametersForPass[binding.Set];
						if(gpuParameters)
							gpuParameters->SetUniformBuffer(binding.Slot, suballocation);
					}
				}
			}
		}
	}

	void MaterialParameterAdapter::SetUniformBuffer(const String& name, const GpuBufferSuballocation& suballocation, bool ignoreInUpdate)
	{
		const u32 bufferIndex = GetUniformBufferIndex(name);
		if(bufferIndex == (u32)-1)
		{
			B3D_LOG(Error, LogRenderBackend, "Cannot set uniform buffer with the name \"{0}\". Buffer name not found. ", name);
			return;
		}

		SetUniformBuffer(bufferIndex, suballocation, ignoreInUpdate);
	}
}

