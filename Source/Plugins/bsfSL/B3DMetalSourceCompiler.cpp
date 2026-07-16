//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DMetalSourceCompiler.h"

#if B3D_PLATFORM_MACOS

#include "B3DGLSLToSPIRV.h"
#include "B3DMetalShaderABI.h"
#include "GpuBackend/B3DGpuProgramParameterDescription.h"

#include "spirv_cross/spirv_msl.hpp"

#include <algorithm>
#include <limits>

using namespace b3d;
using namespace b3d::render;

namespace
{
	/** Converts VKSL to SPIR-V and retains the reflection required to configure SPIRV-Cross's MSL ABI. */
	TShared<GpuProgramBytecode> CompileIntermediateSpirv(const String& source, GpuProgramType type)
	{
		static GLSLToSPIRV converter("MetalSource", 1);

		GpuProgramCreateInformation createInformation;
		createInformation.Source = source;
		createInformation.EntryPoint = "main";
		createInformation.Language = "vksl";
		createInformation.Type = type;

		return converter.CompileBytecode(createInformation);
	}

	spv::ExecutionModel MapExecutionModel(GpuProgramType type)
	{
		switch(type)
		{
		case GPT_VERTEX_PROGRAM: return spv::ExecutionModelVertex;
		case GPT_FRAGMENT_PROGRAM: return spv::ExecutionModelFragment;
		case GPT_COMPUTE_PROGRAM: return spv::ExecutionModelGLCompute;
		case GPT_GEOMETRY_PROGRAM: return spv::ExecutionModelGeometry;
		case GPT_DOMAIN_PROGRAM: return spv::ExecutionModelTessellationEvaluation;
		case GPT_HULL_PROGRAM: return spv::ExecutionModelTessellationControl;
		default:
			B3D_ASSERT(false);
			return spv::ExecutionModelVertex;
		}
	}

	const char* MapEntryPointName(GpuProgramType type)
	{
		switch(type)
		{
		case GPT_VERTEX_PROGRAM: return "vsmain";
		case GPT_FRAGMENT_PROGRAM: return "fsmain";
		case GPT_COMPUTE_PROGRAM: return "csmain";
		default:
			B3D_ASSERT(false);
			return "";
		}
	}

	void AppendError(ShaderCompilerResult& result, const String& message)
	{
		result.ErrorMessage += message;
		if(result.ErrorMessage.empty() || result.ErrorMessage.back() != '\n')
			result.ErrorMessage += '\n';
	}

	template<class TEntries, class TGetCount>
	void AddResourceBindings(spirv_cross::CompilerMSL& compiler, const TEntries& entries,
		GpuParameterType type, spv::ExecutionModel stage, u64* nextArgumentIndices,
		UnorderedSet<u64>& occupiedBindings, u64& textureArgumentCount, u64& samplerArgumentCount,
		ShaderCompilerResult& result, TGetCount getCount)
	{
		Vector<decltype(&entries.begin()->second)> sortedEntries;
		sortedEntries.reserve(entries.size());
		for(const auto& entry : entries)
			sortedEntries.push_back(&entry.second);

		std::sort(sortedEntries.begin(), sortedEntries.end(), [](const auto* lhs, const auto* rhs)
		{
			if(lhs->Set != rhs->Set)
				return lhs->Set < rhs->Set;
			if(lhs->Slot != rhs->Slot)
				return lhs->Slot < rhs->Slot;
			return lhs->Name < rhs->Name;
		});

		for(const auto* information : sortedEntries)
		{
			const u32 count = getCount(*information);
			if(information->Set > kMetalMaximumParameterSetIndex
				|| information->Slot > kMetalMaximumArgumentBufferSlot
				|| count == 0 || count > kMetalArgumentBufferArrayStride)
			{
				AppendError(result, StringUtility::Format("Metal argument-buffer binding is outside the supported ABI: "
					"set={0}, slot={1}, arraySize={2}.", information->Set, information->Slot, count));
				continue;
			}

			const u64 bindingKey = ((u64)information->Set << 32) | information->Slot;
			if(!occupiedBindings.insert(bindingKey).second)
			{
				AppendError(result, StringUtility::Format("Metal shader declares multiple resources at set {0}, slot {1}. "
					"SPIR-V descriptor coordinates must be unique.", information->Set, information->Slot));
				continue;
			}

			u64& nextArgumentIndex = nextArgumentIndices[information->Set];
			if(nextArgumentIndex + count > std::numeric_limits<u32>::max())
			{
				AppendError(result, StringUtility::Format("Metal argument-buffer index space overflowed for set {0}.",
					information->Set));
				continue;
			}

			spirv_cross::MSLResourceBinding binding;
			binding.stage = stage;
			binding.desc_set = information->Set;
			binding.binding = information->Slot;
			binding.count = count;
			switch(type)
			{
			case GpuParameterType::UniformBuffer:
			case GpuParameterType::StorageBuffer:
				binding.msl_buffer = (u32)nextArgumentIndex;
				break;
			case GpuParameterType::SampledTexture:
			case GpuParameterType::StorageTexture:
				binding.msl_texture = (u32)nextArgumentIndex;
				textureArgumentCount += count;
				break;
			case GpuParameterType::Sampler:
				binding.msl_sampler = (u32)nextArgumentIndex;
				samplerArgumentCount += count;
				break;
			default:
				B3D_ASSERT(false);
				continue;
			}

			nextArgumentIndex += count;
			compiler.add_msl_resource_binding(binding);
		}
	}

	bool RenameResources(spirv_cross::CompilerMSL& compiler, const GpuProgramParameterDescription& parameterDescription, ShaderCompilerResult& result)
	{
		const spirv_cross::ShaderResources resources = compiler.get_shader_resources();
		bool valid = true;
		const auto fnRenameResources = [&compiler, &result, &valid](const auto& reflectedResources,
			const auto& entries, GpuParameterType parameterType)
		{
			UnorderedSet<String> mappedNames;
			for(const spirv_cross::Resource& resource : reflectedResources)
			{
				const String resourceName = !resource.name.empty()
					? String(resource.name.c_str())
					: String(compiler.get_name(resource.id).c_str());
				const auto found = entries.find(resourceName);
				if(found == entries.end())
				{
					AppendError(result, StringUtility::Format(
						"Metal source generation could not map SPIR-V resource '{0}' to an engine binding.", resourceName));
					valid = false;
					continue;
				}

				const auto& information = found->second;
				const String encodedName = EncodeMetalResourceName(parameterType, information.Set,
					information.Slot, information.Name);
				compiler.set_name(resource.id, encodedName.c_str());
				mappedNames.insert(found->first);
			}

			for(const auto& entry : entries)
			{
				if(mappedNames.find(entry.first) == mappedNames.end())
				{
					AppendError(result, StringUtility::Format("Metal source generation did not find declared SPIR-V resource "
						"'{0}' while encoding engine bindings.", entry.first));
					valid = false;
				}
			}
		};

		fnRenameResources(resources.uniform_buffers, parameterDescription.UniformBuffers,
			GpuParameterType::UniformBuffer);
		fnRenameResources(resources.separate_images, parameterDescription.SampledTextures,
			GpuParameterType::SampledTexture);
		fnRenameResources(resources.storage_images, parameterDescription.StorageTextures,
			GpuParameterType::StorageTexture);
		fnRenameResources(resources.storage_buffers, parameterDescription.Buffers,
			GpuParameterType::StorageBuffer);
		fnRenameResources(resources.separate_samplers, parameterDescription.Samplers,
			GpuParameterType::Sampler);

		if(!resources.sampled_images.empty())
		{
			AppendError(result, "Metal source generation received combined image/sampler resources even though "
				"the native Metal target requires separate images and samplers.");
			valid = false;
		}

		return valid;
	}

	ShaderCompilerResult CompileMetalSource(const String& source, GpuProgramType type, String& outSource)
	{
		ShaderCompilerResult result;
		if(type != GPT_VERTEX_PROGRAM && type != GPT_FRAGMENT_PROGRAM && type != GPT_COMPUTE_PROGRAM)
		{
			result.ErrorMessage = "The macOS Metal backend does not support geometry or tessellation shader stages.";
			return result;
		}

		TShared<GpuProgramBytecode> spirv = CompileIntermediateSpirv(source, type);
		if(spirv == nullptr || spirv->Instructions.Data == nullptr || spirv->Instructions.Size == 0)
		{
			result.ErrorMessage = spirv != nullptr && !spirv->Messages.empty()
				? spirv->Messages
				: "Failed to compile the VKSL intermediate to SPIR-V for Metal source generation.";

			return result;
		}

		B3D_ASSERT((spirv->Instructions.Size % sizeof(u32)) == 0);
		spirv_cross::CompilerMSL compiler((u32*)spirv->Instructions.Data, spirv->Instructions.Size / sizeof(u32));
		const spv::ExecutionModel executionModel = MapExecutionModel(type);
		compiler.rename_entry_point("main", MapEntryPointName(type), executionModel);

		spirv_cross::CompilerMSL::Options mslOptions;
		mslOptions.msl_version = spirv_cross::CompilerMSL::Options::make_msl_version(3, 0);
		mslOptions.argument_buffers = true;
		mslOptions.argument_buffers_tier = spirv_cross::CompilerMSL::Options::ArgumentBuffersTier::Tier2;
		mslOptions.platform = spirv_cross::CompilerMSL::Options::macOS;
		compiler.set_msl_options(mslOptions);

		spirv_cross::CompilerGLSL::Options commonOptions;
		commonOptions.separate_shader_objects = true;
		commonOptions.vulkan_semantics = true;
		commonOptions.vertex.flip_vert_y = false;
		compiler.set_common_options(commonOptions);

		if(spirv->ParameterDescription != nullptr)
		{
			const GpuProgramParameterDescription& parameterDescription = *spirv->ParameterDescription;
			for(const auto& entry : parameterDescription.Buffers)
			{
				if(entry.second.Type != GPOT_STRUCTURED_BUFFER && entry.second.Type != GPOT_RWSTRUCTURED_BUFFER)
				{
					AppendError(result, StringUtility::Format("Metal typed/raw storage-buffer binding requires an "
						"MTLTexture buffer view and is not supported yet: {0}.", entry.second.Name));
				}
			}

			u64 nextArgumentIndices[kMetalMaximumParameterSetIndex + 1] = {};
			u64 textureArgumentCount = 0;
			u64 samplerArgumentCount = 0;
			UnorderedSet<u64> occupiedBindings;

			// Calls are intentionally ordered to match the runtime argument-buffer ABI.
			static_assert(kTypeOrderUniformBuffer == 0 && kTypeOrderSampledTexture == 1
				&& kTypeOrderStorageTexture == 2 && kTypeOrderStorageBuffer == 3 && kTypeOrderSampler == 4,
				"Metal argument-buffer resource order changed; update binding construction below.");

			AddResourceBindings(compiler, parameterDescription.UniformBuffers, GpuParameterType::UniformBuffer, executionModel, nextArgumentIndices, occupiedBindings, textureArgumentCount, samplerArgumentCount, result, [](const auto&) { return 1u; });
			AddResourceBindings(compiler, parameterDescription.SampledTextures, GpuParameterType::SampledTexture, executionModel, nextArgumentIndices, occupiedBindings, textureArgumentCount, samplerArgumentCount, result, [](const auto& information) { return information.ArraySize; });
			AddResourceBindings(compiler, parameterDescription.StorageTextures, GpuParameterType::StorageTexture, executionModel, nextArgumentIndices, occupiedBindings, textureArgumentCount, samplerArgumentCount, result, [](const auto& information) { return information.ArraySize; });
			AddResourceBindings(compiler, parameterDescription.Buffers, GpuParameterType::StorageBuffer, executionModel, nextArgumentIndices, occupiedBindings, textureArgumentCount, samplerArgumentCount, result, [](const auto& information) { return information.ArraySize; });
			AddResourceBindings(compiler, parameterDescription.Samplers, GpuParameterType::Sampler, executionModel, nextArgumentIndices, occupiedBindings, textureArgumentCount, samplerArgumentCount, result, [](const auto& information) { return information.ArraySize; });

			if(textureArgumentCount > kMetalMaximumTextureArgumentsPerStage)
				AppendError(result, StringUtility::Format("Metal shader exceeds the Apple7 Tier-2 texture argument limit: {0} > {1}.", textureArgumentCount, kMetalMaximumTextureArgumentsPerStage));

			if(samplerArgumentCount > kMetalMaximumSamplerArgumentsPerStage)
				AppendError(result, StringUtility::Format("Metal shader exceeds the Apple7 Tier-2 sampler argument limit: {0} > {1}.", samplerArgumentCount, kMetalMaximumSamplerArgumentsPerStage));

			if(!result.ErrorMessage.empty())
				return result;
		}

		if(spirv->ParameterDescription != nullptr && !RenameResources(compiler, *spirv->ParameterDescription, result))
			return result;

		outSource = compiler.compile();
		if(compiler.needs_swizzle_buffer() || compiler.needs_buffer_size_buffer()
			|| compiler.needs_view_mask_buffer() || compiler.needs_dispatch_base_buffer()
			|| compiler.needs_output_buffer() || compiler.needs_patch_output_buffer()
			|| compiler.needs_input_threadgroup_mem())
		{
			outSource.clear();
			result.ErrorMessage = "The Metal shader requires a SPIRV-Cross auxiliary buffer that the backend does not "
				"encode. Runtime-sized storage-buffer arrays, swizzle emulation, multiview, and tessellation "
				"auxiliary buffers are not supported.";

			return result;
		}

		if(outSource.empty())
		{
			result.ErrorMessage = "SPIRV-Cross produced empty MSL source.";
			return result;
		}

		return result;
	}
}

ShaderCompilerResult MetalSourceCompiler::Compile(const String& source, GpuProgramType type, String& outSource)
{
	outSource.clear();
	return CompileMetalSource(source, type, outSource);
}

#endif // B3D_PLATFORM_MACOS
