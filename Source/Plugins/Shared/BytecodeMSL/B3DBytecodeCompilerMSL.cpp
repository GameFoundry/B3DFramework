//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DBytecodeCompilerMSL.h"

#if B3D_PLATFORM_MACOS

#include "B3DGLSLToSPIRV.h"
#include "B3DMetalBytecodeLayout.h"
#include "GpuBackend/B3DGpuProgram.h"
#include "GpuBackend/B3DGpuProgramParameterDescription.h"

#include "spirv_cross/spirv_msl.hpp"

#include <algorithm>
#include <TargetConditionals.h>

using namespace b3d;
using namespace b3d::render;

TShared<IGpuBytecodeCompiler> render::CreateBytecodeCompilermsl()
{
	return B3DMakeShared<BytecodeCompilerMSL>(kMetalCompilerId, kMetalCompilerVersion);
}

BytecodeCompilerMSL::BytecodeCompilerMSL(const char* compilerId, u32 compilerVersion)
	: mConverter(B3DMakeUnique<GLSLToSPIRV>(compilerId, compilerVersion))
{ }

BytecodeCompilerMSL::~BytecodeCompilerMSL() = default;

TShared<GpuProgramBytecode> BytecodeCompilerMSL::CompileBytecode(const GpuProgramCreateInformation& createInformation)
{
	// Cache-hit short-circuit. Shader caches persist the already-compiled MSL payload on disk
	// (see MetalGpuProgram::Initialize — bytecode arrives here as the full packed MSL blob). If
	// the caller is re-invoking CompileBytecode on such pre-compiled bytecode, the
	// VKSL->SPIR-V->MSL pass is pure wasted work: the output would be bit-identical to the input.
	// Detect that case with the same magic check MetalGpuProgram::Initialize uses, and return the
	// bytecode as-is. The engine loader path does not re-compile cached bytecode today; this gate
	// future-proofs the thousands-of-permutations BSL compile hot path.
	if (createInformation.Bytecode
		&& createInformation.Bytecode->CompilerId == kMetalCompilerId
		&& createInformation.Bytecode->CompilerVersion == kMetalCompilerVersion
		&& createInformation.Bytecode->Instructions.Data != nullptr
		&& createInformation.Bytecode->Instructions.Size > 0)
		return createInformation.Bytecode;

	// Step 1: VKSL -> SPIR-V + reflection, via glslang + SPIRV-Cross. The returned
	// bytecode is already tagged with kMetalCompilerId / kMetalCompilerVersion; the final MSL
	// blob replaces the payload below but keeps the tag.
	TShared<GpuProgramBytecode> bytecode = mConverter->CompileBytecode(createInformation);

	// If SPIR-V conversion failed, short-circuit. The message log already explains why.
	if (bytecode->Instructions.Size == 0 || bytecode->Instructions.Data == nullptr)
		return bytecode;

	B3D_ASSERT((bytecode->Instructions.Size % sizeof(u32)) == 0);

	// Step 2: SPIR-V -> MSL via SPIRV-Cross, with argument-buffer emission enabled.
	spirv_cross::CompilerMSL compiler((u32*)bytecode->Instructions.Data, bytecode->Instructions.Size / sizeof(u32));

	// Determine the shader execution model (affects resource-binding stage).
	spv::ExecutionModel stage = spv::ExecutionModelVertex;
	switch (createInformation.Type)
	{
	case GPT_VERTEX_PROGRAM:   stage = spv::ExecutionModelVertex; break;
	case GPT_FRAGMENT_PROGRAM: stage = spv::ExecutionModelFragment; break;
	case GPT_GEOMETRY_PROGRAM: stage = spv::ExecutionModelGeometry; break;
	case GPT_DOMAIN_PROGRAM:   stage = spv::ExecutionModelTessellationEvaluation; break;
	case GPT_HULL_PROGRAM:     stage = spv::ExecutionModelTessellationControl; break;
	case GPT_COMPUTE_PROGRAM:  stage = spv::ExecutionModelGLCompute; break;
	default:
		B3D_ASSERT(false);
		break;
	}

	// Remap each reflected engine binding to an MSL-side binding that lives inside its set's
	// argument buffer. SPIRV-Cross's "discrete descriptors" fallback is intentionally not used
	// because argument buffers are the Metal backend's only binding path.
	//
	// Argument-buffer members share a single flat index namespace per set across buffers,
	// textures, and samplers. If we set msl_buffer/msl_texture/msl_sampler directly to the
	// engine slot, a UB at slot 0 and a sampler at slot 0 would map to the same argument-buffer
	// member and overwrite each other. Instead we assign monotonically increasing argument-buffer
	// indices per set using the exact same canonical ordering as
	// MetalGpuPipelineParameterSetLayout (type order: UB, SampledTex, StorageTex, StorageBuf,
	// Sampler; within each type, slot ascending).
	if (bytecode->ParameterDescription)
	{
		GpuProgramParameterDescription& paramDesc = *bytecode->ParameterDescription;

		// Type-order values (kTypeOrder*) are published on B3DMetalBytecodeLayout.h so this site
		// and @c MetalGpuPipelineParameterSetLayout's ctor consume the single authoritative
		// copy — any drift between the two would silently corrupt argument-buffer indices.
		//
		// Collected bindings are packed into a single u64 sort key so @c std::sort runs an
		// integer comparison instead of a dereferencing lambda — a measurable win on shader
		// permutations with dozens of bindings (the BSL cold-compile hot path fires thousands
		// of these). Layout, high->low bits: Set:24 | TypeOrder:8 | Slot:32. The set occupies
		// the most-significant range so a single sort pass already groups by set for free — no
		// outer per-set bucket / UnorderedMap is needed, and the argument-index counter simply
		// resets whenever the top bits of the key change.
		//
		// 32 fits every BSL permutation we've measured (worst case ~18 bindings across 4 sets);
		// the TInlineArray falls back to the heap for anything larger without changing the
		// calling code.
		TInlineArray<u64, 32> sortKeys;
		const auto fnCollectBinding = [&](u32 set, u32 slot, u64 typeOrder)
		{
			const u64 key = ((u64)set << 40) | (typeOrder << 32) | (u64)slot;
			sortKeys.Add(key);
		};

		for (auto& entry : paramDesc.UniformBuffers)
			fnCollectBinding(entry.second.Set, entry.second.Slot, kTypeOrderUniformBuffer);
		for (auto& entry : paramDesc.SampledTextures)
			fnCollectBinding(entry.second.Set, entry.second.Slot, kTypeOrderSampledTexture);
		for (auto& entry : paramDesc.StorageTextures)
			fnCollectBinding(entry.second.Set, entry.second.Slot, kTypeOrderStorageTexture);
		for (auto& entry : paramDesc.Buffers)
			fnCollectBinding(entry.second.Set, entry.second.Slot, kTypeOrderStorageBuffer);
		for (auto& entry : paramDesc.Samplers)
			fnCollectBinding(entry.second.Set, entry.second.Slot, kTypeOrderSampler);

		std::sort(sortKeys.begin(), sortKeys.end());

		// Walk the sorted keys assigning monotonically increasing argument-buffer indices. Reset
		// the counter on every set boundary (detected by the top 24 bits changing) so each set's
		// argument-buffer member namespace restarts at 0, matching the layout in
		// MetalGpuPipelineParameterSetLayout.
		u32 currentSet = ~0u;
		u32 argIndex = 0;
		for (u64 key : sortKeys)
		{
			const u32 set = (u32)(key >> 40);
			const u32 slot = (u32)(key & 0xFFFFFFFFull);
			if (set != currentSet)
			{
				currentSet = set;
				argIndex = 0;
			}

			spirv_cross::MSLResourceBinding mslBinding;
			mslBinding.stage = stage;
			mslBinding.desc_set = set;
			mslBinding.binding = slot;
			// Set all three because SPIRV-Cross picks the relevant one based on the resource's
			// MSL type, and argument-buffer members share a flat namespace so the same index
			// applies regardless of kind.
			mslBinding.msl_buffer = argIndex;
			mslBinding.msl_texture = argIndex;
			mslBinding.msl_sampler = argIndex;
			compiler.add_msl_resource_binding(mslBinding);
			++argIndex;
		}
	}

	// Enable argument buffers globally. From this point on, SPIRV-Cross emits one MSL argument
	// struct per descriptor set, and the encoder slots within each struct correspond to the
	// engine binding slots we registered above.
	spirv_cross::CompilerMSL::Options mslOptions;
	mslOptions.msl_version = spirv_cross::CompilerMSL::Options::make_msl_version(2, 1);
	mslOptions.argument_buffers = true;
#if TARGET_OS_IPHONE
	mslOptions.platform = spirv_cross::CompilerMSL::Options::iOS;
#else
	mslOptions.platform = spirv_cross::CompilerMSL::Options::macOS;
#endif
	compiler.set_msl_options(mslOptions);

	spirv_cross::CompilerGLSL::Options commonOptions;
	commonOptions.separate_shader_objects = true;
	commonOptions.vulkan_semantics = true;
	// Metal's clip-space Y is up, matching the HLSL convention that BSL shaders author in, so
	// no y-flip is needed when cross-compiling to MSL. Setting this to true here would emit
	// gl_Position.y = -gl_Position.y at the end of every vertex shader, producing upside-down
	// geometry on Metal (correct for Vulkan's y-down NDC, wrong for Metal's y-up). The engine
	// capability @c NdcYAxis is set to @c Up in SetUpCapabilities() to match, so the renderer
	// does not apply its own compensating flip.
	commonOptions.vertex.flip_vert_y = false;
	compiler.set_common_options(commonOptions);

	std::string mslSource;
	try
	{
		mslSource = compiler.compile();
	}
	catch (const spirv_cross::CompilerError& error)
	{
		bytecode->Messages += String("SPIRV-Cross MSL emission error: ") + error.what();
		if (bytecode->Instructions.Data)
			B3DFree(bytecode->Instructions.Data);
		bytecode->Instructions = DataBlob();
		return bytecode;
	}

	// Step 3: extract compute workgroup size from the SPIR-V entry point (SPIRV-Cross's MSL
	// output does not encode [[threads_per_threadgroup]] on the function signature; we
	// must propagate it on the C++ side).
	u32 workgroupSize[3] = { 1, 1, 1 };
	if (createInformation.Type == GPT_COMPUTE_PROGRAM)
	{
		const auto& entryPoints = compiler.get_entry_points_and_stages();
		if (!entryPoints.empty())
		{
			const auto& ep = entryPoints[0];
			spirv_cross::SPIREntryPoint spvEP = compiler.get_entry_point(ep.name, ep.execution_model);
			workgroupSize[0] = spvEP.workgroup_size.X;
			workgroupSize[1] = spvEP.workgroup_size.Y;
			workgroupSize[2] = spvEP.workgroup_size.Z;
		}
	}

	// Step 4: re-pack the bytecode blob as MSL source. Layout contract lives in
	// B3DMetalBytecodeLayout.h — WriteMetalBytecode returns a heap-owned DataBlob that
	// GpuProgramBytecode's destructor will free when the bytecode is finally released.
	if (bytecode->Instructions.Data)
		B3DFree(bytecode->Instructions.Data);
	bytecode->Instructions = DataBlob();

	if (mslSource.empty())
	{
		bytecode->Messages += "SPIRV-Cross produced empty MSL source.";
		return bytecode;
	}

	bytecode->Instructions = WriteMetalBytecode(createInformation.Type, workgroupSize, mslSource.data(), (u32)mslSource.size());

	return bytecode;
}

#endif // B3D_PLATFORM_MACOS
