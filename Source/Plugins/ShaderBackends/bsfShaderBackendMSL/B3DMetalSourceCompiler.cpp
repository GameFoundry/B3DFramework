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
	TShared<GpuProgramBytecode> CompileSpirvBytecode(const String& vkslSource, GpuProgramType programType, u32 pushConstantBufferSize)
	{
		static GLSLToSPIRV converter("MetalSource", 1);

		GpuProgramCreateInformation createInformation;
		createInformation.Source = vkslSource;
		createInformation.EntryPoint = "main";
		createInformation.Language = "vksl";
		createInformation.Type = programType;
		createInformation.PushConstantBufferSize = pushConstantBufferSize;

		return converter.CompileBytecode(createInformation);
	}

	/** Returns the SPIR-V execution model corresponding to an engine GPU program stage. */
	spv::ExecutionModel GetExecutionModel(GpuProgramType programType)
	{
		switch(programType)
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

	/** Returns the MSL entry-point name reserved for an engine GPU program stage. */
	const char* GetEntryPointName(GpuProgramType programType)
	{
		const char* entryPointName = GetMetalStageEntryPointName(programType);
		B3D_ASSERT(entryPointName[0] != '\0');
		return entryPointName;
	}

	/** Appends one newline-terminated message to a compiler result. */
	void AppendError(ShaderCompilerResult& outResult, const String& message)
	{
		outResult.ErrorMessage += message;

		if(outResult.ErrorMessage.empty() || outResult.ErrorMessage.back() != '\n')
			outResult.ErrorMessage += '\n';
	}

	/** Orders reflected parameter information by its stable argument-buffer binding coordinates. */
	template<class TInformation>
	bool CompareResourceBindings(const TInformation* left, const TInformation* right)
	{
		if(left->Set != right->Set)
			return (left->Set) < right->Set;

		if(left->Slot != right->Slot)
			return left->Slot < right->Slot;

		return left->Name < right->Name;
	}

	/**
	 * Builds SPIRV-Cross resource bindings in the order required by the runtime Metal argument-buffer layout.
	 *
	 * Binding indices are shared between resource classes within each parameter set. Texture and sampler counts are
	 * also tracked independently because Metal applies separate per-stage limits to those argument types.
	 */
	class MetalResourceBindingBuilder final
	{
	public:
		MetalResourceBindingBuilder(spirv_cross::CompilerMSL& outCompiler, spv::ExecutionModel executionModel, ShaderCompilerResult& outResult)
			: mCompiler(outCompiler), mExecutionModel(executionModel), mResult(outResult)
		{ }

		/** Adds all reflected engine resources and validates the resulting Metal argument-buffer layout. */
		void Build(const GpuProgramParameterDescription& parameterDescription)
		{
			for(const Map<String, GpuObjectParameterInformation>::value_type& bufferEntry : parameterDescription.Buffers)
			{
				const GpuParameterObjectType bufferType = bufferEntry.second.Type;
				if(bufferType != GPOT_STRUCTURED_BUFFER && bufferType != GPOT_RWSTRUCTURED_BUFFER && bufferType != GPOT_BYTE_BUFFER && bufferType != GPOT_RWBYTE_BUFFER)
					AppendError(mResult, StringUtility::Format("Metal storage buffers with counters are not supported: {0}.", bufferEntry.second.Name));
			}

			// Resource classes must be added in the same order used by the runtime argument-buffer ABI.
			static_assert(kTypeOrderUniformBuffer == 0 && kTypeOrderSampledTexture == 1 && kTypeOrderStorageTexture == 2 && kTypeOrderStorageBuffer == 3 && kTypeOrderSampler == 4, "Metal argument-buffer resource order changed; update binding construction below.");

			AddResources(parameterDescription.UniformBuffers, GpuParameterType::UniformBuffer);
			AddResources(parameterDescription.SampledTextures, GpuParameterType::SampledTexture);
			AddResources(parameterDescription.StorageTextures, GpuParameterType::StorageTexture);
			AddResources(parameterDescription.Buffers, GpuParameterType::StorageBuffer);
			AddResources(parameterDescription.Samplers, GpuParameterType::Sampler);

			if(mTextureArgumentCount > kMetalMaximumTextureArgumentsPerStage)
				AppendError(mResult, StringUtility::Format("Metal shader exceeds the Apple7 Tier-2 texture argument limit: {0} > {1}.", mTextureArgumentCount, kMetalMaximumTextureArgumentsPerStage));

			if(mSamplerArgumentCount > kMetalMaximumSamplerArgumentsPerStage)
				AppendError(mResult, StringUtility::Format("Metal shader exceeds the Apple7 Tier-2 sampler argument limit: {0} > {1}.", mSamplerArgumentCount, kMetalMaximumSamplerArgumentsPerStage));
		}

	private:
		/** Returns the number of consecutive argument-buffer indices occupied by a uniform buffer. */
		static u32 GetArgumentCount(const GpuUniformBufferInformation&)
		{
			return 1;
		}

		/** Returns the number of consecutive argument-buffer indices occupied by an object resource. */
		static u32 GetArgumentCount(const GpuObjectParameterInformation& information)
		{
			return information.ArraySize;
		}

		/** Returns whether a uniform buffer occupies a Metal texture argument. */
		static bool UsesTextureArgument(GpuParameterType, const GpuUniformBufferInformation&)
		{
			return false;
		}

		/** Returns whether an object resource occupies a Metal texture argument. */
		static bool UsesTextureArgument(GpuParameterType parameterType, const GpuObjectParameterInformation& information)
		{
			if(parameterType == GpuParameterType::SampledTexture || parameterType == GpuParameterType::StorageTexture)
				return true;

			return parameterType == GpuParameterType::StorageBuffer && (information.Type == GPOT_BYTE_BUFFER || information.Type == GPOT_RWBYTE_BUFFER);
		}

		/** Adds one resource category to the Metal argument-buffer layout. */
		template<class TEntries>
		void AddResources(const TEntries& entries, GpuParameterType parameterType)
		{
			using InformationType = typename TEntries::mapped_type;
			Vector<const InformationType*> sortedInformation;
			sortedInformation.reserve(entries.size());
			for(const typename TEntries::value_type& entry : entries)
				sortedInformation.push_back(&entry.second);

			std::sort(sortedInformation.begin(), sortedInformation.end(), CompareResourceBindings<InformationType>);

			for(const InformationType* information : sortedInformation)
			{
				const u32 argumentCount = GetArgumentCount(*information);
				if(information->Set > kMetalMaximumParameterSetIndex || information->Slot > kMetalMaximumArgumentBufferSlot || argumentCount == 0 || argumentCount > kMetalArgumentBufferArrayStride)
				{
					AppendError(mResult, StringUtility::Format("Metal argument-buffer binding is outside the supported ABI: set={0}, slot={1}, arraySize={2}.", information->Set, information->Slot, argumentCount));
					continue;
				}

				const u64 bindingKey = ((u64)information->Set << 32) | information->Slot;
				if(!mOccupiedBindings.insert(bindingKey).second)
				{
					AppendError(mResult, StringUtility::Format("Metal shader declares multiple resources at set {0}, slot {1}. SPIR-V descriptor coordinates must be unique.", information->Set, information->Slot));
					continue;
				}

				u64& nextArgumentIndex = mNextArgumentIndices[information->Set];
				if(nextArgumentIndex + argumentCount > std::numeric_limits<u32>::max())
				{
					AppendError(mResult, StringUtility::Format("Metal argument-buffer index space overflowed for set {0}.", information->Set));
					continue;
				}

				spirv_cross::MSLResourceBinding binding;
				binding.stage = mExecutionModel;
				binding.desc_set = information->Set;
				binding.binding = information->Slot;
				binding.count = argumentCount;

				if(UsesTextureArgument(parameterType, *information))
				{
					// Typed buffers are SPIR-V images, so SPIRV-Cross resolves their index through msl_texture.
					binding.msl_texture = (u32)nextArgumentIndex;
					mTextureArgumentCount += argumentCount;
				}
				else if(parameterType == GpuParameterType::Sampler)
				{
					binding.msl_sampler = (u32)nextArgumentIndex;
					mSamplerArgumentCount += argumentCount;
				}
				else
					binding.msl_buffer = (u32)nextArgumentIndex;

				nextArgumentIndex += argumentCount;
				mCompiler.add_msl_resource_binding(binding);
			}
		}

		spirv_cross::CompilerMSL& mCompiler;
		spv::ExecutionModel mExecutionModel;
		ShaderCompilerResult& mResult;
		u64 mNextArgumentIndices[kMetalMaximumParameterSetIndex + 1] = {};
		UnorderedSet<u64> mOccupiedBindings;
		u64 mTextureArgumentCount = 0;
		u64 mSamplerArgumentCount = 0;
	};

	/**
	 * Encodes engine binding coordinates into reflected SPIR-V resource names before MSL generation.
	 *
	 * Typed buffers require special handling because SPIR-V reflects them as images while the engine parameter
	 * description stores them in the buffer collection.
	 */
	class MetalResourceRenamer final
	{
	public:
		MetalResourceRenamer(spirv_cross::CompilerMSL& outCompiler, const GpuProgramParameterDescription& parameterDescription, ShaderCompilerResult& outResult)
			: mCompiler(outCompiler), mParameterDescription(parameterDescription), mResult(outResult)
		{ }

		/** Renames every reflected resource and verifies that every declared engine resource was found. */
		bool Rename()
		{
			const spirv_cross::ShaderResources resources = mCompiler.get_shader_resources();

			const UnorderedSet<String> mappedUniformBufferNames = RenameExactResources(resources.uniform_buffers, mParameterDescription.UniformBuffers, GpuParameterType::UniformBuffer);
			ReportMissingResources(mParameterDescription.UniformBuffers, mappedUniformBufferNames);

			RenameImageResources(resources.separate_images, mParameterDescription.SampledTextures, GpuParameterType::SampledTexture, GPOT_BYTE_BUFFER);
			RenameImageResources(resources.storage_images, mParameterDescription.StorageTextures, GpuParameterType::StorageTexture, GPOT_RWBYTE_BUFFER);

			const UnorderedSet<String> mappedStructuredBufferNames = RenameExactResources(resources.storage_buffers, mParameterDescription.Buffers, GpuParameterType::StorageBuffer);
			mMappedBufferNames.insert(mappedStructuredBufferNames.begin(), mappedStructuredBufferNames.end());

			const UnorderedSet<String> mappedSamplerNames = RenameExactResources(resources.separate_samplers, mParameterDescription.Samplers, GpuParameterType::Sampler);
			ReportMissingResources(mParameterDescription.Samplers, mappedSamplerNames);
			ReportMissingResources(mParameterDescription.Buffers, mMappedBufferNames);

			if(!resources.sampled_images.empty())
			{
				AppendError(mResult, "Metal source generation received combined image/sampler resources even though the native Metal target requires separate images and samplers.");
				mValid = false;
			}

			return mValid;
		}

	private:
		/** Returns the source-level name of a reflected SPIR-V resource. */
		String GetResourceName(const spirv_cross::Resource& resource) const
		{
			return !resource.name.empty()
				? String(resource.name.c_str())
				: String(mCompiler.get_name(resource.id).c_str());
		}

		/** Renames one reflected resource using its engine parameter information. */
		template<class TInformation>
		void RenameResource(const spirv_cross::Resource& resource, GpuParameterType parameterType, const TInformation& information)
		{
			const String encodedName = EncodeMetalResourceName(parameterType, information.Set, information.Slot, information.Name);
			mCompiler.set_name(resource.id, encodedName.c_str());
		}

		/** Renames resources that map directly to one engine parameter collection. */
		template<class TResources, class TEntries>
		UnorderedSet<String> RenameExactResources(const TResources& reflectedResources, const TEntries& entries, GpuParameterType parameterType)
		{
			UnorderedSet<String> mappedNames;
			for(const spirv_cross::Resource& resource : reflectedResources)
			{
				const String resourceName = GetResourceName(resource);
				const typename TEntries::const_iterator found = entries.find(resourceName);
				if(found == entries.end())
				{
					AppendError(mResult, StringUtility::Format("Metal source generation could not map SPIR-V resource '{0}' to an engine binding.", resourceName));
					mValid = false;
					continue;
				}

				RenameResource(resource, parameterType, found->second);
				mappedNames.insert(found->first);
			}

			return mappedNames;
		}

		/** Renames reflected images, including typed buffers represented as SPIR-V Buffer-dimension images. */
		template<class TResources>
		void RenameImageResources(const TResources& reflectedResources, const Map<String, GpuObjectParameterInformation>& textureEntries, GpuParameterType textureType, GpuParameterObjectType typedBufferType)
		{
			UnorderedSet<String> mappedTextureNames;
			for(const spirv_cross::Resource& resource : reflectedResources)
			{
				const String resourceName = GetResourceName(resource);
				const Map<String, GpuObjectParameterInformation>::const_iterator texture = textureEntries.find(resourceName);
				if(texture != textureEntries.end())
				{
					RenameResource(resource, textureType, texture->second);
					mappedTextureNames.insert(texture->first);
					continue;
				}

				const Map<String, GpuObjectParameterInformation>::const_iterator buffer = mParameterDescription.Buffers.find(resourceName);
				if(buffer != mParameterDescription.Buffers.end() && buffer->second.Type == typedBufferType)
				{
					RenameResource(resource, GpuParameterType::StorageBuffer, buffer->second);
					mMappedBufferNames.insert(buffer->first);
					continue;
				}

				AppendError(mResult, StringUtility::Format("Metal source generation could not map SPIR-V resource '{0}' to an engine binding.", resourceName));
				mValid = false;
			}

			ReportMissingResources(textureEntries, mappedTextureNames);
		}

		/** Reports engine parameters that were not present in the reflected SPIR-V resources. */
		template<class TEntries>
		void ReportMissingResources(const TEntries& entries, const UnorderedSet<String>& mappedNames)
		{
			for(const typename TEntries::value_type& entry : entries)
			{
				if(mappedNames.find(entry.first) == mappedNames.end())
				{
					AppendError(mResult, StringUtility::Format("Metal source generation did not find declared SPIR-V resource '{0}' while encoding engine bindings.", entry.first));
					mValid = false;
				}
			}
		}

		spirv_cross::CompilerMSL& mCompiler;
		const GpuProgramParameterDescription& mParameterDescription;
		ShaderCompilerResult& mResult;
		UnorderedSet<String> mMappedBufferNames;
		bool mValid = true;
	};

	/** Combined result of internal VKSL-to-MSL translation. */
	struct MetalSourceCompilation
	{
		ShaderCompilerResult Result;
		String MslSource;
	};

	/** Compiles VKSL to MSL and returns the generated source together with any diagnostics. */
	MetalSourceCompilation CompileMetalSource(const String& vkslSource, GpuProgramType programType, u32 pushConstantBufferSize)
	{
		MetalSourceCompilation compilation;
		if(programType != GPT_VERTEX_PROGRAM && programType != GPT_FRAGMENT_PROGRAM && programType != GPT_COMPUTE_PROGRAM)
		{
			compilation.Result.ErrorMessage = "The macOS Metal backend does not support geometry or tessellation shader stages.";
			return compilation;
		}

		TShared<GpuProgramBytecode> spirvBytecode = CompileSpirvBytecode(vkslSource, programType, pushConstantBufferSize);
		if(spirvBytecode == nullptr || spirvBytecode->Instructions.Data == nullptr || spirvBytecode->Instructions.Size == 0)
		{
			compilation.Result.ErrorMessage = spirvBytecode != nullptr && !spirvBytecode->Messages.empty()
				? spirvBytecode->Messages
				: "Failed to compile the VKSL intermediate to SPIR-V for Metal source generation.";

			return compilation;
		}

		B3D_ASSERT((spirvBytecode->Instructions.Size % sizeof(u32)) == 0);

		spirv_cross::CompilerMSL compiler((u32*)spirvBytecode->Instructions.Data, spirvBytecode->Instructions.Size / sizeof(u32));
		const spv::ExecutionModel executionModel = GetExecutionModel(programType);
		compiler.rename_entry_point("main", GetEntryPointName(programType), executionModel);

		const spirv_cross::ShaderResources resources = compiler.get_shader_resources();
		if(resources.push_constant_buffers.size() > 1)
		{
			compilation.Result.ErrorMessage = "Metal source generation received more than one SPIR-V push-constant block.";
			return compilation;
		}

		const u32 reflectedPushConstantBufferSize = spirvBytecode->ParameterDescription != nullptr ? spirvBytecode->ParameterDescription->PushConstantBufferSize : 0;
		if(reflectedPushConstantBufferSize != pushConstantBufferSize)
		{
			compilation.Result.ErrorMessage = StringUtility::Format("Metal SPIR-V push-constant size mismatch: source reflection reports {0} bytes but SPIR-V reports {1} bytes.", pushConstantBufferSize, reflectedPushConstantBufferSize);
			return compilation;
		}

		if((pushConstantBufferSize != 0) != !resources.push_constant_buffers.empty())
		{
			compilation.Result.ErrorMessage = "Metal SPIR-V push-constant presence does not match source reflection.";
			return compilation;
		}

		if(pushConstantBufferSize != 0)
		{
			spirv_cross::MSLResourceBinding pushConstantBinding;
			pushConstantBinding.stage = executionModel;
			pushConstantBinding.desc_set = spirv_cross::ResourceBindingPushConstantDescriptorSet;
			pushConstantBinding.binding = spirv_cross::ResourceBindingPushConstantBinding;
			pushConstantBinding.count = 1;
			pushConstantBinding.msl_buffer = kMetalPushConstantBufferIndex;

			compiler.add_msl_resource_binding(pushConstantBinding);
		}

		spirv_cross::CompilerMSL::Options mslOptions;
		mslOptions.msl_version = spirv_cross::CompilerMSL::Options::make_msl_version(3, 0);
		mslOptions.argument_buffers = true;
		mslOptions.argument_buffers_tier = spirv_cross::CompilerMSL::Options::ArgumentBuffersTier::Tier2;
		mslOptions.platform = spirv_cross::CompilerMSL::Options::macOS;

		// Typed buffers (Buffer<T>/RWBuffer<T>) emit as native texture_buffer<T> rather than a texture2d emulation
		mslOptions.texture_buffer_native = true;

		// Every stage of a pass declares the full resource set; emitting members for statically-unused
		// resources too keeps the argument-buffer struct layout identical across the pass's stages,
		// which the runtime's common Tier-2 argument-buffer layout requires.
		mslOptions.force_active_argument_buffer_resources = true;

		compiler.set_msl_options(mslOptions);

		spirv_cross::CompilerGLSL::Options commonOptions;
		commonOptions.separate_shader_objects = true;
		commonOptions.vulkan_semantics = true;
		commonOptions.vertex.flip_vert_y = false;
		compiler.set_common_options(commonOptions);

		if(spirvBytecode->ParameterDescription != nullptr)
		{
			const GpuProgramParameterDescription& parameterDescription = *spirvBytecode->ParameterDescription;
			MetalResourceBindingBuilder bindingBuilder(compiler, executionModel, compilation.Result);
			bindingBuilder.Build(parameterDescription);
			if(!compilation.Result.ErrorMessage.empty())
				return compilation;

			MetalResourceRenamer resourceRenamer(compiler, parameterDescription, compilation.Result);
			if(!resourceRenamer.Rename())
				return compilation;
		}

		compilation.MslSource = compiler.compile();
		if(compiler.needs_swizzle_buffer() || compiler.needs_buffer_size_buffer() || compiler.needs_view_mask_buffer() || compiler.needs_dispatch_base_buffer() || compiler.needs_output_buffer() || compiler.needs_patch_output_buffer() || compiler.needs_input_threadgroup_mem())
		{
			compilation.MslSource.clear();
			compilation.Result.ErrorMessage = "The Metal shader requires a SPIRV-Cross auxiliary buffer that the backend does not "
				"encode. Runtime-sized storage-buffer arrays, swizzle emulation, multiview, and tessellation "
				"auxiliary buffers are not supported.";

			return compilation;
		}

		if(compilation.MslSource.empty())
		{
			compilation.Result.ErrorMessage = "SPIRV-Cross produced empty MSL source.";
			return compilation;
		}

		if(pushConstantBufferSize != 0 && compilation.MslSource.find(StringUtility::Format("[[buffer({0})]]", kMetalPushConstantBufferIndex)) == String::npos)
		{
			compilation.MslSource.clear();
			compilation.Result.ErrorMessage = "SPIRV-Cross did not emit the Metal push-constant block at the reserved buffer index.";
			return compilation;
		}

		return compilation;
	}
}

ShaderCompilerResult MetalSourceCompiler::Compile(const String& vkslSource, GpuProgramType programType, u32 pushConstantBufferSize, String& outMslSource)
{
	outMslSource.clear();

	MetalSourceCompilation compilation = CompileMetalSource(vkslSource, programType, pushConstantBufferSize);
	outMslSource = std::move(compilation.MslSource);
	return std::move(compilation.Result);
}

#endif // B3D_PLATFORM_MACOS
