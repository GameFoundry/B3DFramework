//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DGpuProgramParameterDescription.h"
#include "RTTI/B3DGpuProgramRTTI.h"
#include "String/B3DStringFormat.h"

using namespace b3d;

constexpr u32 RTTIPlainType<GpuUniformBufferMemberInformation>::kVersion;

Result GpuProgramParameterDescription::TryCombine(const GpuProgramParameterDescription& other, GpuProgramStageBit stage)
{
	// Combine uniform buffers
	for(const auto& entry : other.UniformBuffers)
	{
		const GpuUniformBufferInformation& uniformBuffer = entry.second;

		if(auto found = UniformBuffers.find(uniformBuffer.Name); found != UniformBuffers.end())
		{
			if(found->second.Slot != uniformBuffer.Slot)
				return Result::Fail("Uniform buffer slot mismatch.", ResultStatus::Failed, StringUtility::Format("Uniform buffer '{0}' has slot {1}, but expected {2}.", uniformBuffer.Name, uniformBuffer.Slot, found->second.Slot));

			if(found->second.Size != uniformBuffer.Size)
				return Result::Fail("Uniform buffer size mismatch.", ResultStatus::Failed, StringUtility::Format("Uniform buffer '{0}' has size {1}, but expected {2}.", uniformBuffer.Name, uniformBuffer.Size, found->second.Size));

			found->second.Stages |= stage;
		}
		else
		{
			GpuUniformBufferInformation newEntry = uniformBuffer;
			newEntry.Stages = stage;
			UniformBuffers[uniformBuffer.Name] = std::move(newEntry);
		}
	}

	// Helper lambda to combine object parameters
	auto fnCombineObjectParameters = [stage](Map<String, GpuObjectParameterInformation>& targetMap, const Map<String, GpuObjectParameterInformation>& sourceMap) -> Result
	{
		for(const auto& entry : sourceMap)
		{
			const GpuObjectParameterInformation& objectParameter = entry.second;

			if(auto found = targetMap.find(objectParameter.Name); found != targetMap.end())
			{
				if(found->second.Type != objectParameter.Type)
					return Result::Fail("Object parameter type mismatch.", ResultStatus::Failed, StringUtility::Format("Object parameter '{0}' has type {1}, but expected {2}.", objectParameter.Name, (u32)objectParameter.Type, (u32)found->second.Type));

				if(found->second.Slot != objectParameter.Slot)
					return Result::Fail("Object parameter slot mismatch.", ResultStatus::Failed, StringUtility::Format("Object parameter '{0}' has slot {1}, but expected {2}.", objectParameter.Name, objectParameter.Slot, found->second.Slot));

				if(found->second.ArraySize != objectParameter.ArraySize)
					return Result::Fail("Object parameter array size mismatch.", ResultStatus::Failed, StringUtility::Format("Object parameter '{0}' has array size {1}, but expected {2}.", objectParameter.Name, objectParameter.ArraySize, found->second.ArraySize));

				found->second.Stages |= stage;
			}
			else
			{
				GpuObjectParameterInformation newEntry = objectParameter;
				newEntry.Stages = stage;
				targetMap[objectParameter.Name] = std::move(newEntry);
			}
		}

		return Result::Success();
	};

	// Combine sampled textures
	if(Result result = fnCombineObjectParameters(SampledTextures, other.SampledTextures); !result.IsSuccessful())
		return result;

	// Combine storage textures
	if(Result result = fnCombineObjectParameters(StorageTextures, other.StorageTextures); !result.IsSuccessful())
		return result;

	// Combine buffers
	if(Result result = fnCombineObjectParameters(Buffers, other.Buffers); !result.IsSuccessful())
		return result;

	// Combine samplers
	if(Result result = fnCombineObjectParameters(Samplers, other.Samplers); !result.IsSuccessful())
		return result;

	// Combine uniform buffer members
	for(const auto& entry : other.UniformBufferMembers)
	{
		const GpuUniformBufferMemberInformation& uniformBufferMember = entry.second;

		if(auto found = UniformBufferMembers.find(uniformBufferMember.Name); found != UniformBufferMembers.end())
		{
			if(found->second != uniformBufferMember)
				return Result::Fail("Uniform buffer member mismatch.", ResultStatus::Failed, StringUtility::Format("Uniform buffer member '{0}' has different type information.", uniformBufferMember.Name));
		}
		else
			UniformBufferMembers[uniformBufferMember.Name] = uniformBufferMember;
	}

	return Result::Success();
}

void GpuProgramParameterDescription::SplitBySet(TInlineArray<GpuProgramParameterDescription, 4>& output) const
{
	auto fnSplitMap = [&output](const auto& sourceMap, auto targetMapSelector)
	{
		for(const auto& entry : sourceMap)
		{
			while(output.size() <= entry.second.Set)
				output.Add(GpuProgramParameterDescription());

			targetMapSelector(output[entry.second.Set])[entry.first] = entry.second;
		}
	};

	fnSplitMap(UniformBuffers, [](GpuProgramParameterDescription& desc) -> auto& { return desc.UniformBuffers; });
	fnSplitMap(SampledTextures, [](GpuProgramParameterDescription& desc) -> auto& { return desc.SampledTextures; });
	fnSplitMap(StorageTextures, [](GpuProgramParameterDescription& desc) -> auto& { return desc.StorageTextures; });
	fnSplitMap(Buffers, [](GpuProgramParameterDescription& desc) -> auto& { return desc.Buffers; });
	fnSplitMap(Samplers, [](GpuProgramParameterDescription& desc) -> auto& { return desc.Samplers; });

	for(const auto& entry : UniformBufferMembers)
	{
		while(output.size() <= entry.second.ParentUniformBufferSet)
			output.Add(GpuProgramParameterDescription());

		output[entry.second.ParentUniformBufferSet].UniformBufferMembers[entry.first] = entry.second;
	}
}

RTTIType* GpuProgramParameterDescription::GetRttiStatic()
{
	return GpuProgramParameterDescriptionRTTI::Instance();
}

RTTIType* GpuProgramParameterDescription::GetRtti() const
{
	return GpuProgramParameterDescription::GetRttiStatic();
}
