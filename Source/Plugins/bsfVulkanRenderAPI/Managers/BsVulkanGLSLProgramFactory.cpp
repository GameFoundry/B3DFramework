//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Managers/BsVulkanGLSLProgramFactory.h"
#include "Math/BsMath.h"
#include "BsVulkanGpuProgram.h"
#include "BsVulkanGLSLToSPIRV.h"
#include "RenderAPI/BsGpuParamDesc.h"

#if BS_PLATFORM == BS_PLATFORM_OSX
#include "spirv_cross/spirv_msl.hpp"
#include "spirv_cross/spirv_glsl.hpp"
#endif

namespace bs { namespace ct
{
	template<class T, class CB>
	void iterateSorted(const Map<String, T>& entries, CB callback)
	{
		auto count = (UINT32)entries.size();
		auto sortedEntries = bs_managed_stack_alloc<const T*>(count);

		UINT32 i = 0;
		for(auto& entry : entries)
			sortedEntries[i++] = &entry.second;

		std::sort(sortedEntries + 0, sortedEntries + count,
		  [](const T* a, const T* b)
		  {
			  if(a->set == b->set)
				  return a->slot < b->slot;

			  return a->set < b->set;
		  });

		for(i = 0; i < count; i++)
			callback(sortedEntries[i]);
	}

	VulkanGLSLProgramFactory::VulkanGLSLProgramFactory()
	{
		GLSLToSPIRV::startUp();
	}

	VulkanGLSLProgramFactory::~VulkanGLSLProgramFactory()
	{
		GLSLToSPIRV::shutDown();
	}

	SPtr<GpuProgram> VulkanGLSLProgramFactory::create(const GPU_PROGRAM_DESC& desc, GpuDeviceFlags deviceMask)
	{
		SPtr<GpuProgram> gpuProg = bs_shared_ptr<VulkanGpuProgram>(new (bs_alloc<VulkanGpuProgram>())
			VulkanGpuProgram(desc, deviceMask));
		gpuProg->_setThisPtr(gpuProg);

		return gpuProg;
	}

	SPtr<GpuProgram> VulkanGLSLProgramFactory::create(GpuProgramType type, GpuDeviceFlags deviceMask)
	{
		GPU_PROGRAM_DESC desc;
		desc.type = type;

		SPtr<GpuProgram> gpuProg = bs_shared_ptr<VulkanGpuProgram>(new (bs_alloc<VulkanGpuProgram>())
			VulkanGpuProgram(desc, deviceMask));
		gpuProg->_setThisPtr(gpuProg);

		return gpuProg;
	}

	SPtr<GpuProgramBytecode> VulkanGLSLProgramFactory::compileBytecode(const GPU_PROGRAM_DESC& desc)
	{
		SPtr<GpuProgramBytecode> spirv = GLSLToSPIRV::instance().convert(desc);

#if BS_PLATFORM == BS_PLATFORM_OSX
		// We'll just re-purpose the existing data structure
		SPtr<GpuProgramBytecode> msl = spirv;
		msl->compilerId = MOLTENVK_COMPILER_ID;
		msl->compilerVersion = MOLTENVK_COMPILER_VERSION;

		// SPIR-V failed to compile, just pass along the data structure with updated compiler ID
		if(spirv->instructions.size == 0 || !spirv->instructions.data)
		{
			msl->instructions = DataBlob();
			return msl;
		}

		assert((spirv->instructions.size % sizeof(UINT32)) == 0);

		// Compile to MSL
		spirv_cross::CompilerMSL compiler((UINT32*)spirv->instructions.data, spirv->instructions.size / sizeof(UINT32));

		// Remap resource bindings
		if(msl->paramDesc)
		{
			spv::ExecutionModel stage;
			switch(desc.type)
			{
			case GPT_VERTEX_PROGRAM:
				stage = spv::ExecutionModelVertex;
				break;
			case GPT_FRAGMENT_PROGRAM:
				stage = spv::ExecutionModelFragment;
				break;
			case GPT_GEOMETRY_PROGRAM:
				stage = spv::ExecutionModelGeometry;
				break;
			case GPT_DOMAIN_PROGRAM:
				stage = spv::ExecutionModelTessellationEvaluation;
				break;
			case GPT_HULL_PROGRAM:
				stage = spv::ExecutionModelTessellationControl;
				break;
			case GPT_COMPUTE_PROGRAM:
				stage = spv::ExecutionModelGLCompute;
				break;
			default:
				assert(false);
				break;
			}

			auto count
				= msl->paramDesc->paramBlocks.size()
				+ msl->paramDesc->textures.size()
				+ msl->paramDesc->samplers.size()
				+ msl->paramDesc->loadStoreTextures.size()
				+ msl->paramDesc->buffers.size();

			auto sortedEntries = bs_managed_stack_alloc<spirv_cross::MSLResourceBinding>((UINT32)count);
			size_t i = 0;

			for(auto& entry : msl->paramDesc->paramBlocks)
			{
				spirv_cross::MSLResourceBinding& binding = sortedEntries[i++];
				binding.stage = stage;
				binding.desc_set = entry.second.set;
				binding.binding = entry.second.slot;
				binding.msl_buffer = 2;
			}

			for(auto& entry : msl->paramDesc->textures)
			{
				spirv_cross::MSLResourceBinding& binding = sortedEntries[i++];
				binding.stage = stage;
				binding.desc_set = entry.second.set;
				binding.binding = entry.second.slot;
				binding.msl_buffer = 0;
			}

			for(auto& entry : msl->paramDesc->samplers)
			{
				spirv_cross::MSLResourceBinding& binding = sortedEntries[i++];
				binding.stage = stage;
				binding.desc_set = entry.second.set;
				binding.binding = entry.second.slot;
				binding.msl_buffer = 1;
			}

			for(auto& entry : msl->paramDesc->loadStoreTextures)
			{
				spirv_cross::MSLResourceBinding& binding = sortedEntries[i++];
				binding.stage = stage;
				binding.desc_set = entry.second.set;
				binding.binding = entry.second.slot;
				binding.msl_buffer = 0;
			}

			for(auto& entry : msl->paramDesc->buffers)
			{
				spirv_cross::MSLResourceBinding& binding = sortedEntries[i++];
				binding.stage = stage;
				binding.desc_set = entry.second.set;
				binding.binding = entry.second.slot;

				// Non-structured buffers treated as textures by MSL
				if(entry.second.type == GPOT_BYTE_BUFFER || entry.second.type == GPOT_RWBYTE_BUFFER)
					binding.msl_buffer = 0;
				else
					binding.msl_buffer = 2;
			}

			std::sort(sortedEntries + 0, sortedEntries + count,
				[](const spirv_cross::MSLResourceBinding& a, const spirv_cross::MSLResourceBinding& b)
				{
					if(a.desc_set == b.desc_set)
						return a.binding < b.binding;

					return a.desc_set < b.desc_set;
				});

			UINT32 bufferIdx = 0;
			UINT32 samplerIdx = 0;
			UINT32 textureIdx = 0;

			for(i = 0; i < count; i++)
			{
				spirv_cross::MSLResourceBinding& binding = sortedEntries[i];
				switch(binding.msl_buffer)
				{
				default:
				case 0: // Texture
					binding.msl_sampler = binding.msl_buffer = binding.msl_texture = textureIdx++;
					break;
				case 1: // Sampler
					binding.msl_sampler = binding.msl_buffer = binding.msl_texture = samplerIdx++;
					break;
				case 2: // Buffer
					binding.msl_sampler = binding.msl_buffer = binding.msl_texture = bufferIdx++;
					break;
				}

				compiler.add_msl_resource_binding(binding);
			}
		}

		spirv_cross::CompilerMSL::Options mslOptions;
		mslOptions.msl_version = spirv_cross::CompilerMSL::Options::make_msl_version(2, 1);
		compiler.set_msl_options(mslOptions);

		spirv_cross::CompilerGLSL::Options glslOptions;
		glslOptions.separate_shader_objects = true;
		glslOptions.vulkan_semantics = true;
		glslOptions.vertex.flip_vert_y = true;

		compiler.set_common_options(glslOptions);
		std::string source = compiler.compile();

		// Parse workgroup size for compute shaders
		UINT32 workgroupSize[3] = { 1, 1, 1 };
		if(desc.type == GPT_COMPUTE_PROGRAM)
		{
			spirv_cross::SPIREntryPoint spvEP;
			const auto& entryPoints = compiler.get_entry_points_and_stages();
			if (!entryPoints.empty())
			{
				auto& ep = entryPoints[0];
				spvEP = compiler.get_entry_point(ep.name, ep.execution_model);
			}

			workgroupSize[0] = spvEP.workgroup_size.x;
			workgroupSize[1] = spvEP.workgroup_size.y;
			workgroupSize[2] = spvEP.workgroup_size.z;
		}

		// Copy the source into destination buffer
		if(msl->instructions.data)
			bs_free(msl->instructions.data);

		if(source.empty())
		{
			msl->instructions = DataBlob();
			return msl;
		}

		// Magic numbers as defined in vk_mvk_moltenvk.h
		constexpr UINT32 MVK_MSL_Source = 0x19960412;

		UINT32 size = (UINT32)source.size() + sizeof(MVK_MSL_Source) + 1;
		if(desc.type == GPT_COMPUTE_PROGRAM)
			size += sizeof(workgroupSize);

		UINT32 wordSize = Math::divideAndRoundUp(size, 4U);

		UINT8* buffer = (UINT8*)bs_alloc(wordSize * 4);
		UINT8* dst = buffer;

		if(desc.type == GPT_COMPUTE_PROGRAM)
		{
			memcpy(dst, workgroupSize, sizeof(workgroupSize));
			dst += sizeof(workgroupSize);
		}

		memcpy(dst, &MVK_MSL_Source, sizeof(MVK_MSL_Source));
		dst += sizeof(MVK_MSL_Source);

		memcpy(dst, source.data(), source.size());

		for(UINT32 i = size - 1; i < wordSize * 4; i++)
			buffer[i] = '\0';

		msl->instructions.size = wordSize * 4;
		msl->instructions.data = buffer;

		return msl;

		// TODO - Compile the Metal source code into intermediate representation, right now we aren't outputting bytecode,
		// just for the sake of trying to get MoltenVK port running in the first place.
		// (Ideally we can also move GLSL->SPIRV->MSL steps to the shader importer, so we just receive pure MSL here, as that
		// would make the system ready for when we have a proper MSL cross-compiler. Downside of this approach is that
		// we then need shader reflection code for MSL).
#else
		return spirv;
#endif
	}
}}
