//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Managers/BsVulkanGLSLProgramFactory.h"
#include "Math/BsMath.h"
#include "BsVulkanGpuProgram.h"
#include "BsVulkanGLSLToSPIRV.h"
#include "RenderAPI/BsGpuParamDesc.h"

#if B3D_PLATFORM == B3D_PLATFORM_ID_MACOS
#	include "spirv_cross/spirv_msl.hpp"
#	include "spirv_cross/spirv_glsl.hpp"
#endif

using namespace bs;
using namespace bs::ct;

template <class T, class CB>
void IterateSorted(const Map<String, T>& entries, CB callback)
{
	auto count = (u32)entries.size();
	auto sortedEntries = B3DManagedStackAllocate<const T*>(count);

	u32 i = 0;
	for(auto& entry : entries)
		sortedEntries[i++] = &entry.second;

	std::sort(sortedEntries + 0, sortedEntries + count, [](const T* a, const T* b)
			  {
			  if(a->set == b->set)
				  return a->slot < b->slot;

			  return a->set < b->set; });

	for(i = 0; i < count; i++)
		callback(sortedEntries[i]);
}

VulkanGLSLProgramFactory::VulkanGLSLProgramFactory()
{
	GLSLToSPIRV::StartUp();
}

VulkanGLSLProgramFactory::~VulkanGLSLProgramFactory()
{
	GLSLToSPIRV::ShutDown();
}

SPtr<ct::GpuProgram> VulkanGLSLProgramFactory::Create(const GpuProgramCreateInformation& desc, GpuDeviceFlags deviceMask)
{
	SPtr<GpuProgram> gpuProg = B3DMakeSharedFromExisting<VulkanGpuProgram>(new(B3DAllocate<VulkanGpuProgram>())
																   VulkanGpuProgram(desc, deviceMask));
	gpuProg->SetThisPtrInternal(gpuProg);

	return gpuProg;
}

SPtr<ct::GpuProgram> VulkanGLSLProgramFactory::Create(GpuProgramType type, GpuDeviceFlags deviceMask)
{
	GpuProgramCreateInformation desc;
	desc.Type = type;

	SPtr<GpuProgram> gpuProg = B3DMakeSharedFromExisting<VulkanGpuProgram>(new(B3DAllocate<VulkanGpuProgram>())
																   VulkanGpuProgram(desc, deviceMask));
	gpuProg->SetThisPtrInternal(gpuProg);

	return gpuProg;
}

SPtr<GpuProgramBytecode> VulkanGLSLProgramFactory::CompileBytecode(const GpuProgramCreateInformation& desc)
{
	SPtr<GpuProgramBytecode> spirv = GLSLToSPIRV::Instance().Convert(desc);

#if B3D_PLATFORM == B3D_PLATFORM_ID_MACOS
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

	B3D_ASSERT((spirv->instructions.size % sizeof(u32)) == 0);

	// Compile to MSL
	spirv_cross::CompilerMSL compiler((u32*)spirv->instructions.data, spirv->instructions.size / sizeof(u32));

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
			B3D_ASSERT(false);
			break;
		}

		auto count = msl->paramDesc->paramBlocks.size() + msl->paramDesc->textures.size() + msl->paramDesc->samplers.size() + msl->paramDesc->loadStoreTextures.size() + msl->paramDesc->buffers.size();

		auto sortedEntries = B3DManagedStackAllocate<spirv_cross::MSLResourceBinding>((u32)count);
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

		std::sort(sortedEntries + 0, sortedEntries + count, [](const spirv_cross::MSLResourceBinding& a, const spirv_cross::MSLResourceBinding& b)
				  {
					if(a.desc_set == b.desc_set)
						return a.binding < b.binding;

					return a.desc_set < b.desc_set; });

		u32 bufferIdx = 0;
		u32 samplerIdx = 0;
		u32 textureIdx = 0;

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
	std::string source = compiler.Compile();

	// Parse workgroup size for compute shaders
	u32 workgroupSize[3] = { 1, 1, 1 };
	if(desc.type == GPT_COMPUTE_PROGRAM)
	{
		spirv_cross::SPIREntryPoint spvEP;
		const auto& entryPoints = compiler.get_entry_points_and_stages();
		if(!entryPoints.empty())
		{
			auto& ep = entryPoints[0];
			spvEP = compiler.get_entry_point(ep.name, ep.execution_model);
		}

		workgroupSize[0] = spvEP.workgroup_size.X;
		workgroupSize[1] = spvEP.workgroup_size.Y;
		workgroupSize[2] = spvEP.workgroup_size.Z;
	}

	// Copy the source into destination buffer
	if(msl->instructions.data)
		B3DFree(msl->instructions.data);

	if(source.empty())
	{
		msl->instructions = DataBlob();
		return msl;
	}

	// Magic numbers as defined in vk_mvk_moltenvk.h
	constexpr u32 MVK_MSL_Source = 0x19960412;

	u32 size = (u32)source.size() + sizeof(MVK_MSL_Source) + 1;
	if(desc.type == GPT_COMPUTE_PROGRAM)
		size += sizeof(workgroupSize);

	u32 wordSize = Math::DivideAndRoundUp(size, 4U);

	u8* buffer = (u8*)B3DAllocate(wordSize * 4);
	u8* dst = buffer;

	if(desc.type == GPT_COMPUTE_PROGRAM)
	{
		memcpy(dst, workgroupSize, sizeof(workgroupSize));
		dst += sizeof(workgroupSize);
	}

	memcpy(dst, &MVK_MSL_Source, sizeof(MVK_MSL_Source));
	dst += sizeof(MVK_MSL_Source);

	memcpy(dst, source.data(), source.size());

	for(u32 i = size - 1; i < wordSize * 4; i++)
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
