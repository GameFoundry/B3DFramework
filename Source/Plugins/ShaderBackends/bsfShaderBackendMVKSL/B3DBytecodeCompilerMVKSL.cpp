//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DBytecodeCompilerMVKSL.h"

#if B3D_PLATFORM_MACOS

#include <algorithm>
#include "spirv_cross/spirv_msl.hpp"
#include "B3DGLSLToSPIRV.h"
#include "GpuBackend/B3DGpuProgram.h"
#include "GpuBackend/B3DGpuProgramParameterDescription.h"
#include "Math/B3DMath.h"

using namespace b3d;
using namespace b3d::render;

TShared<IGpuBytecodeCompiler> render::CreateBytecodeCompilermvksl()
{
	return B3DMakeShared<BytecodeCompilerMVKSL>(kMoltenVkCompilerId, kMoltenVkCompilerVersion);
}

BytecodeCompilerMVKSL::BytecodeCompilerMVKSL(const char* compilerId, u32 compilerVersion)
	: mConverter(B3DMakeUnique<GLSLToSPIRV>(compilerId, compilerVersion))
{ }

BytecodeCompilerMVKSL::~BytecodeCompilerMVKSL() = default;

bool BytecodeCompilerMVKSL::IsUpToDate(const GpuProgramBytecode& bytecode) const
{
	return mConverter->IsUpToDate(bytecode);
}

TShared<GpuProgramBytecode> BytecodeCompilerMVKSL::CompileBytecode(const GpuProgramCreateInformation& createInformation)
{
	TShared<GpuProgramBytecode> spirv = mConverter->CompileBytecode(createInformation);
	// We'll just re-purpose the existing data structure
	TShared<GpuProgramBytecode> msl = spirv;

	// SPIR-V failed to compile, just pass along the data structure with updated compiler ID
	if(spirv->Instructions.Size == 0 || !spirv->Instructions.Data)
	{
		msl->Instructions = DataBlob();
		return msl;
	}

	B3D_ASSERT((spirv->Instructions.Size % sizeof(u32)) == 0);

	// Compile to MSL
	spirv_cross::CompilerMSL compiler((u32*)spirv->Instructions.Data, spirv->Instructions.Size / sizeof(u32));

	// Remap resource bindings
	if(msl->ParameterDescription)
	{
		spv::ExecutionModel stage;
		switch(createInformation.Type)
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

		auto count = msl->ParameterDescription->UniformBuffers.size() + msl->ParameterDescription->SampledTextures.size() + msl->ParameterDescription->Samplers.size() + msl->ParameterDescription->StorageTextures.size() + msl->ParameterDescription->Buffers.size();

		// Collected resource bindings, sorted by (set, slot) below before flat MSL indices are assigned.
		TInlineArray<spirv_cross::MSLResourceBinding, 32> sortedEntries;
		sortedEntries.resize(count);
		size_t i = 0;

		for(auto& entry : msl->ParameterDescription->UniformBuffers)
		{
			spirv_cross::MSLResourceBinding& binding = sortedEntries[i++];
			binding.stage = stage;
			binding.desc_set = entry.second.Set;
			binding.binding = entry.second.Slot;
			binding.msl_buffer = 2;
		}

		for(auto& entry : msl->ParameterDescription->SampledTextures)
		{
			spirv_cross::MSLResourceBinding& binding = sortedEntries[i++];
			binding.stage = stage;
			binding.desc_set = entry.second.Set;
			binding.binding = entry.second.Slot;
			binding.msl_buffer = 0;
		}

		for(auto& entry : msl->ParameterDescription->Samplers)
		{
			spirv_cross::MSLResourceBinding& binding = sortedEntries[i++];
			binding.stage = stage;
			binding.desc_set = entry.second.Set;
			binding.binding = entry.second.Slot;
			binding.msl_buffer = 1;
		}

		for(auto& entry : msl->ParameterDescription->StorageTextures)
		{
			spirv_cross::MSLResourceBinding& binding = sortedEntries[i++];
			binding.stage = stage;
			binding.desc_set = entry.second.Set;
			binding.binding = entry.second.Slot;
			binding.msl_buffer = 0;
		}

		for(auto& entry : msl->ParameterDescription->Buffers)
		{
			spirv_cross::MSLResourceBinding& binding = sortedEntries[i++];
			binding.stage = stage;
			binding.desc_set = entry.second.Set;
			binding.binding = entry.second.Slot;

			// Non-structured buffers treated as textures by MSL
			if(entry.second.Type == GPOT_BYTE_BUFFER || entry.second.Type == GPOT_RWBYTE_BUFFER)
				binding.msl_buffer = 0;
			else
				binding.msl_buffer = 2;
		}

		std::sort(sortedEntries.begin(), sortedEntries.end(), [](const spirv_cross::MSLResourceBinding& a, const spirv_cross::MSLResourceBinding& b)
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
	std::string source = compiler.compile();

	// Parse workgroup size for compute shaders
	u32 workgroupSize[3] = { 1, 1, 1 };
	if(createInformation.Type == GPT_COMPUTE_PROGRAM)
	{
		spirv_cross::SPIREntryPoint spvEP;
		const auto& entryPoints = compiler.get_entry_points_and_stages();
		if(!entryPoints.empty())
		{
			auto& ep = entryPoints[0];
			spvEP = compiler.get_entry_point(ep.name, ep.execution_model);
		}

		workgroupSize[0] = spvEP.workgroup_size.x;
		workgroupSize[1] = spvEP.workgroup_size.y;
		workgroupSize[2] = spvEP.workgroup_size.z;
	}

	// Copy the source into destination buffer
	if(msl->Instructions.Data)
		B3DFree(msl->Instructions.Data);

	if(source.empty())
	{
		msl->Instructions = DataBlob();
		return msl;
	}

	// Magic numbers as defined in vk_mvk_moltenvk.h
	constexpr u32 MVK_MSL_Source = 0x19960412;

	u32 size = (u32)source.size() + sizeof(MVK_MSL_Source) + 1;
	if(createInformation.Type == GPT_COMPUTE_PROGRAM)
		size += sizeof(workgroupSize);

	u32 wordSize = Math::DivideAndRoundUp(size, 4U);

	u8* buffer = (u8*)B3DAllocate(wordSize * 4);
	u8* dst = buffer;

	if(createInformation.Type == GPT_COMPUTE_PROGRAM)
	{
		memcpy(dst, workgroupSize, sizeof(workgroupSize));
		dst += sizeof(workgroupSize);
	}

	memcpy(dst, &MVK_MSL_Source, sizeof(MVK_MSL_Source));
	dst += sizeof(MVK_MSL_Source);

	memcpy(dst, source.data(), source.size());

	for(u32 i = size - 1; i < wordSize * 4; i++)
		buffer[i] = '\0';

	msl->Instructions.Size = wordSize * 4;
	msl->Instructions.Data = buffer;

	return msl;

	// TODO - Compile the Metal source code into intermediate representation, right now we aren't outputting bytecode,
	// just for the sake of trying to get MoltenVK port running in the first place.
	// (Ideally we can also move GLSL->SPIRV->MSL steps to the shader importer, so we just receive pure MSL here, as that
	// would make the system ready for when we have a proper MSL cross-compiler. Downside of this approach is that
	// we then need shader reflection code for MSL).
}

#endif // B3D_PLATFORM_MACOS
