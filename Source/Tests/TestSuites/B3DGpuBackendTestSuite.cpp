//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DGpuBackendTestSuite.h"
#include "Utility/B3DPushConstantShaderCompilationTest.h"
#include "Utility/B3DUnitTestGpuPipelines.h"
#include "Utility/B3DGpuBarrierTestUtility.h"
#include "CoreObject/B3DRenderThread.h"
#include "GpuBackend/B3DGpuBarriers.h"
#include "GpuBackend/Allocators/B3DGpuResource.h"
#include "GpuBackend/B3DGpuBackend.h"
#include "GpuBackend/B3DGpuBackendUtility.h"
#include "GpuBackend/B3DGpuCompletionTracker.h"
#include "GpuBackend/B3DGpuDevice.h"
#include "GpuBackend/B3DGpuDeviceCapabilities.h"
#include "GpuBackend/B3DGpuSplitBarrier.h"
#include "GpuBackend/B3DGpuCommandBuffer.h"
#include "GpuBackend/B3DGpuBarrierValidator.h"
#include "GpuBackend/B3DGpuParameterSet.h"
#include "GpuBackend/B3DGpuParameterSetPool.h"
#include "GpuBackend/B3DGpuPipelineState.h"
#include "GpuBackend/B3DGpuWorkContext.h"
#include "GpuBackend/B3DGpuPipelineParameterLayout.h"
#include "GpuBackend/B3DGpuResourceTracker.h"
#include "GpuBackend/B3DGpuProgram.h"
#include "GpuBackend/B3DGpuProgramParameterDescription.h"
#include "GpuBackend/B3DGpuPushConstants.h"
#include "GpuBackend/B3DRenderTexture.h"
#include "GpuBackend/B3DGpuTransientResourceAllocator.h"
#include "Renderer/B3DRenderer.h"
#include "Material/B3DShaderCompiler.h"
#include "Material/B3DShader.h"
#include "Material/B3DVariation.h"
#include "Material/B3DPass.h"
#include "Serialization/B3DBinarySerializer.h"
#include "FileSystem/B3DDataStream.h"
#include "String/B3DStringFormat.h"
#include "Utility/B3DResult.h"

using namespace b3d;
using namespace b3d::render;

namespace
{
	Result ValidatePushConstantWrite(u32 maximumPushConstantSize, u32 offsetInBytes, u32 sizeInBytes, const void* data)
	{
		if(sizeInBytes == 0)
			return Result::Success();

		if(maximumPushConstantSize < kMaxPushConstantSizeInBytes)
		{
			return Result::Fail("The active GPU backend does not support the guaranteed push-constant block.",
				ResultStatus::FailedInvalidInput, StringUtility::Format("Backend limit {0} bytes; required {1} bytes.", maximumPushConstantSize, kMaxPushConstantSizeInBytes));
		}

		if((offsetInBytes & 3u) != 0 || (sizeInBytes & 3u) != 0)
			return Result::Fail("Push-constant offsets and sizes must be aligned to four bytes.", ResultStatus::FailedInvalidInput);

		if(data == nullptr)
			return Result::Fail("Push-constant data cannot be null for a non-empty update.", ResultStatus::FailedInvalidInput);

		if(offsetInBytes > kMaxPushConstantSizeInBytes || sizeInBytes > kMaxPushConstantSizeInBytes - offsetInBytes)
		{
			return Result::Fail("Push-constant update is outside the guaranteed block.", ResultStatus::FailedInvalidInput,
				StringUtility::Format("Offset {0}, size {1}, block size {2} bytes.", offsetInBytes, sizeInBytes, kMaxPushConstantSizeInBytes));
		}

		return Result::Success();
	}

	class TrackedTestBuffer : public IGpuBufferResource
	{
	public:
		TrackedTestBuffer() = default;
	};

	class TrackedTestImage : public IGpuImageResource
	{
	public:
		TrackedTestImage(u32 faceCount, u32 mipLevelCount, GpuTextureAspectFlags aspects)
			: IGpuImageResource(faceCount, mipLevelCount, aspects)
		{ }
	};

	class AttachmentTestFramebuffer : public GpuFramebuffer
	{
	public:
		AttachmentTestFramebuffer(u32 width, u32 height, u32 layerCount)
			: GpuFramebuffer(width, height, layerCount)
		{ }

		using GpuFramebuffer::AddColorAttachment;
		using GpuFramebuffer::AddDepthStencilAttachment;
	};
}

GpuBackendTestSuite::GpuBackendTestSuite()
	: TestSuite("GpuBackendTestSuite")
{
#if B3D_BUILD_TYPE_DEVELOPMENT
	B3D_ADD_TEST(GpuBackendTestSuite::TestDrawAccessValidation)
#endif
	B3D_ADD_TEST(GpuBackendTestSuite::TestQueueStageFlags)
	B3D_ADD_TEST(GpuBackendTestSuite::TestTransientResourceAllocator)
	B3D_ADD_TEST(GpuBackendTestSuite::TestTransientResourceExecution)
	B3D_ADD_TEST(GpuBackendTestSuite::TestTransientResourcePlacement)
	B3D_ADD_TEST(GpuBackendTestSuite::TestFramebufferAttachmentUsage)
	B3D_ADD_TEST(GpuBackendTestSuite::TestResourceUseTracking)
	B3D_ADD_TEST(GpuBackendTestSuite::TestBarriers)
	B3D_ADD_TEST(GpuBackendTestSuite::TestSplitBarrierExecution)
	B3D_ADD_TEST(GpuBackendTestSuite::TestBarrierAcrossPasses)
	B3D_ADD_TEST(GpuBackendTestSuite::TestTransientAliasing)
#if B3D_BUILD_TYPE_DEVELOPMENT
	B3D_ADD_TEST(GpuBackendTestSuite::TestBarrierValidation)
	B3D_ADD_TEST(GpuBackendTestSuite::TestBarrierResourceValidation)
#endif
	B3D_ADD_TEST(GpuBackendTestSuite::TestPushConstantMetadata)
	B3D_ADD_TEST(GpuBackendTestSuite::TestPushConstantWrites)
	B3D_ADD_TEST(GpuBackendTestSuite::TestPushConstantSerialization)
	B3D_ADD_TEST(GpuBackendTestSuite::TestDynamicOffsetUniformBufferLayout)
	B3D_ADD_TEST(GpuBackendTestSuite::TestResourceLocations)
	// Shader compilation is performed on the host; console applications load cooked shaders.
#if !B3D_PLATFORM_PS5
	B3D_ADD_TEST(GpuBackendTestSuite::TestDynamicUniformBufferOffsets)
	B3D_ADD_TEST(GpuBackendTestSuite::TestHostPushConstantShaderCompilation)
	B3D_ADD_TEST(GpuBackendTestSuite::TestVulkanStorageBufferAccessReflection)
	B3D_ADD_TEST(GpuBackendTestSuite::TestHlslShaderModel66Compilation)
#endif
#if B3D_PLATFORM_MACOS
	B3D_ADD_TEST(GpuBackendTestSuite::TestMetalDynamicUniformBufferReflection)
#endif
}

void GpuBackendTestSuite::TestResourceLocations()
{
	GpuBackend& backend = GpuBackend::Instance();
	if(backend.GetDeviceCount() == 0)
		return;

	const TShared<GpuDevice> device = backend.GetDevice(0);

	constexpr u32 kElementCount = 64;
	const GpuBufferCreateInformation bufferInformation = GpuBufferCreateInformation::CreateStructuredStorage(sizeof(u32), kElementCount);
	const GpuMemoryRequirements bufferRequirements = device->GetMemoryRequirements(bufferInformation);
	B3D_TEST_ASSERT(bufferRequirements.MemoryType != GpuMemoryRequirements::kUnsupportedMemoryType)
	B3D_TEST_ASSERT(bufferRequirements.Size >= kElementCount * sizeof(u32))
	B3D_TEST_ASSERT(bufferRequirements.Alignment > 0)
	if(bufferRequirements.MemoryType == GpuMemoryRequirements::kUnsupportedMemoryType)
		return;

	IGpuAllocator& bufferAllocator = device->GetPersistentAllocator(bufferRequirements.MemoryType);

	// Pending allocation: memory is allocated from the allocator, and the buffer stays relocatable
	const TShared<render::GpuBuffer> pendingBuffer = device->CreateGpuBuffer(bufferInformation, GpuAllocation::CreatePending(bufferAllocator));
	B3D_TEST_ASSERT(pendingBuffer != nullptr && !pendingBuffer->HasFixedLocation())

	// Owned allocation: the buffer adopts memory allocated by the caller
	GpuAllocation ownedAllocation;
	B3D_TEST_ASSERT(bufferAllocator.TryAllocate(bufferRequirements.Size, (u32)bufferRequirements.Alignment, bufferRequirements.Kind, nullptr, ownedAllocation))
	B3D_TEST_ASSERT(ownedAllocation.IsOwned())
	if(!ownedAllocation.IsOwned())
		return;

	const TShared<render::GpuBuffer> ownedBuffer = device->CreateGpuBuffer(bufferInformation, ownedAllocation);
	B3D_TEST_ASSERT(ownedBuffer != nullptr && ownedBuffer->HasFixedLocation())

	// Non-owning allocation: the buffer binds to memory that stays owned by the caller
	GpuAllocation sharedAllocation;
	B3D_TEST_ASSERT(bufferAllocator.TryAllocate(bufferRequirements.Size, (u32)bufferRequirements.Alignment, bufferRequirements.Kind, nullptr, sharedAllocation))

	GpuAllocation nonOwningAllocation = sharedAllocation;
	nonOwningAllocation.Allocator = nullptr;
	B3D_TEST_ASSERT(nonOwningAllocation.HasMemory() && !nonOwningAllocation.IsOwned() && !nonOwningAllocation.IsPending())

	TShared<render::GpuBuffer> nonOwningBuffer = device->CreateGpuBuffer(bufferInformation, nonOwningAllocation);
	B3D_TEST_ASSERT(nonOwningBuffer != nullptr && nonOwningBuffer->HasFixedLocation())
	if(pendingBuffer == nullptr || ownedBuffer == nullptr || nonOwningBuffer == nullptr)
		return;

	// Round-trip data through the fixed location buffers, to verify the native resources are bound to usable memory
	u32 expected[kElementCount];
	for(u32 elementIndex = 0; elementIndex < kElementCount; elementIndex++)
		expected[elementIndex] = 0xC0DE0000 + elementIndex;

	const TShared<render::GpuBuffer> upload = device->CreateGpuBuffer(GpuBufferCreateInformation::CreateStagingWrite(sizeof(expected)));
	const TShared<render::GpuBuffer> readback = device->CreateGpuBuffer(GpuBufferCreateInformation::CreateStagingRead(sizeof(expected)));
	{
		const render::GpuBufferMappedScope mapping = upload->Map(GpuMapOption::Write);
		B3D_TEST_ASSERT(mapping.IsValid())
		if(!mapping.IsValid())
			return;

		memcpy(mapping.GetMappedMemory(), expected, sizeof(expected));
	}

	const TShared<GpuWorkContext> context = GpuWorkContext::Create(*device);
	const TShared<render::GpuCommandBufferPool> pool = GpuBarrierTestUtility::CreateCommandBufferPool(*device);
	const TShared<render::GpuCommandBuffer> commands = pool->Create(render::GpuCommandBufferCreateInformation::Create("Resource allocations"));

	// Each copy reads what the copy before it wrote. The host wrote the upload, which submission makes visible.
	const GpuAccessState transferWrite(GpuStageFlag::Transfer, GpuAccessFlag::Write);
	const GpuAccessState transferRead(GpuStageFlag::Transfer, GpuAccessFlag::Read);
	const TShared<render::GpuBuffer> copyChain[] = { upload, ownedBuffer, nonOwningBuffer, pendingBuffer, readback };
	for(u32 destinationIndex = 1; destinationIndex < 5; destinationIndex++)
	{
		TInlineArray<GpuBufferBarrier, 2> barriers;
		if(destinationIndex > 1)
			barriers.Add(GpuBufferBarrier(copyChain[destinationIndex - 1], transferWrite, transferRead));

		barriers.Add(GpuBufferBarrier(copyChain[destinationIndex], GpuAccessState(), transferWrite));
		commands->IssueBarriers(GpuBarriers(TArrayView<const GpuBufferBarrier>(barriers.Data(), barriers.Size())));
		commands->CopyBufferToBuffer(copyChain[destinationIndex - 1], copyChain[destinationIndex], 0, 0, sizeof(expected));
	}

	commands->IssueBarriers(GpuBarrierTestUtility::CreateReadbackBarrier(readback));
	context->SubmitCommandBuffer(commands);
	device->WaitUntilIdle();
	{
		const render::GpuBufferMappedScope mapping = readback->Map(GpuMapOption::Read);
		B3D_TEST_ASSERT(mapping.IsValid())
		if(mapping.IsValid())
			B3D_TEST_ASSERT(memcmp(mapping.GetMappedMemory(), expected, sizeof(expected)) == 0)
	}

	// Memory behind a non-owning allocation outlives the buffer, and is released by its owner
	nonOwningBuffer = nullptr;
	device->WaitUntilIdle();
	bufferAllocator.Free(sharedAllocation);

	// Textures follow the same rules
	TextureCreateInformation textureInformation;
	textureInformation.Name = "Resource allocation test";
	textureInformation.Width = 64;
	textureInformation.Height = 64;
	textureInformation.Format = PF_RGBA8;
	textureInformation.Usage = TextureUsageFlag::Default;

	const GpuMemoryRequirements textureRequirements = device->GetMemoryRequirements(textureInformation);
	B3D_TEST_ASSERT(textureRequirements.MemoryType != GpuMemoryRequirements::kUnsupportedMemoryType)
	B3D_TEST_ASSERT(textureRequirements.Size >= 64 * 64 * 4)
	if(textureRequirements.MemoryType == GpuMemoryRequirements::kUnsupportedMemoryType)
		return;

	IGpuAllocator& textureAllocator = device->GetPersistentAllocator(textureRequirements.MemoryType);

	GpuAllocation textureAllocation;
	B3D_TEST_ASSERT(textureAllocator.TryAllocate(textureRequirements.Size, (u32)textureRequirements.Alignment, textureRequirements.Kind, nullptr, textureAllocation))

	TextureCreateInformation cpuTextureInformation = textureInformation;
	cpuTextureInformation.Usage = TextureUsageFlag::StoreOnCPUWithGPUAccess;

	const GpuMemoryRequirements cpuTextureRequirements = device->GetMemoryRequirements(cpuTextureInformation);
	IGpuAllocator& cpuTextureAllocator = device->GetPersistentAllocator(cpuTextureRequirements.MemoryType);

	GpuAllocation cpuTextureAllocation;
	B3D_TEST_ASSERT(cpuTextureAllocator.TryAllocate(cpuTextureRequirements.Size, (u32)cpuTextureRequirements.Alignment, cpuTextureRequirements.Kind, nullptr, cpuTextureAllocation))

	bool pendingTextureCreated = false;
	bool ownedTextureCreated = false;
	bool cpuTextureCreated = false;
	GetRenderThread().PostCommand([&]()
	{
		const TShared<render::Texture> pendingTexture = device->CreateTexture(textureInformation, GpuAllocation::CreatePending(textureAllocator));
		pendingTextureCreated = pendingTexture != nullptr && !pendingTexture->HasFixedLocation();

		const TShared<render::Texture> ownedTexture = device->CreateTexture(textureInformation, textureAllocation);
		ownedTextureCreated = ownedTexture != nullptr && ownedTexture->HasFixedLocation();

		// CPU accessible textures can be created at a fixed location
		const TShared<render::Texture> cpuTexture = device->CreateTexture(cpuTextureInformation, cpuTextureAllocation);
		cpuTextureCreated = cpuTexture != nullptr && cpuTexture->HasFixedLocation();
	}, "GpuBackendTestSuite::TestResourceLocations", true);

	B3D_TEST_ASSERT(pendingTextureCreated)
	B3D_TEST_ASSERT(ownedTextureCreated)
	B3D_TEST_ASSERT(cpuTextureCreated)

	// Invalid allocations are rejected
	{
		LoggingScope logs(*this);
		logs.ExpectError("Cannot create a GPU resource at an empty allocation.");
		B3D_TEST_ASSERT(device->CreateGpuBuffer(bufferInformation, GpuAllocation()) == nullptr)
	}

	{
		LoggingScope logs(*this);
		logs.ExpectError("Only a GPU resource created at a fixed memory location can be aliased.");
		B3D_TEST_ASSERT(device->CreateGpuBuffer(bufferInformation, GpuAllocation::CreatePending(bufferAllocator), GpuObjectCreateFlag::Aliased) == nullptr)
	}

	// CPU accessible and suballocated buffers can be created at a fixed location
	{
		const GpuBufferCreateInformation uniformInformation = GpuBufferCreateInformation::CreateUniform(16, GpuBufferFlag::StoreOnCPUWithGPUAccess, 4);
		const GpuMemoryRequirements uniformRequirements = device->GetMemoryRequirements(uniformInformation);
		IGpuAllocator& uniformAllocator = device->GetPersistentAllocator(uniformRequirements.MemoryType);

		GpuAllocation uniformAllocation;
		B3D_TEST_ASSERT(uniformAllocator.TryAllocate(uniformRequirements.Size, (u32)uniformRequirements.Alignment, uniformRequirements.Kind, nullptr, uniformAllocation))

		const TShared<render::GpuBuffer> uniformBuffer = device->CreateGpuBuffer(uniformInformation, uniformAllocation);
		B3D_TEST_ASSERT(uniformBuffer != nullptr && uniformBuffer->HasFixedLocation())
		if(uniformBuffer != nullptr)
		{
			const render::GpuBufferMappedScope mapping = uniformBuffer->Map(GpuMapOption::Write);
			B3D_TEST_ASSERT(mapping.IsValid())
		}
	}
}

void GpuBackendTestSuite::TestDynamicOffsetUniformBufferLayout()
{
	GpuBackend& backend = GpuBackend::Instance();
	if(backend.GetDeviceCount() == 0)
		return;

	const TShared<GpuDevice> device = backend.GetDevice(0);

	GpuUniformBufferInformation staticBuffer;
	staticBuffer.Name = "StaticData";
	staticBuffer.Set = 0;
	staticBuffer.Slot = device->GetParameterSlot(ShaderRegisterClass::ConstantBuffer, 0);
	staticBuffer.Size = 4;
	staticBuffer.Stages = GpuProgramStageBit::Vertex;
	staticBuffer.IsShareable = true;

	GpuUniformBufferInformation dynamicBuffer = staticBuffer;
	dynamicBuffer.Name = "PerObject";
	dynamicBuffer.Slot = device->GetParameterSlot(ShaderRegisterClass::ConstantBuffer, 1);
	dynamicBuffer.UsesDynamicOffset = true;

	GpuProgramParameterDescription description;
	description.UniformBuffers[staticBuffer.Name] = staticBuffer;
	description.UniformBuffers[dynamicBuffer.Name] = dynamicBuffer;

	const TShared<GpuPipelineParameterSetLayout> layout = device->CreateGpuPipelineParameterSetLayout(description);
	B3D_TEST_ASSERT(layout != nullptr)
	if(layout == nullptr)
		return;

	B3D_TEST_ASSERT(layout->GetDynamicOffsetCount() == 1)
	B3D_TEST_ASSERT(layout->GetDynamicOffsetIndex("PerObject") == 0)
	B3D_TEST_ASSERT(layout->GetDynamicOffsetIndex("StaticData") == ~0u)

	const UniformInformation* dynamicInformation = layout->TryGetUniformInformation("PerObject");
	B3D_TEST_ASSERT(dynamicInformation != nullptr && dynamicInformation->DynamicOffsetIndex == 0)

	const UniformInformation* staticInformation = layout->TryGetUniformInformation("StaticData");
	B3D_TEST_ASSERT(staticInformation != nullptr && staticInformation->DynamicOffsetIndex == ~0u)

	// Stages sharing a buffer must agree on its dynamic-offset declaration
	GpuProgramParameterDescription vertex;
	vertex.UniformBuffers[dynamicBuffer.Name] = dynamicBuffer;

	GpuProgramParameterDescription fragment;
	fragment.UniformBuffers[dynamicBuffer.Name] = dynamicBuffer;
	fragment.UniformBuffers[dynamicBuffer.Name].UsesDynamicOffset = false;

	GpuProgramParameterDescription combined;
	B3D_TEST_ASSERT(combined.TryCombine(vertex, GpuProgramStageBit::Vertex).IsSuccessful())
	B3D_TEST_ASSERT(!combined.TryCombine(fragment, GpuProgramStageBit::Fragment).IsSuccessful())
}

#if !B3D_PLATFORM_PS5
void GpuBackendTestSuite::TestDynamicUniformBufferOffsets()
{
	GpuBackend& backend = GpuBackend::Instance();
	const String backendName = backend.GetBackendName();
	if(backend.GetDeviceCount() == 0 || (backendName != "bsfD3D12GpuBackend" && backendName != "bsfVulkanGpuBackend"))
		return;

	GpuDevice* const device = backend.GetDevice(0).get();
	const String language = backendName == "bsfD3D12GpuBackend" ? "hlsl" : "vksl";

	const TShared<IShaderCompiler> compiler = ShaderCompilers::Instance().GetCompiler("bsl");
	B3D_TEST_ASSERT(compiler != nullptr)
	if(compiler == nullptr)
		return;

	const String source = R"(
shader DynamicUniformBufferOffsets
{
	code
	{
		cbuffer StaticParameters : register(c0, space0) { uint StaticValue; };
		[dynamicOffset] cbuffer DynamicParameters : register(c3, space0) { uint DynamicValue; };
		[dynamicOffset] cbuffer OtherParameters : register(c1, space1) { uint OtherValue; };
		[pushConstant] cbuffer Constants { uint OutputIndex; };
		RWStructuredBuffer<uint> Output;
		[numthreads(1, 1, 1)]
		void csmain() { Output[OutputIndex] = StaticValue + 100 * DynamicValue + 10000 * OtherValue; }
	};
};
)";
	TShared<b3d::Shader> shader;
	const ShaderCompilerResult result = compiler->Compile("DynamicUniformBufferOffsets", source, {}, { language }, true, shader);
	B3D_TEST_ASSERT_MSG(result.ErrorMessage.empty(), result.ErrorMessage)
	B3D_TEST_ASSERT(shader != nullptr && shader->GetVariations().size() == 1)
	if(!result.ErrorMessage.empty() || shader == nullptr || shader->GetVariations().size() != 1)
		return;

	const TShared<b3d::Variation>& variation = shader->GetVariations().front();
	B3D_TEST_ASSERT(variation->GetPassCount() == 1)
	if(variation->GetPassCount() != 1)
		return;

	GpuComputePipelineStateCreateInformation pipelineInformation;
	pipelineInformation.Program = device->CreateGpuProgram(variation->GetPass(0)->GetGpuProgramCreateInformation(GPT_COMPUTE_PROGRAM));
	const TShared<GpuComputePipelineState> pipeline = device->CreateGpuComputePipelineState(pipelineInformation);
	const TShared<GpuComputePipelineState> alternatePipeline = device->CreateGpuComputePipelineState(pipelineInformation);
	B3D_TEST_ASSERT(pipeline != nullptr && alternatePipeline != nullptr)
	if(pipeline == nullptr || alternatePipeline == nullptr)
		return;

	const TShared<GpuWorkContext> context = GpuWorkContext::Create(*device);
	const TShared<render::GpuBuffer> values = device->CreateGpuBuffer(GpuBufferCreateInformation::CreateUniform(16, GpuBufferFlag::StoreOnCPUWithGPUAccess, 4));
	const u32 stride = values->GetSuballocationSize();
	{
		const render::GpuBufferMappedScope mapping = values->Map(GpuMapOption::Write);
		B3D_TEST_ASSERT(mapping.IsValid())
		if(!mapping.IsValid())
			return;

		for(u32 valueIndex = 0; valueIndex < 4; valueIndex++)
			*reinterpret_cast<u32*>(static_cast<u8*>(mapping.GetMappedMemory()) + valueIndex * stride) = 1 + 10 * valueIndex;
	}

	const u32 expected[] = { 12111, 13111, 213111, 210111, 212131, 212111 };
	const TShared<render::GpuBuffer> output = device->CreateGpuBuffer(GpuBufferCreateInformation::CreateStructuredStorage(sizeof(u32), 6, GpuBufferFlag::StoreOnGPU | GpuBufferFlag::AllowUnorderedAccessOnTheGPU));
	const TShared<render::GpuBuffer> readback = device->CreateGpuBuffer(GpuBufferCreateInformation::CreateStagingRead(sizeof(expected)));
	const TShared<GpuPipelineParameterSetLayout> firstLayout = pipeline->GetParameterLayout()->GetSet(0);
	const TShared<GpuPipelineParameterSetLayout> secondLayout = pipeline->GetParameterLayout()->GetSet(1);
	const TShared<render::GpuParameterSet> firstSet = context->GetParameterSetPool().Create(firstLayout, 0);
	const TShared<render::GpuParameterSet> secondSet = context->GetParameterSetPool().Create(secondLayout, 1);
	firstSet->SetUniformBuffer("DynamicParameters", values, 0, 2 * stride);
	firstSet->SetStorageBuffer("Output", output);
	secondSet->SetUniformBuffer("OtherParameters", values);
	const TShared<render::GpuCommandBufferPool> pool = GpuBarrierTestUtility::CreateCommandBufferPool(*device);

	// The dispatches write different elements of the output, so they need no barriers between them. The host wrote the values, which
	// submission makes visible.
	const GpuAccessState computeWrite(GpuStageFlag::ComputeShaderNonUniform, GpuAccessFlag::Write);
	const GpuAccessState transferWrite(GpuStageFlag::Transfer, GpuAccessFlag::Write);
	const GpuAccessState transferRead(GpuStageFlag::Transfer, GpuAccessFlag::Read);
	const GpuAccessState hostRead(GpuStageFlag::Host, GpuAccessFlag::Read);

	// Reusing parameter sets with a fresh command buffer restores their initial offsets.
	for(u32 iteration = 0; iteration < 2; iteration++)
	{
		firstSet->SetUniformBuffer("StaticParameters", values, 0, stride);
		const TShared<render::GpuCommandBuffer> commands = pool->Create(render::GpuCommandBufferCreateInformation::Create("Dynamic uniform offsets"));
		commands->IssueBarriers(GpuBufferBarrier(output, iteration == 0 ? GpuAccessState() : transferRead, computeWrite));
		commands->SetGpuComputePipelineState(pipeline);
		commands->SetGpuParameterSet(firstSet);
		commands->SetGpuParameterSet(secondSet);
		for(u32 outputIndex = 0; outputIndex < 6; outputIndex++)
		{
			if(outputIndex == 1)
				commands->SetDynamicBufferOffset(0, firstLayout->GetDynamicOffsetIndex("DynamicParameters"), 3 * stride);
			else if(outputIndex == 2)
				commands->SetDynamicBufferOffset(1, secondLayout->GetDynamicOffsetIndex("OtherParameters"), 2 * stride);
			else if(outputIndex == 3)
			{
				commands->SetGpuComputePipelineState(alternatePipeline);
				commands->SetDynamicBufferOffset(0, firstLayout->GetDynamicOffsetIndex("DynamicParameters"), 0);
			}
			else if(outputIndex >= 4)
			{
				firstSet->SetUniformBuffer("StaticParameters", values, 0, outputIndex == 4 ? 3 * stride : stride);
				commands->SetGpuParameterSet(firstSet);
			}

			commands->SetPushConstants(0, sizeof(outputIndex), &outputIndex);
			commands->DispatchCompute(1);
		}

		const GpuBufferBarrier copyBarriers[] = { GpuBufferBarrier(output, computeWrite, transferRead),
			GpuBufferBarrier(readback, iteration == 0 ? GpuAccessState() : hostRead, transferWrite) };
		commands->IssueBarriers(GpuBarriers(TArrayView<const GpuBufferBarrier>(copyBarriers, 2)));
		commands->CopyBufferToBuffer(output, readback, 0, 0, sizeof(expected));
		commands->IssueBarriers(GpuBufferBarrier(readback, transferWrite, hostRead));
		context->SubmitCommandBuffer(commands);
		device->WaitUntilIdle();

		const render::GpuBufferMappedScope mapping = readback->Map(GpuMapOption::Read);
		B3D_TEST_ASSERT(mapping.IsValid())
		if(!mapping.IsValid())
			return;

		const u32* actual = static_cast<const u32*>(mapping.GetMappedMemory());
		for(u32 outputIndex = 0; outputIndex < 6; outputIndex++)
			B3D_TEST_ASSERT(actual[outputIndex] == expected[outputIndex])
	}
}
#endif

#if B3D_PLATFORM_MACOS
void GpuBackendTestSuite::TestMetalDynamicUniformBufferReflection()
{
	const TShared<IShaderCompiler> compiler = ShaderCompilers::Instance().GetCompiler("bsl");
	B3D_TEST_ASSERT_MSG(compiler != nullptr, "Metal reflection tests require the BSL compiler.")
	if(compiler == nullptr)
		return;

	const String shaderName = "MetalDynamicUniformBufferReflection";
	const String source = R"(
shader MetalDynamicUniformBufferReflection
{
	code
	{
		[dynamicOffset]
		cbuffer PerObject
		{
			float4x4 gTransform;
		};

		[dynamicOffset]
		cbuffer Tint
		{
			float4 gColor;
		};

		SamplerState gSampler;
		Texture2D gTexture;

		struct VStoFS
		{
			float4 position : SV_Position;
			float2 uv : TEXCOORD0;
		};

		VStoFS vsmain(float3 position : POSITION, float2 uv : TEXCOORD0)
		{
			VStoFS output;
			output.position = mul(gTransform, float4(position, 1.0f));
			output.uv = uv;
			return output;
		}

		float4 fsmain(VStoFS input) : SV_Target0
		{
			return gTexture.Sample(gSampler, input.uv) * gColor;
		}
	};
};
)";

	TShared<Shader> shader;
	const ShaderCompilerResult compileResult = compiler->Compile(shaderName, source, {}, { "msl" }, true, shader);
	B3D_TEST_ASSERT_MSG(compileResult.ErrorMessage.empty(), compileResult.ErrorMessage)
	B3D_TEST_ASSERT(shader != nullptr && shader->GetVariations().size() == 1)
	if(!compileResult.ErrorMessage.empty() || shader == nullptr || shader->GetVariations().size() != 1)
		return;

	const TShared<Variation>& variation = shader->GetVariations().front();
	B3D_TEST_ASSERT(variation != nullptr && variation->GetPassCount() == 1 && variation->GetPass(0) != nullptr)
	if(variation == nullptr || variation->GetPassCount() != 1 || variation->GetPass(0) == nullptr)
		return;

	const GpuProgramType stages[] = { GPT_VERTEX_PROGRAM, GPT_FRAGMENT_PROGRAM };
	for(GpuProgramType stage : stages)
	{
		const GpuProgramCreateInformation& program = variation->GetPass(0)->GetGpuProgramCreateInformation(stage);
		B3D_TEST_ASSERT(program.Bytecode != nullptr)
		if(program.Bytecode == nullptr)
			continue;

		B3D_TEST_ASSERT_MSG(program.Bytecode->Instructions.Data != nullptr, program.Bytecode->Messages)
		B3D_TEST_ASSERT(program.Bytecode->ParameterDescription != nullptr && program.Bytecode->ResourceTableLayout != nullptr)
		if(program.Bytecode->Instructions.Data == nullptr || program.Bytecode->ParameterDescription == nullptr || program.Bytecode->ResourceTableLayout == nullptr)
			continue;

		// Each stage reflects only the resources it reads, but argument-table indices follow the declared (set, slot)
		// order shared by every stage: PerObject is declared first and takes index 8 in the vertex stage, so Tint keeps
		// index 9 in the fragment stage even though PerObject is absent there
		const GpuProgramParameterDescription& parameterDescription = *program.Bytecode->ParameterDescription;
		const GpuResourceTableLayout& tableLayout = *program.Bytecode->ResourceTableLayout;
		const char* expectedBuffer = stage == GPT_VERTEX_PROGRAM ? "PerObject" : "Tint";
		const char* expectedMember = stage == GPT_VERTEX_PROGRAM ? "gTransform" : "gColor";
		const u32 expectedIndex = stage == GPT_VERTEX_PROGRAM ? 8 : 9;
		B3D_TEST_ASSERT(parameterDescription.UniformBuffers.size() == 1)
		B3D_TEST_ASSERT_MSG(parameterDescription.UniformBuffers.find(expectedBuffer) != parameterDescription.UniformBuffers.end(), expectedBuffer)
		B3D_TEST_ASSERT_MSG(parameterDescription.UniformBufferMembers.find(expectedMember) != parameterDescription.UniformBufferMembers.end(), expectedMember)
		B3D_TEST_ASSERT(!tableLayout.IsEmpty())
		if(tableLayout.IsEmpty())
			continue;

		// The uniform buffer is flagged and listed directly in the root table at its argument-table index rather than
		// as an argument-buffer member of the set's table
		for(const auto& [name, uniformBuffer] : parameterDescription.UniformBuffers)
		{
			B3D_TEST_ASSERT_MSG(uniformBuffer.UsesDynamicOffset, name)

			const GpuDescriptorTableEntry* rootEntry = tableLayout.FindRootResourceEntry(GpuParameterType::UniformBuffer, uniformBuffer.Set, uniformBuffer.Slot);

			B3D_TEST_ASSERT_MSG(rootEntry != nullptr && rootEntry->BindingIndex == expectedIndex, name)
		}

		for(u32 tableIndex = 1; tableIndex < (u32)tableLayout.Tables.size(); tableIndex++)
		{
			for(const GpuDescriptorTableEntry& entry : tableLayout.GetEntries(tableLayout.Tables[tableIndex]))
				B3D_TEST_ASSERT(entry.Kind != GpuDescriptorEntryKind::Resource || entry.Type != GpuParameterType::UniformBuffer)
		}

		// The texture still lives in the fragment stage's argument buffer
		bool foundTexture = false;
		for(u32 tableIndex = 1; tableIndex < (u32)tableLayout.Tables.size(); tableIndex++)
		{
			for(const GpuDescriptorTableEntry& entry : tableLayout.GetEntries(tableLayout.Tables[tableIndex]))
				foundTexture |= entry.Kind == GpuDescriptorEntryKind::Resource && entry.Type == GpuParameterType::SampledTexture;
		}

		if(stage == GPT_FRAGMENT_PROGRAM)
			B3D_TEST_ASSERT(foundTexture)
	}
}
#endif

void GpuBackendTestSuite::TestPushConstantMetadata()
{
	GpuProgramParameterDescription vertex;
	vertex.PushConstantBufferSize = 4;
	GpuProgramParameterDescription fragment;
	fragment.PushConstantBufferSize = 12;

	GpuProgramParameterDescription combined;
	B3D_TEST_ASSERT(combined.TryCombine(vertex, GpuProgramStageBit::Vertex).IsSuccessful())
	B3D_TEST_ASSERT(combined.TryCombine(fragment, GpuProgramStageBit::Fragment).IsSuccessful())
	B3D_TEST_ASSERT(combined.PushConstantBufferSize == 12)

	TInlineArray<GpuProgramParameterDescription, 4> perSetDescriptions;
	combined.SplitBySet(perSetDescriptions);
	B3D_TEST_ASSERT(perSetDescriptions.Size() == 0)
}

void GpuBackendTestSuite::TestPushConstantWrites()
{
	const Array<u32, 4> values = { 1, 2, 3, 4 };

	B3D_TEST_ASSERT(ValidatePushConstantWrite(16, 0, 16, values.data()).IsSuccessful())
	B3D_TEST_ASSERT(ValidatePushConstantWrite(16, 4, 8, values.data()).IsSuccessful())
	B3D_TEST_ASSERT(ValidatePushConstantWrite(0, 16, 0, nullptr).IsSuccessful())
	B3D_TEST_ASSERT(!ValidatePushConstantWrite(0, 0, 4, values.data()).IsSuccessful())
	B3D_TEST_ASSERT(!ValidatePushConstantWrite(8, 0, 4, values.data()).IsSuccessful())
	B3D_TEST_ASSERT(!ValidatePushConstantWrite(16, 2, 4, values.data()).IsSuccessful())
	B3D_TEST_ASSERT(!ValidatePushConstantWrite(16, 12, 8, values.data()).IsSuccessful())
	B3D_TEST_ASSERT(!ValidatePushConstantWrite(16, 0, 4, nullptr).IsSuccessful())

	GpuPushConstantPayload payload;
	payload.Write(4, 8, values.data() + 1);
	B3D_TEST_ASSERT(payload.Values[0] == 0)
	B3D_TEST_ASSERT(payload.Values[1] == 2)
	B3D_TEST_ASSERT(payload.Values[2] == 3)
	B3D_TEST_ASSERT(payload.Values[3] == 0)

	const u32 lastValue = 9;
	payload.Write(12, 4, &lastValue);
	B3D_TEST_ASSERT(payload.Values[3] == 9)
	payload.Clear();
	const Array<u32, 4> emptyValues{};
	B3D_TEST_ASSERT(payload.Values == emptyValues)
}

void GpuBackendTestSuite::TestPushConstantSerialization()
{
	GpuProgramCreateInformation createInformation;
	createInformation.Name = "PushConstantSerialization";
	createInformation.Type = GPT_COMPUTE_PROGRAM;
	createInformation.ShaderReflection = B3DMakeShared<ShaderReflection>();
	createInformation.ShaderReflection->EntryPoints[createInformation.EntryPoint].Type = createInformation.Type;
	createInformation.ShaderReflection->EntryPoints[createInformation.EntryPoint].PushConstantBufferSize = 12;
	createInformation.Bytecode = B3DMakeShared<GpuProgramBytecode>();
	createInformation.Bytecode->ParameterDescription = B3DMakeShared<GpuProgramParameterDescription>();
	createInformation.Bytecode->ParameterDescription->PushConstantBufferSize = 12;

	const TShared<MemoryDataStream> stream = B3DMakeShared<MemoryDataStream>();
	BinarySerializer serializer;
	serializer.Encode(&createInformation, stream);
	stream->Seek(0);

	const TShared<GpuProgramCreateInformation> decoded = B3DRTTICast<GpuProgramCreateInformation>(
		serializer.Decode(stream, (u32)stream->Size()));
	B3D_TEST_ASSERT(decoded != nullptr)
	if(decoded == nullptr)
		return;

	B3D_TEST_ASSERT(decoded->GetEntryPointReflection().PushConstantBufferSize == 12)
	B3D_TEST_ASSERT(decoded->Bytecode != nullptr)
	if(decoded->Bytecode == nullptr)
		return;

	B3D_TEST_ASSERT(decoded->Bytecode->ParameterDescription != nullptr)
	if(decoded->Bytecode->ParameterDescription == nullptr)
		return;

	B3D_TEST_ASSERT(decoded->Bytecode->ParameterDescription->PushConstantBufferSize == 12)
}

#if !B3D_PLATFORM_PS5
void GpuBackendTestSuite::TestHostPushConstantShaderCompilation()
{
	TestPushConstantShaderCompilation(*this, "vksl");
#if B3D_PLATFORM_MACOS
	TestPushConstantShaderCompilation(*this, "msl");
#endif
#if B3D_PLATFORM_WIN32
	TestPushConstantShaderCompilation(*this, "hlsl");
#endif
}
#endif

void GpuBackendTestSuite::TestVulkanStorageBufferAccessReflection()
{
	const TShared<IGpuBytecodeCompiler> compiler = ShaderCompilers::Instance().GetBytecodeCompiler("vksl");
	B3D_TEST_ASSERT(compiler != nullptr)
	if(compiler == nullptr)
		return;

	GpuProgramCreateInformation createInformation;
	createInformation.Name = "StorageBufferAccessReflection";
	createInformation.Language = "vksl";
	createInformation.Type = GPT_COMPUTE_PROGRAM;
	createInformation.EntryPoint = "main";
	createInformation.Source = R"(
#version 450
layout(local_size_x = 1) in;
layout(set = 0, binding = 0, std430) readonly buffer ReadOnlyBlock { vec4 Values[]; } readOnlyData;
layout(set = 0, binding = 1, std430) buffer WritableBlock { vec4 Values[]; } writableData;
layout(set = 0, binding = 2, std430) writeonly buffer WriteOnlyBlock { vec4 Values[]; } writeOnlyData;
layout(set = 0, binding = 3, std430) buffer ReadOnlyMemberBlock { readonly vec4 Values[]; } readOnlyMemberData;
layout(set = 0, binding = 4, std430) buffer MixedMemberBlock { readonly vec4 Input; vec4 Output; } mixedMemberData;
void main()
{
	writeOnlyData.Values[0] = readOnlyData.Values[0] + writableData.Values[0] + readOnlyMemberData.Values[0];
	mixedMemberData.Output = mixedMemberData.Input;
}
)";

	const TShared<GpuProgramBytecode> bytecode = compiler->CompileBytecode(createInformation);
	B3D_TEST_ASSERT(bytecode != nullptr)
	if(bytecode == nullptr)
		return;

	B3D_TEST_ASSERT_MSG(bytecode->Instructions.Data != nullptr, bytecode->Messages)
	B3D_TEST_ASSERT(bytecode->ParameterDescription != nullptr)
	if(bytecode->ParameterDescription == nullptr)
		return;

	B3D_TEST_ASSERT(bytecode->ParameterDescription->Buffers.size() == 5)
	for(const auto& [name, buffer] : bytecode->ParameterDescription->Buffers)
	{
		const bool isReadOnly = buffer.Slot == 0 || buffer.Slot == 3;
		B3D_TEST_ASSERT_MSG(buffer.Type == (isReadOnly ? GPOT_STRUCTURED_BUFFER : GPOT_RWSTRUCTURED_BUFFER), name)
	}
}

void GpuBackendTestSuite::TestHlslShaderModel66Compilation()
{
#if B3D_PLATFORM_WIN32
	const TShared<IGpuBytecodeCompiler> compiler = ShaderCompilers::Instance().GetBytecodeCompiler("hlsl");
	B3D_TEST_ASSERT(compiler != nullptr)

	auto compile = [&](const String& name, GpuProgramType type, const String& source)
	{
		GpuProgramCreateInformation createInformation;
		createInformation.Name = name;
		createInformation.Source = source;
		createInformation.EntryPoint = "main";
		createInformation.Language = "hlsl";
		createInformation.Type = type;

		const TShared<GpuProgramBytecode> bytecode = compiler->CompileBytecode(createInformation);
		B3D_TEST_ASSERT(bytecode != nullptr)
		B3D_TEST_ASSERT(bytecode->Instructions.Data != nullptr)
		B3D_TEST_ASSERT(bytecode->Instructions.Size != 0)
		B3D_TEST_ASSERT(bytecode->CompilerId == "HLSL_DXC")
		B3D_TEST_ASSERT(compiler->IsUpToDate(*bytecode))

		bool containsDxilPart = false;
		for(u32 offset = 0; offset + 4 <= bytecode->Instructions.Size; offset++)
		{
			const u8* marker = bytecode->Instructions.Data + offset;
			if(marker[0] == 'D' && marker[1] == 'X' && marker[2] == 'I' && marker[3] == 'L')
			{
				containsDxilPart = true;
				break;
			}
		}
		B3D_TEST_ASSERT(containsDxilPart)

		return bytecode;
	};

	const String vertexSource = R"(
cbuffer FrameData : register(b2, space1)
{
	column_major float4x4 Transform;
	float Exponent;
};

struct VertexInput
{
	float3 Position : POSITION0;
	float2 Uv : TEXCOORD0;
};

struct VertexOutput
{
	float4 Position : SV_Position;
	float Value : TEXCOORD0;
};

VertexOutput main(VertexInput input)
{
	VertexOutput output;
	output.Position = mul(Transform, float4(input.Position, 1.0f));
	output.Value = pow(abs(input.Uv.x + 1.0f), Exponent);
	return output;
}
)";
	const TShared<GpuProgramBytecode> vertexBytecode = compile("HlslVertex", GPT_VERTEX_PROGRAM, vertexSource);
	B3D_TEST_ASSERT(vertexBytecode->VertexInput.size() == 2)
	B3D_TEST_ASSERT(vertexBytecode->ParameterDescription != nullptr)
	B3D_TEST_ASSERT(vertexBytecode->ParameterDescription->UniformBuffers.find("FrameData") != vertexBytecode->ParameterDescription->UniformBuffers.end())
	const auto transformMember = vertexBytecode->ParameterDescription->UniformBufferMembers.find("Transform");
	const auto exponentMember = vertexBytecode->ParameterDescription->UniformBufferMembers.find("Exponent");
	B3D_TEST_ASSERT(transformMember != vertexBytecode->ParameterDescription->UniformBufferMembers.end())
	B3D_TEST_ASSERT(exponentMember != vertexBytecode->ParameterDescription->UniformBufferMembers.end())
	B3D_TEST_ASSERT(transformMember->second.GpuOffset == 0)
	B3D_TEST_ASSERT(exponentMember->second.GpuOffset == 16)

	const String fragmentSource = R"(
Texture2D<float4> InputTextures[3] : register(t2, space1);
SamplerState InputSampler : register(s1, space1);

float4 main(float2 uv : TEXCOORD0) : SV_Target0
{
	return InputTextures[1].Sample(InputSampler, uv);
}
)";
	const TShared<GpuProgramBytecode> fragmentBytecode = compile("HlslFragment", GPT_FRAGMENT_PROGRAM, fragmentSource);
	B3D_TEST_ASSERT(fragmentBytecode->ParameterDescription != nullptr)
	B3D_TEST_ASSERT(fragmentBytecode->ParameterDescription->SampledTextures.find("InputTextures") != fragmentBytecode->ParameterDescription->SampledTextures.end())
	B3D_TEST_ASSERT(fragmentBytecode->ParameterDescription->Samplers.find("InputSampler") != fragmentBytecode->ParameterDescription->Samplers.end())
	B3D_TEST_ASSERT(fragmentBytecode->ResourceTableLayout != nullptr)
	B3D_TEST_ASSERT(fragmentBytecode->ResourceTableLayout->Tables.size() == 2)
	B3D_TEST_ASSERT(fragmentBytecode->ResourceTableLayout->Tables[1].Set == 1)
	const TArrayView<const GpuDescriptorTableEntry> fragmentEntries = fragmentBytecode->ResourceTableLayout->GetEntries(
		fragmentBytecode->ResourceTableLayout->Tables[1]);
	B3D_TEST_ASSERT(fragmentEntries.Size() == 2)
	B3D_TEST_ASSERT(fragmentEntries[1].DescriptorCount == 3)

	const String geometrySource = R"(
struct Vertex
{
	float4 Position : SV_Position;
};

[maxvertexcount(3)]
void main(triangle Vertex input[3], inout TriangleStream<Vertex> outputStream)
{
	outputStream.Append(input[0]);
	outputStream.Append(input[1]);
	outputStream.Append(input[2]);
}
)";
	compile("HlslGeometry", GPT_GEOMETRY_PROGRAM, geometrySource);

	const String hullSource = R"(
struct ControlPoint
{
	float4 Position : POSITION0;
};

struct PatchConstants
{
	float Edges[3] : SV_TessFactor;
	float Inside : SV_InsideTessFactor;
};

PatchConstants GetPatchConstants(InputPatch<ControlPoint, 3> input)
{
	PatchConstants output;
	output.Edges[0] = 1.0f;
	output.Edges[1] = 1.0f;
	output.Edges[2] = 1.0f;
	output.Inside = 1.0f;
	return output;
}

[domain("tri")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("GetPatchConstants")]
ControlPoint main(InputPatch<ControlPoint, 3> input, uint controlPointId : SV_OutputControlPointID)
{
	return input[controlPointId];
}
)";
	compile("HlslHull", GPT_HULL_PROGRAM, hullSource);

	const String domainSource = R"(
struct ControlPoint
{
	float4 Position : POSITION0;
};

struct PatchConstants
{
	float Edges[3] : SV_TessFactor;
	float Inside : SV_InsideTessFactor;
};

[domain("tri")]
float4 main(PatchConstants constants, float3 barycentric : SV_DomainLocation,
	const OutputPatch<ControlPoint, 3> input) : SV_Position
{
	return input[0].Position * barycentric.x + input[1].Position * barycentric.y +
		input[2].Position * barycentric.z + constants.Inside * 0.0f;
}
)";
	compile("HlslDomain", GPT_DOMAIN_PROGRAM, domainSource);

	const String computeSource = R"(
RWStructuredBuffer<uint> OutputData : register(u0, space2);

[numthreads(8, 4, 2)]
void main(uint3 dispatchThreadId : SV_DispatchThreadID)
{
	OutputData[dispatchThreadId.x] = dispatchThreadId.y;
}
)";
	const TShared<GpuProgramBytecode> computeBytecode = compile("HlslCompute", GPT_COMPUTE_PROGRAM, computeSource);
	B3D_TEST_ASSERT(computeBytecode->ThreadGroupSize[0] == 8)
	B3D_TEST_ASSERT(computeBytecode->ThreadGroupSize[1] == 4)
	B3D_TEST_ASSERT(computeBytecode->ThreadGroupSize[2] == 2)

	const String waveSource = R"(
RWStructuredBuffer<uint> OutputData : register(u0);

[numthreads(32, 1, 1)]
void main(uint3 dispatchThreadId : SV_DispatchThreadID)
{
	OutputData[dispatchThreadId.x] = WaveActiveSum(dispatchThreadId.x + 1);
}
)";
	compile("HlslWave", GPT_COMPUTE_PROGRAM, waveSource);

	const String bindlessSource = R"(
cbuffer ResourceIndices : register(b0)
{
	uint TextureIndex;
	uint SamplerIndex;
};

float4 main(float2 uv : TEXCOORD0) : SV_Target0
{
	Texture2D<float4> textureResource = ResourceDescriptorHeap[TextureIndex];
	SamplerState samplerResource = SamplerDescriptorHeap[SamplerIndex];
	return textureResource.Sample(samplerResource, uv);
}
)";
	compile("HlslBindless", GPT_FRAGMENT_PROGRAM, bindlessSource);

	GpuProgramBytecode fxcBytecode;
	fxcBytecode.CompilerId = "HLSL_FXC";
	fxcBytecode.CompilerVersion = 6;
	B3D_TEST_ASSERT(!compiler->IsUpToDate(fxcBytecode))
#endif
}

void GpuBackendTestSuite::TestQueueStageFlags()
{
	B3D_TEST_ASSERT(GpuBackendUtility::GetQueueStageFlags(GQT_GRAPHICS) == GpuStageFlag::All)
	B3D_TEST_ASSERT(GpuBackendUtility::GetQueueStageFlags(GQT_COMPUTE).IsSet(GpuStageFlag::ComputeShaderNonUniform))
	B3D_TEST_ASSERT(!GpuBackendUtility::GetQueueStageFlags(GQT_COMPUTE).IsSetAny(GpuStageFlag::VertexShaderNonUniform | GpuStageFlag::FragmentShaderNonUniform))
	B3D_TEST_ASSERT(GpuBackendUtility::GetQueueStageFlags(GQT_TRANSFER) == (GpuStageFlag::Transfer | GpuStageFlag::Host))
}

void GpuBackendTestSuite::TestFramebufferAttachmentUsage()
{
	TrackedTestImage colorImage(4, 2, GpuTextureAspectFlag::Color);
	TrackedTestImage depthStencilImage(2, 1, GpuTextureAspectFlag::Depth | GpuTextureAspectFlag::Stencil);
	AttachmentTestFramebuffer framebuffer(640, 480, 2);

	const GpuTextureSubresourceRange colorRange(1, 1, 2, 2, GpuTextureAspectFlag::Color);
	const GpuTextureSubresourceRange depthStencilRange(0, 1, 1, 1, GpuTextureAspectFlag::Depth | GpuTextureAspectFlag::Stencil);
	framebuffer.AddColorAttachment(colorImage, colorRange, 3, GpuImageLayout::TransferSource);
	framebuffer.AddDepthStencilAttachment(depthStencilImage, depthStencilRange, GpuImageLayout::General);

	B3D_TEST_ASSERT(framebuffer.GetWidth() == 640)
	B3D_TEST_ASSERT(framebuffer.GetHeight() == 480)
	B3D_TEST_ASSERT(framebuffer.GetLayerCount() == 2)
	B3D_TEST_ASSERT(framebuffer.GetAttachmentCount() == 3)
	B3D_TEST_ASSERT(framebuffer.GetColorAttachmentCount() == 1)
	B3D_TEST_ASSERT(framebuffer.FindAttachment(RT_COLOR0) == nullptr)
	B3D_TEST_ASSERT(framebuffer.FindAttachment(RT_COLOR3)->GetIndex() == 3)
	B3D_TEST_ASSERT(framebuffer.FindAttachment(RT_DEPTH)->GetIndex() == 0)
	B3D_TEST_ASSERT(framebuffer.FindAttachment(RT_STENCIL)->GetIndex() == 0)
	B3D_TEST_ASSERT(framebuffer.FindAttachment(RT_DEPTH)->Range.AspectMask == GpuTextureAspectFlag::Depth)
	B3D_TEST_ASSERT(framebuffer.FindAttachment(RT_STENCIL)->Range.AspectMask == GpuTextureAspectFlag::Stencil)

	const RenderSurfaceMask readOnlyMask = RT_COLOR3 | RT_DEPTH;
	const RenderSurfaceMask loadMask = RT_COLOR3 | RT_STENCIL;
	const GpuRenderPassAttachmentUsageArray attachmentUsages = framebuffer.BuildRenderPassAttachmentUsages(readOnlyMask, loadMask);

	B3D_TEST_ASSERT(attachmentUsages.Size() == 3)
	B3D_TEST_ASSERT(attachmentUsages[0].Surface == RT_COLOR3)
	B3D_TEST_ASSERT(attachmentUsages[0].Access == GpuAccessFlag::Read)
	B3D_TEST_ASSERT(attachmentUsages[0].BarrierFlags == GpuBarrierFlag::None)
	B3D_TEST_ASSERT(attachmentUsages[0].Layout == GpuImageLayout::General)
	B3D_TEST_ASSERT(attachmentUsages[0].FinalLayout == GpuImageLayout::TransferSource)
	B3D_TEST_ASSERT(attachmentUsages[1].Surface == RT_DEPTH)
	B3D_TEST_ASSERT(attachmentUsages[1].Access == GpuAccessFlag::Read)
	B3D_TEST_ASSERT(attachmentUsages[1].BarrierFlags == GpuBarrierFlag::None)
	B3D_TEST_ASSERT(attachmentUsages[1].Layout == GpuImageLayout::DepthReadOnlyStencilAttachment)
	B3D_TEST_ASSERT(attachmentUsages[2].Surface == RT_STENCIL)
	B3D_TEST_ASSERT(attachmentUsages[2].Access == (GpuAccessFlag::Read | GpuAccessFlag::Write))
	B3D_TEST_ASSERT(attachmentUsages[2].BarrierFlags == GpuBarrierFlag::None)
	B3D_TEST_ASSERT(attachmentUsages[2].Layout == GpuImageLayout::DepthReadOnlyStencilAttachment)
	B3D_TEST_ASSERT(attachmentUsages[2].FinalLayout == GpuImageLayout::General)

	const GpuRenderPassAttachmentUsageArray discardAttachmentUsages = framebuffer.BuildRenderPassAttachmentUsages(RT_NONE, RT_NONE);
	for(const GpuRenderPassAttachmentUsage& attachmentUsage : discardAttachmentUsages)
		B3D_TEST_ASSERT(attachmentUsage.BarrierFlags == GpuBarrierFlag::DiscardContents)
}

void GpuBackendTestSuite::TestResourceUseTracking()
{
	GpuResourceTracker tracker;

	// Repeated uses of a buffer bind it once and accumulate their access
	TrackedTestBuffer buffer;
	tracker.TrackBufferUsage(&buffer, GpuAccessFlag::Write);
	tracker.TrackBufferUsage(&buffer, GpuAccessFlag::Read);

	const GpuBufferTrackingState* const bufferState = tracker.FindBufferTrackingState(&buffer);
	B3D_TEST_ASSERT(bufferState != nullptr)
	B3D_TEST_ASSERT(bufferState->UseHandle.Flags == (GpuAccessFlag::Read | GpuAccessFlag::Write))
	B3D_TEST_ASSERT(buffer.GetBoundCount() == 1)

	// Image uses bind the image once, and every subresource they cover
	TrackedTestImage image(2, 2, GpuTextureAspectFlag::Color);
	tracker.TrackImageUsage(&image, GpuTextureSubresourceRange(1, 1, 0, 2, GpuTextureAspectFlag::Color), GpuAccessFlag::Write);
	tracker.TrackImageUsage(&image, image.GetRange(), GpuAccessFlag::Read);

	const GpuResourceUseHandle* const imageUseHandle = tracker.FindImageUseHandle(&image);
	B3D_TEST_ASSERT(imageUseHandle != nullptr)
	B3D_TEST_ASSERT(imageUseHandle->Flags == (GpuAccessFlag::Read | GpuAccessFlag::Write))
	B3D_TEST_ASSERT(image.GetBoundCount() == 1)
	B3D_TEST_ASSERT(image.GetFullRangeSubresource()->GetBoundCount() == 1)
	B3D_TEST_ASSERT(image.GetSubresource(0, 1, GpuTextureAspectFlag::Color)->GetBoundCount() == 1)
	B3D_TEST_ASSERT(image.GetSubresource(0, 0, GpuTextureAspectFlag::Color)->GetBoundCount() == 0)

	// Shaders read a read-only attachment of the render pass in the attachment's layout
	TrackedTestImage attachmentImage(1, 1, GpuTextureAspectFlag::Color);
	GpuRenderPassAttachmentUsage attachmentUsage;
	attachmentUsage.Image = &attachmentImage;
	attachmentUsage.Range = attachmentImage.GetRange();
	attachmentUsage.Surface = RT_COLOR0;
	attachmentUsage.Access = GpuAccessFlag::Read;
	attachmentUsage.Layout = GpuImageLayout::General;

	TInlineArray<GpuRenderPassAttachmentUsage, 1> attachments;
	attachments.Add(attachmentUsage);
	tracker.BeginRenderPass(attachments);

	B3D_TEST_ASSERT(tracker.ResolveShaderImageLayout(&attachmentImage, attachmentImage.GetRange(), GpuImageLayout::ShaderReadOnly) == GpuImageLayout::General)
	B3D_TEST_ASSERT(tracker.TrackShaderImageUsage(&attachmentImage, attachmentImage.GetRange(), GpuAccessFlag::Read))

	// Writes to a sampled attachment are rejected while the render pass is active
	{
		LoggingScope logs(*this);
		logs.ExpectError("Framebuffer attachments sampled during a render pass must be marked read-only.");
		B3D_TEST_ASSERT(!tracker.TrackShaderImageUsage(&attachmentImage, attachmentImage.GetRange(), GpuAccessFlag::Read | GpuAccessFlag::Write))
	}

	tracker.EndRenderPass();
	B3D_TEST_ASSERT(attachmentImage.GetBoundCount() == 1)

	tracker.NotifyUnbound();
	tracker.Clear();
	B3D_TEST_ASSERT(buffer.GetBoundCount() == 0)
	B3D_TEST_ASSERT(image.GetBoundCount() == 0)
	B3D_TEST_ASSERT(image.GetFullRangeSubresource()->GetBoundCount() == 0)
	B3D_TEST_ASSERT(attachmentImage.GetBoundCount() == 0)
	B3D_TEST_ASSERT(tracker.FindBufferTrackingState(&buffer) == nullptr)
	B3D_TEST_ASSERT(tracker.FindImageUseHandle(&image) == nullptr)
}

void GpuBackendTestSuite::TestBarriers()
{
	GpuBackend& backend = GpuBackend::Instance();
	if(backend.GetDeviceCount() == 0)
		return;

	// Shaders are resources, which are created outside of the render thread
	const TShared<GpuDevice> device = backend.GetDevice(0);
	const TShared<GpuComputePipelineState> pipeline = CreateTestDispatchPipeline(*device);
	GetRenderThread().PostCommand([this, &device, &pipeline]()
	{
		static constexpr u32 kSize = 16;
		static constexpr u32 kPattern = 0x5A000000;
		const GpuAccessState transferWrite(GpuStageFlag::Transfer, GpuAccessFlag::Write, GpuImageLayout::TransferDestination);
		const GpuAccessState transferRead(GpuStageFlag::Transfer, GpuAccessFlag::Read, GpuImageLayout::TransferSource);
		const GpuAccessState hostRead(GpuStageFlag::Host, GpuAccessFlag::Read);
		static constexpr u32 bufferBytes = kSize * kSize * sizeof(u32);

		// Render targets are exclusive to one queue family on Vulkan, while sampleable-only textures are shared between them
		TextureCreateInformation exclusiveInformation;
		exclusiveInformation.Name = "Barrier exclusive texture";
		exclusiveInformation.Width = kSize;
		exclusiveInformation.Height = kSize;
		exclusiveInformation.Format = PF_RGBA8;
		exclusiveInformation.Usage = TextureUsageFlag::RenderTarget;

		TextureCreateInformation sharedInformation = exclusiveInformation;
		sharedInformation.Name = "Barrier shared texture";
		sharedInformation.Usage = TextureUsageFlag::Default;

		const ImageSubresourcePitch pitch = device->CreateTexture(exclusiveInformation)->GetStagingBufferPitchForSubresource(0, 0);
		const u32 stagingElementCount = pitch.RowPitch * pitch.SliceHeight;

		const TShared<render::GpuBuffer> upload = device->CreateGpuBuffer(GpuBufferCreateInformation::CreateStagingWrite(stagingElementCount * sizeof(u32)));
		{
			const render::GpuBufferMappedScope mapping = upload->Map(GpuMapOption::Write);
			B3D_TEST_ASSERT(mapping.IsValid())
			if(!mapping.IsValid())
				return;

			u32* elements = static_cast<u32*>(mapping.GetMappedMemory());
			for(u32 elementIndex = 0; elementIndex < stagingElementCount; elementIndex++)
				elements[elementIndex] = kPattern | elementIndex;
		}

		// A buffer and two textures written by copies, and a readback for each
		struct CaseResources
		{
			TShared<render::GpuBuffer> Buffer;
			TShared<render::Texture> Textures[2];
			TShared<render::GpuBuffer> Readbacks[3];
		};

		auto fnCreateResources = [&device, &exclusiveInformation, &sharedInformation, stagingElementCount]()
		{
			CaseResources resources;
			resources.Buffer = device->CreateGpuBuffer(GpuBufferCreateInformation::CreateStructuredStorage(sizeof(u32), kSize * kSize));
			resources.Textures[0] = device->CreateTexture(exclusiveInformation);
			resources.Textures[1] = device->CreateTexture(sharedInformation);
			for(TShared<render::GpuBuffer>& readback : resources.Readbacks)
				readback = device->CreateGpuBuffer(GpuBufferCreateInformation::CreateStagingRead(stagingElementCount * sizeof(u32)));

			return resources;
		};

		// The barriers between the writes of a case and its reads
		struct CaseBarriers
		{
			CaseBarriers(const CaseResources& resources, const GpuAccessState& source, const GpuAccessState& destination)
				: Buffer(resources.Buffer, source, destination),
				Textures{ GpuTextureBarrier(resources.Textures[0], source, destination), GpuTextureBarrier(resources.Textures[1], source, destination) }
			{ }

			GpuBarriers Get() const { return GpuBarriers(TArrayView<const GpuBufferBarrier>(&Buffer, 1), TArrayView<const GpuTextureBarrier>(Textures, 2)); }

			GpuBufferBarrier Buffer;
			GpuTextureBarrier Textures[2];
		};

		// Writes the buffer and the textures with copies, discarding their contents. The new textures start at rest.
		auto fnWrite = [&upload, &transferWrite](render::GpuCommandBuffer& commands, const CaseResources& resources)
		{
			CaseBarriers barriers(resources, GpuAccessState(), transferWrite);
			for(GpuTextureBarrier& barrier : barriers.Textures)
			{
				barrier.Source = GpuBackendUtility::GetRestingState(*barrier.Object, commands.GetQueueType());
				barrier.Source.Layout = GpuImageLayout::Undefined;
			}

			commands.IssueBarriers(barriers.Get());
			commands.CopyBufferToBuffer(upload, resources.Buffer, 0, 0, bufferBytes);
			for(const TShared<render::Texture>& texture : resources.Textures)
				commands.CopyBufferToTexture(upload, texture, 0, 0, 0);
		};

		// Copies the buffer and the textures into the readbacks, and makes the copies visible to the host
		auto fnRead = [&transferWrite, &hostRead](render::GpuCommandBuffer& commands, const CaseResources& resources)
		{
			const auto fnIssueReadbackBarriers = [&commands, &resources](const GpuAccessState& source, const GpuAccessState& destination)
			{
				const GpuBufferBarrier barriers[] = { GpuBufferBarrier(resources.Readbacks[0], source, destination),
					GpuBufferBarrier(resources.Readbacks[1], source, destination), GpuBufferBarrier(resources.Readbacks[2], source, destination) };
				commands.IssueBarriers(GpuBarriers(TArrayView<const GpuBufferBarrier>(barriers, 3)));
			};

			fnIssueReadbackBarriers(GpuAccessState(), transferWrite);
			commands.CopyBufferToBuffer(resources.Buffer, resources.Readbacks[0], 0, 0, bufferBytes);
			commands.CopyTextureToBuffer(resources.Textures[0], resources.Readbacks[1], 0, 0, 0);
			commands.CopyTextureToBuffer(resources.Textures[1], resources.Readbacks[2], 0, 0, 0);
			fnIssueReadbackBarriers(transferWrite, hostRead);
		};

		// Checks that a readback holds the pattern, in the buffer layout or in the texture staging layout
		auto fnCheckReadback = [this, &pitch](const TShared<render::GpuBuffer>& readback, bool isTexture, const String& caseName)
		{
			const render::GpuBufferMappedScope mapping = readback->Map(GpuMapOption::Read);
			B3D_TEST_ASSERT(mapping.IsValid())
			if(!mapping.IsValid())
				return;

			const u32* elements = static_cast<const u32*>(mapping.GetMappedMemory());
			bool matches = true;
			for(u32 row = 0; row < kSize; row++)
			{
				for(u32 column = 0; column < kSize; column++)
				{
					const u32 elementIndex = isTexture ? row * pitch.RowPitch + column : row * kSize + column;
					matches &= elements[elementIndex] == (kPattern | elementIndex);
				}
			}

			B3D_TEST_ASSERT_MSG(matches, caseName + ": the readback does not hold the written values.")
		};

		auto fnCheck = [&fnCheckReadback](const CaseResources& resources, const String& caseName)
		{
			fnCheckReadback(resources.Readbacks[0], false, caseName);
			fnCheckReadback(resources.Readbacks[1], true, caseName);
			fnCheckReadback(resources.Readbacks[2], true, caseName);
		};

		const TShared<GpuWorkContext> context = GpuWorkContext::Create(*device);
		const TShared<GpuCommandBufferPool> graphicsPool = GpuBarrierTestUtility::CreateCommandBufferPool(*device, GQT_GRAPHICS);

		// Each command buffer validates its barriers on its own, which must not report errors for any of them. The validators keep the
		// last bound parameter sets alive, so they must be destroyed before the context that allocated them.
#if B3D_BUILD_TYPE_DEVELOPMENT
		Vector<TUnique<GpuBarrierValidator>> validators;
#endif
		auto fnCreateCommands = [&](const TShared<GpuCommandBufferPool>& pool, const char* name)
		{
			const TShared<GpuCommandBuffer> commands = pool->Create(GpuCommandBufferCreateInformation::Create(name));
#if B3D_BUILD_TYPE_DEVELOPMENT
			validators.push_back(B3DMakeUnique<GpuBarrierValidator>());
			commands->SetValidator(validators.back().get());
#endif
			return commands;
		};

		// Full barriers
		{
			const CaseResources resources = fnCreateResources();
			const TShared<GpuCommandBuffer> commands = fnCreateCommands(graphicsPool, "Explicit full barriers");
			fnWrite(*commands, resources);
			commands->IssueBarriers(CaseBarriers(resources, transferWrite, transferRead).Get());
			fnRead(*commands, resources);
			context->SubmitCommandBuffer(commands, GpuQueueMask::kNone);
			device->WaitUntilIdle();
			fnCheck(resources, "Full barriers");
		}

		// Split barrier within one command buffer, with unrelated work between the halves
		{
			const CaseResources resources = fnCreateResources();
			const CaseBarriers barriers(resources, transferWrite, transferRead);
			const TShared<GpuSplitBarrier> split = device->CreateSplitBarrier();
			const TShared<render::GpuBuffer> unrelated = device->CreateGpuBuffer(GpuBufferCreateInformation::CreateStructuredStorage(sizeof(u32), kSize * kSize));

			const TShared<GpuCommandBuffer> commands = fnCreateCommands(graphicsPool, "Explicit split barriers");
			fnWrite(*commands, resources);
			commands->ReleaseBarriers(barriers.Get(), split);
			commands->IssueBarriers(GpuBufferBarrier(unrelated, GpuAccessState(), transferWrite));
			commands->CopyBufferToBuffer(upload, unrelated, 0, 0, bufferBytes);
			commands->AcquireBarriers(barriers.Get(), split);
			fnRead(*commands, resources);
			context->SubmitCommandBuffer(commands, GpuQueueMask::kNone);
			device->WaitUntilIdle();
			fnCheck(resources, "Split barriers within a command buffer");
		}

		// Split barriers whose halves are on different command buffers, recorded in either order. With @p readQueue on a queue of
		// another type than @p writeQueue, the split barrier transfers the resources between the queues.
		auto fnTestSplitAcrossCommandBuffers = [&](const TShared<GpuCommandBufferPool>& writePool, GpuQueueType writeQueue, GpuQueueType readQueue, bool acquireFirst, const String& caseName)
		{
			const CaseResources resources = fnCreateResources();
			const CaseBarriers barriers(resources, transferWrite, transferRead);
			const TShared<GpuSplitBarrier> split = writeQueue == readQueue ? device->CreateSplitBarrier() : device->CreateSplitBarrier(writeQueue, readQueue);

			const TShared<GpuCommandBuffer> readCommands = fnCreateCommands(graphicsPool, "Explicit split barrier acquire");
			if(acquireFirst)
				readCommands->AcquireBarriers(barriers.Get(), split);

			const TShared<GpuCommandBuffer> writeCommands = fnCreateCommands(writePool, "Explicit split barrier release");
			fnWrite(*writeCommands, resources);
			writeCommands->ReleaseBarriers(barriers.Get(), split);

			if(!acquireFirst)
				readCommands->AcquireBarriers(barriers.Get(), split);

			fnRead(*readCommands, resources);

			context->SubmitCommandBuffer(writeCommands, GpuQueueMask::kNone);
			context->SubmitCommandBuffer(readCommands, writeQueue == readQueue ? GpuQueueMask::kNone : GpuQueueMask(GpuQueueId(writeQueue, 0)));
			device->WaitUntilIdle();
			fnCheck(resources, caseName);
		};

		// Both command buffers record at once, which some backends only allow for command buffers from different pools
		const TShared<GpuCommandBufferPool> writePool = GpuBarrierTestUtility::CreateCommandBufferPool(*device, GQT_GRAPHICS);
		fnTestSplitAcrossCommandBuffers(writePool, GQT_GRAPHICS, GQT_GRAPHICS, false, "Split barriers across command buffers");
		fnTestSplitAcrossCommandBuffers(writePool, GQT_GRAPHICS, GQT_GRAPHICS, true, "Split barriers across command buffers, acquire recorded first");

		// Every supported device has a compute queue
		B3D_TEST_ASSERT(device->GetQueueCount(GQT_COMPUTE) > 0)
		const TShared<GpuCommandBufferPool> computePool = GpuBarrierTestUtility::CreateCommandBufferPool(*device, GQT_COMPUTE);
		fnTestSplitAcrossCommandBuffers(computePool, GQT_COMPUTE, GQT_GRAPHICS, false, "Split barriers across queues");
		fnTestSplitAcrossCommandBuffers(computePool, GQT_COMPUTE, GQT_GRAPHICS, true, "Split barriers across queues, acquire recorded first");

		// Compute shader writes to a storage buffer and a storage texture, on backends that compile shaders on the host
		if(pipeline != nullptr)
		{
			TextureCreateInformation storageInformation = exclusiveInformation;
			storageInformation.Name = "Barrier storage texture";
			storageInformation.Format = PF_R32U;
			storageInformation.Usage = TextureUsageFlag::AllowUnorderedAccessOnTheGPU;

			const TShared<render::GpuBuffer> buffer = device->CreateGpuBuffer(GpuBufferCreateInformation::CreateStructuredStorage(sizeof(u32), kSize * kSize, GpuBufferFlag::StoreOnGPU | GpuBufferFlag::AllowUnorderedAccessOnTheGPU));
			const TShared<render::Texture> texture = device->CreateTexture(storageInformation);
			const ImageSubresourcePitch storagePitch = texture->GetStagingBufferPitchForSubresource(0, 0);
			const TShared<render::GpuBuffer> bufferReadback = device->CreateGpuBuffer(GpuBufferCreateInformation::CreateStagingRead(bufferBytes));
			const TShared<render::GpuBuffer> textureReadback = device->CreateGpuBuffer(GpuBufferCreateInformation::CreateStagingRead(storagePitch.RowPitch * storagePitch.SliceHeight * sizeof(u32)));

			const TShared<render::GpuParameterSet> parameters = context->GetParameterSetPool().Create(pipeline->GetParameterLayout()->GetSet(0), 0);
			parameters->SetStorageBuffer("OutputBuffer", buffer);
			parameters->SetStorageTexture("OutputTexture", texture, TextureSurface());

			const TShared<GpuCommandBuffer> commands = fnCreateCommands(graphicsPool, "Barriers around a dispatch");
			const auto fnIssueBarriers = [&](const GpuAccessState& source, const GpuAccessState& textureSource, const GpuAccessState& destination, const GpuAccessState& readbackSource, const GpuAccessState& readbackDestination)
			{
				const GpuBufferBarrier bufferBarriers[] = { GpuBufferBarrier(buffer, source, destination),
					GpuBufferBarrier(bufferReadback, readbackSource, readbackDestination), GpuBufferBarrier(textureReadback, readbackSource, readbackDestination) };
				const GpuTextureBarrier textureBarrier(texture, textureSource, destination);
				commands->IssueBarriers(GpuBarriers(TArrayView<const GpuBufferBarrier>(bufferBarriers, 3), TArrayView<const GpuTextureBarrier>(&textureBarrier, 1)));
			};

			// The new texture starts at rest, and the dispatch overwrites it
			const GpuAccessState discardedRest = GpuBarrierTestUtility::GetDiscardedRestingState(*texture);

			const GpuAccessState computeReadWrite(GpuStageFlag::ComputeShaderNonUniform, GpuAccessFlag::Read | GpuAccessFlag::Write, GpuImageLayout::General);
			fnIssueBarriers(GpuAccessState(), discardedRest, computeReadWrite, GpuAccessState(), transferWrite);
			commands->SetGpuComputePipelineState(pipeline);
			commands->SetGpuParameterSet(parameters);
			commands->DispatchCompute(1, 1, 1);
			fnIssueBarriers(computeReadWrite, computeReadWrite, transferRead, transferWrite, transferWrite);
			commands->CopyBufferToBuffer(buffer, bufferReadback, 0, 0, bufferBytes);
			commands->CopyTextureToBuffer(texture, textureReadback, 0, 0, 0);
			fnIssueBarriers(transferRead, transferRead, transferRead, transferWrite, hostRead);
			context->SubmitCommandBuffer(commands, GpuQueueMask::kNone);
			device->WaitUntilIdle();

			fnCheckReadback(bufferReadback, false, "Dispatch");

			const render::GpuBufferMappedScope mapping = textureReadback->Map(GpuMapOption::Read);
			B3D_TEST_ASSERT(mapping.IsValid())
			if(mapping.IsValid())
			{
				const u32* elements = static_cast<const u32*>(mapping.GetMappedMemory());
				bool matches = true;
				for(u32 row = 0; row < kSize; row++)
				{
					for(u32 column = 0; column < kSize; column++)
						matches &= elements[row * storagePitch.RowPitch + column] == (kPattern | (row * kSize + column));
				}

				B3D_TEST_ASSERT_MSG(matches, "Dispatch: the storage texture readback does not hold the written values.")
			}
		}
		else
			B3D_TEST_ASSERT(String(GpuBackend::Instance().GetBackendName()) != "bsfVulkanGpuBackend")
	}, "GpuBackendTestSuite::TestBarriers", true);
}

void GpuBackendTestSuite::TestSplitBarrierExecution()
{
	GpuBackend& backend = GpuBackend::Instance();
	if(backend.GetDeviceCount() == 0)
		return;

	// Shaders are resources, which are created outside of the render thread
	const TShared<GpuDevice> device = backend.GetDevice(0);
	const TShared<GpuComputePipelineState> pipeline = CreateTestSlowWritePipeline(*device);
	if(pipeline == nullptr)
		return;

	GetRenderThread().PostCommand([this, &device, &pipeline]()
	{
		static constexpr u32 kThreadCount = 64 * 1024;
		static constexpr u32 kPattern = 0x5A000000;
		static constexpr u32 kBufferBytes = kThreadCount * 2 * sizeof(u32);
		const GpuAccessState computeReadWrite(GpuStageFlag::ComputeShaderNonUniform, GpuAccessFlag::Read | GpuAccessFlag::Write);
		const GpuAccessState transferWrite(GpuStageFlag::Transfer, GpuAccessFlag::Write);
		const GpuAccessState transferRead(GpuStageFlag::Transfer, GpuAccessFlag::Read);
		const GpuAccessState hostRead(GpuStageFlag::Host, GpuAccessFlag::Read);

		const TShared<GpuCommandBufferPool> pool = GpuBarrierTestUtility::CreateCommandBufferPool(*device);
		const TShared<GpuCommandBufferPool> otherPool = GpuBarrierTestUtility::CreateCommandBufferPool(*device);
		const TShared<GpuWorkContext> context = GpuWorkContext::Create(*device);

		const TShared<render::GpuBuffer> unrelatedSource = device->CreateGpuBuffer(GpuBufferCreateInformation::CreateStructuredStorage(sizeof(u32), 256));
		const TShared<render::GpuBuffer> unrelatedDestination = device->CreateGpuBuffer(GpuBufferCreateInformation::CreateStructuredStorage(sizeof(u32), 256));

		// The dispatch writes the buffer, the split barrier orders the write before the copy into the readback, and unrelated work
		// is recorded between the halves. With @p separateCommandBuffers the acquire and the copy are on a second command buffer.
		auto fnTest = [&](bool separateCommandBuffers, const String& caseName)
		{
			const TShared<render::GpuBuffer> buffer = device->CreateGpuBuffer(GpuBufferCreateInformation::CreateStructuredStorage(sizeof(u32), kThreadCount * 2, GpuBufferFlag::StoreOnGPU | GpuBufferFlag::AllowUnorderedAccessOnTheGPU));
			const TShared<render::GpuBuffer> readback = device->CreateGpuBuffer(GpuBufferCreateInformation::CreateStagingRead(kBufferBytes));
			const TShared<GpuSplitBarrier> split = device->CreateSplitBarrier();

			const TShared<render::GpuParameterSet> parameters = context->GetParameterSetPool().Create(pipeline->GetParameterLayout()->GetSet(0), 0);
			parameters->SetStorageBuffer("OutputBuffer", buffer);

			const TShared<GpuCommandBuffer> writeCommands = pool->Create(GpuCommandBufferCreateInformation::Create("Split barrier slow write"));
			const TShared<GpuCommandBuffer> readCommands = separateCommandBuffers ? otherPool->Create(GpuCommandBufferCreateInformation::Create("Split barrier read")) : writeCommands;

			const GpuBufferBarrier writeBarriers[] = { GpuBufferBarrier(buffer, GpuAccessState(), computeReadWrite), GpuBufferBarrier(readback, GpuAccessState(), transferWrite),
				GpuBufferBarrier(unrelatedSource, GpuAccessState(), transferRead), GpuBufferBarrier(unrelatedDestination, GpuAccessState(), transferWrite) };
			writeCommands->IssueBarriers(GpuBarriers(TArrayView<const GpuBufferBarrier>(writeBarriers, 4)));
			writeCommands->SetGpuComputePipelineState(pipeline);
			writeCommands->SetGpuParameterSet(parameters);
			writeCommands->DispatchCompute(kThreadCount / 64, 1, 1);

			const GpuBufferBarrier splitBarrier(buffer, computeReadWrite, transferRead);
			const GpuBarriers splitBarriers(TArrayView<const GpuBufferBarrier>(&splitBarrier, 1));
			writeCommands->ReleaseBarriers(splitBarriers, split);
			writeCommands->CopyBufferToBuffer(unrelatedSource, unrelatedDestination, 0, 0, 256 * sizeof(u32));

			readCommands->AcquireBarriers(splitBarriers, split);
			readCommands->CopyBufferToBuffer(buffer, readback, 0, 0, kBufferBytes);

			const GpuBufferBarrier readbackBarrier(readback, transferWrite, hostRead);
			readCommands->IssueBarriers(GpuBarriers(TArrayView<const GpuBufferBarrier>(&readbackBarrier, 1)));

			context->SubmitCommandBuffer(writeCommands, GpuQueueMask::kNone);
			if(separateCommandBuffers)
				context->SubmitCommandBuffer(readCommands, GpuQueueMask::kNone);

			device->WaitUntilIdle();

			const render::GpuBufferMappedScope mapping = readback->Map(GpuMapOption::Read);
			B3D_TEST_ASSERT(mapping.IsValid())
			if(!mapping.IsValid())
				return;

			const u32* elements = static_cast<const u32*>(mapping.GetMappedMemory());
			bool matches = true;
			for(u32 threadIndex = 0; threadIndex < kThreadCount; threadIndex++)
				matches &= elements[threadIndex * 2] == (kPattern | threadIndex);

			B3D_TEST_ASSERT_MSG(matches, caseName + ": the readback does not hold the values the dispatch wrote.")
		};

		fnTest(false, "Split barrier within a command buffer");
		fnTest(true, "Split barrier across command buffers");
	}, "GpuBackendTestSuite::TestSplitBarrierExecution", true);
}

void GpuBackendTestSuite::TestBarrierAcrossPasses()
{
	GpuBackend& backend = GpuBackend::Instance();
	if(backend.GetDeviceCount() == 0)
		return;

	// Shaders are resources, which are created outside of the render thread
	const TShared<GpuDevice> device = backend.GetDevice(0);
	const TShared<GpuComputePipelineState> pipeline = CreateTestSlowWritePipeline(*device);
	if(pipeline == nullptr)
		return;

	GetRenderThread().PostCommand([this, &device, &pipeline]()
	{
		static constexpr u32 kThreadCount = 64 * 1024;
		static constexpr u32 kPattern = 0x5A000000;
		static constexpr u32 kBufferBytes = kThreadCount * 2 * sizeof(u32);
		const GpuAccessState computeReadWrite(GpuStageFlag::ComputeShaderNonUniform, GpuAccessFlag::Read | GpuAccessFlag::Write);
		const GpuAccessState transferWrite(GpuStageFlag::Transfer, GpuAccessFlag::Write);
		const GpuAccessState transferRead(GpuStageFlag::Transfer, GpuAccessFlag::Read);
		const GpuAccessState hostRead(GpuStageFlag::Host, GpuAccessFlag::Read);
		const GpuAccessState colorAttachment(GpuStageFlag::ColorAttachment, GpuAccessFlag::Read | GpuAccessFlag::Write, GpuImageLayout::ColorAttachment);

		const TShared<GpuCommandBufferPool> pool = GpuBarrierTestUtility::CreateCommandBufferPool(*device);
		const TShared<GpuWorkContext> context = GpuWorkContext::Create(*device);

		TextureCreateInformation targetInformation;
		targetInformation.Name = "BarrierAcrossPassesTarget";
		targetInformation.Width = 16;
		targetInformation.Height = 16;
		targetInformation.Format = PF_RGBA8;
		targetInformation.Usage = TextureUsageFlag::RenderTarget;

		// The dispatch writes the buffer, and the barrier ordering the write before the copy is issued right after it. With
		// @p separateCommandBuffers the copy is on a second command buffer, otherwise it follows a render pass.
		auto fnTest = [&](bool separateCommandBuffers, const String& caseName)
		{
			const TShared<render::GpuBuffer> buffer = device->CreateGpuBuffer(GpuBufferCreateInformation::CreateStructuredStorage(sizeof(u32), kThreadCount * 2, GpuBufferFlag::StoreOnGPU | GpuBufferFlag::AllowUnorderedAccessOnTheGPU));
			const TShared<render::GpuBuffer> readback = device->CreateGpuBuffer(GpuBufferCreateInformation::CreateStagingRead(kBufferBytes));

			const TShared<render::GpuParameterSet> parameters = context->GetParameterSetPool().Create(pipeline->GetParameterLayout()->GetSet(0), 0);
			parameters->SetStorageBuffer("OutputBuffer", buffer);

			const TShared<GpuCommandBuffer> writeCommands = pool->Create(GpuCommandBufferCreateInformation::Create("Barrier across passes write"));
			const TShared<GpuCommandBuffer> readCommands = separateCommandBuffers ? pool->Create(GpuCommandBufferCreateInformation::Create("Barrier across passes read")) : writeCommands;

			const GpuBufferBarrier writeBarriers[] = { GpuBufferBarrier(buffer, GpuAccessState(), computeReadWrite), GpuBufferBarrier(readback, GpuAccessState(), transferWrite) };
			writeCommands->IssueBarriers(GpuBarriers(TArrayView<const GpuBufferBarrier>(writeBarriers, 2)));
			writeCommands->SetGpuComputePipelineState(pipeline);
			writeCommands->SetGpuParameterSet(parameters);
			writeCommands->DispatchCompute(kThreadCount / 64, 1, 1);
			writeCommands->IssueBarriers(GpuBufferBarrier(buffer, computeReadWrite, transferRead));

			if(!separateCommandBuffers)
			{
				render::RenderTextureCreateInformation renderTextureInformation;
				renderTextureInformation.ColorSurfaces[0].Texture = device->CreateTexture(targetInformation);

				RenderPassCreateInformation pass(render::RenderTexture::Create(renderTextureInformation));
				pass.ClearMask = RT_COLOR0;

				// The new texture starts at rest, and the clear discards its contents
				const GpuAccessState discardedRest = GpuBarrierTestUtility::GetDiscardedRestingState(*renderTextureInformation.ColorSurfaces[0].Texture);

				writeCommands->IssueBarriers(GpuTextureBarrier(renderTextureInformation.ColorSurfaces[0].Texture, discardedRest, colorAttachment));
				writeCommands->BeginRenderPass(pass);
				writeCommands->EndRenderPass();
			}

			readCommands->CopyBufferToBuffer(buffer, readback, 0, 0, kBufferBytes);
			readCommands->IssueBarriers(GpuBufferBarrier(readback, transferWrite, hostRead));

			context->SubmitCommandBuffer(writeCommands, GpuQueueMask::kNone);
			if(separateCommandBuffers)
				context->SubmitCommandBuffer(readCommands, GpuQueueMask::kNone);

			device->WaitUntilIdle();

			const render::GpuBufferMappedScope mapping = readback->Map(GpuMapOption::Read);
			B3D_TEST_ASSERT(mapping.IsValid())
			if(!mapping.IsValid())
				return;

			const u32* elements = static_cast<const u32*>(mapping.GetMappedMemory());
			bool matches = true;
			for(u32 threadIndex = 0; threadIndex < kThreadCount; threadIndex++)
				matches &= elements[threadIndex * 2] == (kPattern | threadIndex);

			B3D_TEST_ASSERT_MSG(matches, caseName + ": the readback does not hold the values the dispatch wrote.")
		};

		fnTest(false, "Copy after a render pass");
		fnTest(true, "Copy in a later command buffer");
	}, "GpuBackendTestSuite::TestBarrierAcrossPasses", true);
}

#if B3D_BUILD_TYPE_DEVELOPMENT
void GpuBackendTestSuite::TestBarrierValidation()
{
	GpuBackend& backend = GpuBackend::Instance();
	if(backend.GetDeviceCount() == 0)
		return;

	// Each case uses its own resource, named after the case, so the errors do not cascade and each one identifies its case
	const String undeclaredStages = "The resource was accessed in stages that its last barrier does not declare.";
	// The D3D12 debug layer tracks layouts natively and fails to close a command list with a misdeclared layout
	const bool recordsMisdeclaredLayouts = String(backend.GetBackendName()) != "bsfD3D12GpuBackend";

	LoggingScope logs(*this);
	if(recordsMisdeclaredLayouts)
		logs.ExpectError("The source layout of a barrier differs from the destination layout of the previous barrier of the resource. Image: 'SourceLayout'.");

	logs.ExpectError("The source of a barrier is missing stages that accessed the resource since the previous barrier. Buffer: 'MissingStages'.");
	logs.ExpectError("The source of a barrier is missing the writes of the resource since the previous barrier. Buffer: 'MissingWrites'.");
	logs.ExpectError("The resource was accessed with reads or writes that its last barrier does not declare. Buffer: 'UndeclaredWrite'.");
	logs.ExpectError("The resource was accessed between the release and the acquire of a split barrier. Buffer: 'AccessBetweenHalves'.");
	logs.ExpectError("A barrier was recorded between the release and the acquire of a split barrier of the same resource. Buffer: 'BarrierBetweenHalves'.");
	logs.ExpectError("An alias acquire must precede every other use of the resource. Buffer: 'LateAliasAcquire'.");
	logs.ExpectError("The release and the acquire of a split barrier must be given the same barriers, in the same order.");
	logs.ExpectError("The image was accessed in a different layout than its last barrier declares. Image: 'RenderPassLayout'.");
	logs.ExpectError("The release of a split barrier was recorded on a queue type other than the one the split barrier was created for.");
	for(const char* name : { "Buffer: 'CopyBufferToBuffer'.", "Image: 'CopyBufferToTexture'.", "Image: 'CopyTextureToBuffer'.", "Image: 'CopyTexture'.", "Image: 'BlitTexture'.", "Image: 'RenderPass'." })
		logs.ExpectError(undeclaredStages + " " + name);

	// Backends that track layouts natively, such as the Vulkan validation layers, also report the misdeclared layouts
	logs.IgnoreError("VUID-VkImageMemoryBarrier-oldLayout-01197");
	logs.IgnoreError("VUID-vkCmdBeginRenderPass-initialLayout-00900");
	logs.IgnoreError("does not support the required access (D3D12_BARRIER_ACCESS_RENDER_TARGET)");

	// The D3D12 debug layer reports accesses and barriers between the halves of split barriers on resources that get barriers of their own
	logs.IgnoreError("possibly pending an incomplete split barrier");
	logs.IgnoreError("is pending a previous split barrier");
	logs.IgnoreError("is not paired with a matching begin-only barrier");

	// The host stage maps to no D3D12 synchronization, so the D3D12 debug layer reports commands after barriers that declare only host access
	logs.IgnoreError("cannot be accessed after a previous barrier specifies D3D12_BARRIER_SYNC_NONE as SyncAfter");

	// Vulkan synchronization validation reports the clear of the render pass whose attachment is declared for another stage
	logs.IgnoreError("vkCmdBeginRenderPass(): WRITE_AFTER_WRITE hazard detected");

	const TShared<GpuDevice> device = backend.GetDevice(0);
	const TShared<GpuComputePipelineState> pipeline = CreateTestDispatchPipeline(*device);
	if(pipeline != nullptr)
		logs.ExpectError(undeclaredStages + " Buffer: 'DispatchCompute'.");

	GetRenderThread().PostCommand([&device, &pipeline, recordsMisdeclaredLayouts]()
	{
		static constexpr u32 kSize = 16;
		const GpuAccessFlags readWrite = GpuAccessFlag::Read | GpuAccessFlag::Write;
		const GpuAccessState transferWrite(GpuStageFlag::Transfer, GpuAccessFlag::Write, GpuImageLayout::TransferDestination);
		const GpuAccessState transferRead(GpuStageFlag::Transfer, GpuAccessFlag::Read, GpuImageLayout::TransferSource);
		const GpuAccessState colorAttachment(GpuStageFlag::ColorAttachment, readWrite, GpuImageLayout::ColorAttachment);

		// The misdeclared cases also access resources outside their resting states, which would report errors of their own
		const bool validateRestingStates = gGpuValidateRestingStates;
		gGpuValidateRestingStates.Set(false);

		// Declares accesses in other stages than the ones the commands access in
		const GpuAccessState host(GpuStageFlag::Host, readWrite);

		const auto fnCreateBuffer = [&device](const char* name)
		{
			const TShared<render::GpuBuffer> buffer = device->CreateGpuBuffer(GpuBufferCreateInformation::CreateStructuredStorage(sizeof(u32), kSize * kSize));
			buffer->SetName(name);
			return buffer;
		};

		const auto fnCreateTexture = [&device](const char* name)
		{
			TextureCreateInformation information;
			information.Name = name;
			information.Width = kSize;
			information.Height = kSize;
			information.Format = PF_RGBA8;
			information.Usage = TextureUsageFlag::RenderTarget;
			return device->CreateTexture(information);
		};

		const auto fnCreateTarget = [](const TShared<render::Texture>& texture)
		{
			render::RenderTextureCreateInformation information;
			information.ColorSurfaces[0].Texture = texture;
			return render::RenderTexture::Create(information);
		};

		// Parameter sets come from the context, which outlives the command buffer that keeps them bound
		const TShared<GpuWorkContext> context = GpuWorkContext::Create(*device);

		const TShared<GpuCommandBufferPool> pool = GpuBarrierTestUtility::CreateCommandBufferPool(*device);
		GpuBarrierValidator validator;
		const TShared<GpuCommandBuffer> commandBuffer = pool->Create(GpuCommandBufferCreateInformation::Create("Barrier validation"));
		commandBuffer->SetValidator(&validator);

		const TShared<render::GpuBuffer> source = fnCreateBuffer("Source");
		const TShared<render::Texture> sourceTexture = fnCreateTexture("SourceTexture");
		const ImageSubresourcePitch pitch = sourceTexture->GetStagingBufferPitchForSubresource(0, 0);
		const u32 stagingSize = pitch.RowPitch * pitch.SliceHeight * 4;
		const TShared<render::GpuBuffer> upload = device->CreateGpuBuffer(GpuBufferCreateInformation::CreateStagingWrite(stagingSize));
		commandBuffer->IssueBarriers(GpuBufferBarrier(source, GpuAccessState(), transferRead));
		commandBuffer->IssueBarriers(GpuBufferBarrier(upload, GpuAccessState(), transferRead));
		commandBuffer->IssueBarriers(GpuTextureBarrier(sourceTexture, GpuAccessState(), transferRead));
		const auto fnCopy = [&commandBuffer, &source](const TShared<render::GpuBuffer>& destination) { commandBuffer->CopyBufferToBuffer(source, destination, 0, 0, sizeof(u32)); };

		// Each case reads back into its own buffer, as copies into one buffer would need barriers between them. The validator identifies
		// resources by address, so every buffer must outlive the recording.
		Vector<TShared<render::GpuBuffer>> readbacks;
		const auto fnCreateReadback = [&device, &readbacks, stagingSize]()
		{
			readbacks.push_back(device->CreateGpuBuffer(GpuBufferCreateInformation::CreateStagingRead(stagingSize)));
			return readbacks.back();
		};

		// The source layout must be the declared layout
		if(recordsMisdeclaredLayouts)
		{
			const TShared<render::Texture> sourceLayout = fnCreateTexture("SourceLayout");
			commandBuffer->IssueBarriers(GpuTextureBarrier(sourceLayout, GpuAccessState(), transferWrite));
			commandBuffer->IssueBarriers(GpuTextureBarrier(sourceLayout, transferRead, colorAttachment));
		}

		// The source must include the stages and the writes since the previous barrier
		const TShared<render::GpuBuffer> missingStages = fnCreateBuffer("MissingStages");
		fnCopy(missingStages);
		commandBuffer->IssueBarriers(GpuBufferBarrier(missingStages, GpuAccessState(GpuStageFlag::VertexShaderNonUniform, GpuAccessFlag::Write), transferRead));

		const TShared<render::GpuBuffer> missingWrites = fnCreateBuffer("MissingWrites");
		fnCopy(missingWrites);
		commandBuffer->IssueBarriers(GpuBufferBarrier(missingWrites, transferRead, transferRead));

		// Accesses must be declared by the last barrier
		const TShared<render::GpuBuffer> undeclaredWrite = fnCreateBuffer("UndeclaredWrite");
		commandBuffer->IssueBarriers(GpuBufferBarrier(undeclaredWrite, GpuAccessState(), transferRead));
		fnCopy(undeclaredWrite);

		// The resource must not be accessed, or given another barrier, between the halves of a split barrier
		const TShared<render::GpuBuffer> accessBetweenHalves = fnCreateBuffer("AccessBetweenHalves");
		const TShared<GpuSplitBarrier> accessedSplit = device->CreateSplitBarrier();
		const GpuBufferBarrier accessedBarrier(accessBetweenHalves, transferWrite, transferRead);
		commandBuffer->ReleaseBarriers(accessedBarrier, accessedSplit);
		commandBuffer->CopyBufferToBuffer(accessBetweenHalves, fnCreateReadback(), 0, 0, sizeof(u32));
		commandBuffer->AcquireBarriers(accessedBarrier, accessedSplit);

		const TShared<render::GpuBuffer> barrierBetweenHalves = fnCreateBuffer("BarrierBetweenHalves");
		const TShared<GpuSplitBarrier> interruptedSplit = device->CreateSplitBarrier();
		const GpuBufferBarrier interruptedBarrier(barrierBetweenHalves, transferWrite, transferRead);
		commandBuffer->ReleaseBarriers(interruptedBarrier, interruptedSplit);
		commandBuffer->IssueBarriers(GpuBufferBarrier(barrierBetweenHalves, transferWrite, transferRead));
		commandBuffer->AcquireBarriers(interruptedBarrier, interruptedSplit);

		// An alias acquire must be the first use of the resource
		const TShared<render::GpuBuffer> lateAliasAcquire = fnCreateBuffer("LateAliasAcquire");
		fnCopy(lateAliasAcquire);
		commandBuffer->IssueBarriers(GpuBufferBarrier(lateAliasAcquire, GpuAccessState(), transferWrite, GpuBarrierFlag::AliasAcquire));

		// Both halves of a split barrier must be given the same barriers
		const TShared<render::GpuBuffer> mismatchedHalves = fnCreateBuffer("MismatchedHalves");
		const TShared<GpuSplitBarrier> mismatchedSplit = device->CreateSplitBarrier();
		commandBuffer->ReleaseBarriers(GpuBufferBarrier(mismatchedHalves, transferWrite, transferRead), mismatchedSplit);
		commandBuffer->AcquireBarriers(GpuBufferBarrier(mismatchedHalves, transferWrite, GpuAccessState(GpuStageFlag::Host, GpuAccessFlag::Read)), mismatchedSplit);

		// Each half must be recorded on the queue type the split barrier was created for. Nothing is recorded otherwise.
		const TShared<GpuSplitBarrier> otherQueueSplit = device->CreateSplitBarrier(GQT_COMPUTE, GQT_GRAPHICS);
		commandBuffer->ReleaseBarriers(GpuBufferBarrier(fnCreateBuffer("OtherQueue"), transferWrite, transferRead), otherQueueSplit);

		// Each command validates its accesses
		const TShared<render::GpuBuffer> copyBufferToBuffer = fnCreateBuffer("CopyBufferToBuffer");
		commandBuffer->IssueBarriers(GpuBufferBarrier(copyBufferToBuffer, GpuAccessState(), host));
		commandBuffer->CopyBufferToBuffer(copyBufferToBuffer, fnCreateReadback(), 0, 0, sizeof(u32));

		const TShared<render::Texture> copyBufferToTexture = fnCreateTexture("CopyBufferToTexture");
		commandBuffer->IssueBarriers(GpuTextureBarrier(copyBufferToTexture, GpuAccessState(), host));
		commandBuffer->CopyBufferToTexture(upload, copyBufferToTexture, 0, 0, 0);

		const TShared<render::Texture> copyTextureToBuffer = fnCreateTexture("CopyTextureToBuffer");
		commandBuffer->IssueBarriers(GpuTextureBarrier(copyTextureToBuffer, GpuAccessState(), host));
		commandBuffer->CopyTextureToBuffer(copyTextureToBuffer, fnCreateReadback(), 0, 0);

		const TShared<render::Texture> copyTexture = fnCreateTexture("CopyTexture");
		commandBuffer->IssueBarriers(GpuTextureBarrier(copyTexture, GpuAccessState(), host));
		commandBuffer->CopyTexture(sourceTexture, copyTexture);

		const TShared<render::Texture> blitTexture = fnCreateTexture("BlitTexture");
		commandBuffer->IssueBarriers(GpuTextureBarrier(blitTexture, GpuAccessState(), host));
		commandBuffer->BlitTexture(sourceTexture, blitTexture);

		// Attachments must be declared for the attachment stages, in the attachment layout
		const TShared<render::Texture> renderPass = fnCreateTexture("RenderPass");
		commandBuffer->IssueBarriers(GpuTextureBarrier(renderPass, GpuAccessState(), GpuAccessState(GpuStageFlag::FragmentShaderNonUniform, readWrite, GpuImageLayout::ColorAttachment)));
		commandBuffer->BeginRenderPass(RenderPassCreateInformation(fnCreateTarget(renderPass)));
		commandBuffer->EndRenderPass();

		const TShared<render::Texture> renderPassLayout = fnCreateTexture("RenderPassLayout");
		commandBuffer->IssueBarriers(GpuTextureBarrier(renderPassLayout, GpuAccessState(), GpuAccessState(GpuStageFlag::ColorAttachment, readWrite, GpuImageLayout::TransferDestination)));
		commandBuffer->BeginRenderPass(RenderPassCreateInformation(fnCreateTarget(renderPassLayout)));
		commandBuffer->EndRenderPass();

		// Dispatches validate the resources of their bound parameter sets
		if(pipeline != nullptr)
		{
			const GpuAccessState computeReadWrite(GpuStageFlag::ComputeShaderNonUniform, readWrite, GpuImageLayout::General);

			const TShared<render::GpuBuffer> dispatchCompute = device->CreateGpuBuffer(GpuBufferCreateInformation::CreateStructuredStorage(sizeof(u32), kSize * kSize, GpuBufferFlag::StoreOnGPU | GpuBufferFlag::AllowUnorderedAccessOnTheGPU));
			dispatchCompute->SetName("DispatchCompute");
			commandBuffer->IssueBarriers(GpuBufferBarrier(dispatchCompute, GpuAccessState(), host));

			TextureCreateInformation storageInformation;
			storageInformation.Name = "DispatchComputeTexture";
			storageInformation.Width = kSize;
			storageInformation.Height = kSize;
			storageInformation.Format = PF_R32U;
			storageInformation.Usage = TextureUsageFlag::AllowUnorderedAccessOnTheGPU;
			const TShared<render::Texture> storageTexture = device->CreateTexture(storageInformation);
			commandBuffer->IssueBarriers(GpuTextureBarrier(storageTexture, GpuAccessState(), computeReadWrite));

			const TShared<render::GpuParameterSet> parameters = context->GetParameterSetPool().Create(pipeline->GetParameterLayout()->GetSet(0), 0);
			parameters->SetStorageBuffer("OutputBuffer", dispatchCompute);
			parameters->SetStorageTexture("OutputTexture", storageTexture, TextureSurface());

			commandBuffer->SetGpuComputePipelineState(pipeline);
			commandBuffer->SetGpuParameterSet(parameters);
			commandBuffer->DispatchCompute(1, 1, 1);
		}

		// Correctly declared accesses report nothing
		const TShared<render::Texture> declared = fnCreateTexture("Declared");
		commandBuffer->IssueBarriers(GpuTextureBarrier(declared, GpuAccessState(), transferWrite));
		commandBuffer->CopyTexture(sourceTexture, declared);
		commandBuffer->IssueBarriers(GpuTextureBarrier(declared, transferWrite, colorAttachment));
		commandBuffer->BeginRenderPass(RenderPassCreateInformation(fnCreateTarget(declared)));
		commandBuffer->EndRenderPass();
		commandBuffer->IssueBarriers(GpuTextureBarrier(declared, colorAttachment, transferRead));
		commandBuffer->CopyTextureToBuffer(declared, fnCreateReadback(), 0, 0);

		// The command buffer is never submitted
		commandBuffer->End();
		pool->Reset();
		gGpuValidateRestingStates.Set(validateRestingStates);

		// Backends may only report their validation messages when the device next goes idle, which must happen within the logging scope
		device->WaitUntilIdle();
	}, "GpuBackendTestSuite::TestBarrierValidation", true);
}
#endif

#if B3D_BUILD_TYPE_DEVELOPMENT
void GpuBackendTestSuite::TestDrawAccessValidation()
{
	GpuDrawAccessValidator validator;
	int firstResource = 0;
	int secondResource = 0;
	const GpuAccessFlags accesses[] = { GpuAccessFlag::Read, GpuAccessFlag::Write, GpuAccessFlag::Read | GpuAccessFlag::Write };
	for(GpuAccessFlags firstAccess : accesses)
	{
		for(GpuAccessFlags secondAccess : accesses)
		{
			validator.BeginRenderPass();
			validator.ClearBindings();
			validator.AddResource(&firstResource, firstAccess);
			B3D_TEST_ASSERT(validator.ValidateDraw());
			validator.ClearBindings();
			validator.AddResource(&firstResource, secondAccess);
			B3D_TEST_ASSERT(validator.ValidateDraw() == !(firstAccess | secondAccess).IsSet(GpuAccessFlag::Write));
		}
	}

	validator.BeginRenderPass();
	validator.ClearBindings();
	validator.AddResource(&firstResource, GpuAccessFlag::Read);
	for(u32 drawIndex = 0; drawIndex < 100; drawIndex++)
		B3D_TEST_ASSERT(validator.ValidateDraw());

	// Binding without a draw does not contribute an access.
	validator.ClearBindings();
	validator.AddResource(&secondResource, GpuAccessFlag::Write);
	validator.ClearBindings();
	validator.AddResource(&secondResource, GpuAccessFlag::Read);
	B3D_TEST_ASSERT(validator.ValidateDraw());

	validator.BeginRenderPass();
	validator.ClearBindings();
	validator.AddResource(&firstResource, GpuAccessFlag::Read);
	validator.AddResource(&firstResource, GpuAccessFlag::Write);
	B3D_TEST_ASSERT(validator.ValidateDraw());
	B3D_TEST_ASSERT(!validator.ValidateDraw());

	// Rejected draws must not publish accesses to otherwise independent resources.
	validator.ClearBindings();
	validator.AddResource(&firstResource, GpuAccessFlag::Read);
	validator.AddResource(&secondResource, GpuAccessFlag::Write);
	B3D_TEST_ASSERT(!validator.ValidateDraw());
	validator.ClearBindings();
	validator.AddResource(&secondResource, GpuAccessFlag::Write);
	B3D_TEST_ASSERT(validator.ValidateDraw());
	validator.BeginRenderPass();
	B3D_TEST_ASSERT(validator.ValidateDraw());
}

#endif

namespace
{
	/** Sets the transient resource configuration variables, and restores their earlier values when destroyed. */
	class TransientConfigurationScope
	{
	public:
		TransientConfigurationScope(u32 heapMinimumSize, u32 idleFrames, u32 maxCachedTextures)
			: mHeapMinimumSize(gGpuTransientHeapMinimumSize), mIdleFrames(gGpuTransientIdleFrames), mMaxCachedTextures(gGpuTransientMaxCachedTextures)
		{
			gGpuTransientHeapMinimumSize.Set(heapMinimumSize);
			gGpuTransientIdleFrames.Set(idleFrames);
			gGpuTransientMaxCachedTextures.Set(maxCachedTextures);
		}

		~TransientConfigurationScope()
		{
			gGpuTransientHeapMinimumSize.Set(mHeapMinimumSize);
			gGpuTransientIdleFrames.Set(mIdleFrames);
			gGpuTransientMaxCachedTextures.Set(mMaxCachedTextures);
		}

	private:
		u32 mHeapMinimumSize;
		u32 mIdleFrames;
		u32 mMaxCachedTextures;
	};

	/** Returns the number of heaps in all of the device's transient heap pools, and their total size. */
	GpuTransientHeapPoolStatistics GetTransientHeapStatistics(const GpuDevice& device)
	{
		GpuTransientHeapPoolStatistics output;
		for(const IGpuTransientHeapPool* pool : device.GetTransientHeapPools())
		{
			const GpuTransientHeapPoolStatistics statistics = pool->GetStatistics();
			output.HeapCount += statistics.HeapCount;
			output.Capacity += statistics.Capacity;
		}

		return output;
	}

	/** Returns the source an explicit alias acquire needs to cover the accesses @p acquire orders after. */
	GpuAccessState GetAliasAcquireSource(const render::GpuAliasAcquire& acquire)
	{
		GpuAccessFlags access;
		if(acquire.Source.ReadStages != GpuStageFlag::None)
			access |= GpuAccessFlag::Read;

		if(acquire.Source.WriteStages != GpuStageFlag::None)
			access |= GpuAccessFlag::Write;

		return GpuAccessState(acquire.Source.GetStages(), access);
	}

	/** Returns an explicit alias acquire of @p buffer, placed with @p acquire, into @p destination. */
	GpuBufferBarrier CreateAliasAcquire(const TShared<render::GpuBuffer>& buffer, const render::GpuAliasAcquire& acquire, const GpuAccessState& destination)
	{
		return GpuBufferBarrier(buffer, GetAliasAcquireSource(acquire), destination, GpuBarrierFlag::AliasAcquire);
	}

	/** Returns an explicit alias acquire of @p texture, placed with @p acquire, into @p destination. */
	GpuTextureBarrier CreateAliasAcquire(const TShared<render::Texture>& texture, const render::GpuAliasAcquire& acquire, const GpuAccessState& destination)
	{
		return GpuTextureBarrier(texture, GetAliasAcquireSource(acquire), destination, GpuTextureSubresourceRange::AllSubresources(), GpuBarrierFlag::AliasAcquire);
	}
} // namespace

void GpuBackendTestSuite::TestTransientResourceAllocator()
{
	GpuBackend& backend = GpuBackend::Instance();
	if(backend.GetDeviceCount() == 0)
		return;

	const TShared<GpuDevice> device = backend.GetDevice(0);
	GetRenderThread().PostCommand([this, &device]()
	{
		static constexpr u32 kHeapSizeMegabytes = 4;
		const TransientConfigurationScope configurationScope(kHeapSizeMegabytes, 2, 2);

		GpuSubmissionTimeline timeline;
		timeline.AddSubmission(GpuQueueId(GQT_GRAPHICS, 0), GpuQueueMask::kNone);

		GpuTransientResourceAllocator allocator(*device);

		TextureCreateInformation textureInformation;
		textureInformation.Name = "Transient texture";
		textureInformation.Width = 64;
		textureInformation.Height = 64;
		textureInformation.Format = PF_RGBA8;
		textureInformation.Usage = TextureUsageFlag::RenderTarget;

		const TArrayView<const GpuTransientLastUse> noUses;

		// The same allocations in the same order reuse the textures cached by the previous frame. A texture is placed at most once per
		// scope, so the second allocation creates a new texture at the memory the first one released.
		bool isAliased = false;
		TShared<render::Texture> cachedTextures[2];
		for(u32 frame = 0; frame < 2; frame++)
		{
			GpuTransientScope& scope = allocator.BeginScope(timeline);
			for(u32 textureIndex = 0; textureIndex < 2; textureIndex++)
			{
				const GpuTransientTexture texture = scope.AllocateTexture(textureInformation, 0);
				B3D_TEST_ASSERT(texture.Texture != nullptr)
				if(texture.Texture == nullptr)
					continue;

				if(frame == 0)
				{
					cachedTextures[textureIndex] = texture.Texture;
					isAliased = texture.Texture->HasFixedLocation();
				}
				else
					B3D_TEST_ASSERT(!isAliased || texture.Texture == cachedTextures[textureIndex])

				scope.Release(*texture.Texture, noUses);
			}

			allocator.EndScope();
			device->EndFrame();

			const GpuTransientStatistics statistics = allocator.GetStatistics();
			B3D_TEST_ASSERT(statistics.PlacementCount == 2)
			if(isAliased)
			{
				B3D_TEST_ASSERT(statistics.CacheHits == (frame == 0 ? 0u : 2u))
				B3D_TEST_ASSERT(statistics.CacheMisses == (frame == 0 ? 2u : 0u))
				B3D_TEST_ASSERT(statistics.CachedTextures == 2)
			}
		}

		B3D_TEST_ASSERT(cachedTextures[0] != cachedTextures[1])

		// Textures are placed in transient memory whenever the device has a pool for their memory type
		const u32 textureMemoryType = device->GetMemoryRequirements(textureInformation).MemoryType;
		B3D_TEST_ASSERT(isAliased == (device->GetTransientHeapPool(textureMemoryType) != nullptr))

		if(isAliased)
		{
			// A cached texture whose memory is in use is not reused, so the second and third allocations create new textures. The end of the
			// scope then releases the least recently used textures beyond the cache size.
			TShared<render::Texture> liveTextures[3];
			{
				GpuTransientScope& scope = allocator.BeginScope(timeline);
				for(u32 textureIndex = 0; textureIndex < 3; textureIndex++)
				{
					liveTextures[textureIndex] = scope.AllocateTexture(textureInformation, 0).Texture;
					B3D_TEST_ASSERT(liveTextures[textureIndex] != nullptr)
				}

				B3D_TEST_ASSERT(liveTextures[0] == cachedTextures[0])
				for(const TShared<render::Texture>& texture : liveTextures)
				{
					if(texture != nullptr)
						scope.Release(*texture, noUses);
				}

				allocator.EndScope();

				const GpuTransientStatistics statistics = allocator.GetStatistics();
				B3D_TEST_ASSERT(statistics.PlacementCount == 3)
				B3D_TEST_ASSERT(statistics.CacheHits == 1)
				B3D_TEST_ASSERT(statistics.CacheMisses == 2)
				B3D_TEST_ASSERT(statistics.CachedTextures == 2)
			}

			// The textures created last remain cached
			{
				GpuTransientScope& scope = allocator.BeginScope(timeline);
				const GpuTransientTexture first = scope.AllocateTexture(textureInformation, 0);
				const GpuTransientTexture second = scope.AllocateTexture(textureInformation, 0);
				B3D_TEST_ASSERT(first.Texture == liveTextures[1] && second.Texture == liveTextures[2])

				if(first.Texture != nullptr)
					scope.Release(*first.Texture, noUses);

				if(second.Texture != nullptr)
					scope.Release(*second.Texture, noUses);

				allocator.EndScope();
				B3D_TEST_ASSERT(allocator.GetStatistics().CacheHits == 2)
			}

			// A scope releases the cached textures that went unused for the idle frames. A heap without references is released once it went
			// unused for the idle frames.
			device->EndFrame();
			allocator.BeginScope(timeline);
			allocator.EndScope();
			B3D_TEST_ASSERT(allocator.GetStatistics().CachedTextures == 2)

			device->EndFrame();
			allocator.BeginScope(timeline);
			allocator.EndScope();
			B3D_TEST_ASSERT(allocator.GetStatistics().CachedTextures == 0)

			// The renderer's context releases the transient resources of the renderer's start-up frame graphs as its frames advance,
			// which they do not in this test
			GpuWorkContext& rendererContext = render::GetRenderer()->GetGpuContext();
			rendererContext.SubmitTransferCommandBuffers();
			rendererContext.AdvanceFrame();

			for(u32 frameIndex = 0; frameIndex <= gGpuTransientIdleFrames; frameIndex++)
				device->EndFrame();

			GpuTransientHeapPoolStatistics heapStatistics = GetTransientHeapStatistics(*device);
			B3D_TEST_ASSERT(heapStatistics.HeapCount == 0 && heapStatistics.Capacity == 0)

			// Scopes of two contexts open at the same time each see the heaps as free, so their textures share the heap one of them creates
			{
				const TShared<GpuWorkContext> firstContext = GpuWorkContext::Create(*device);
				const TShared<GpuWorkContext> secondContext = GpuWorkContext::Create(*device);
				GpuTransientResourceAllocator& firstAllocator = firstContext->GetTransientResourceAllocator();
				GpuTransientResourceAllocator& secondAllocator = secondContext->GetTransientResourceAllocator();

				GpuTransientScope& firstScope = firstAllocator.BeginScope(timeline);
				GpuTransientScope& secondScope = secondAllocator.BeginScope(timeline);
				const GpuTransientTexture firstTexture = firstScope.AllocateTexture(textureInformation, 0);
				const GpuTransientTexture secondTexture = secondScope.AllocateTexture(textureInformation, 0);
				B3D_TEST_ASSERT(firstTexture.Texture != nullptr && secondTexture.Texture != nullptr && firstTexture.Texture != secondTexture.Texture)

				heapStatistics = GetTransientHeapStatistics(*device);
				B3D_TEST_ASSERT(heapStatistics.HeapCount == 1 && heapStatistics.Capacity == (u64)kHeapSizeMegabytes * 1024 * 1024)

				if(firstTexture.Texture != nullptr)
					firstScope.Release(*firstTexture.Texture, noUses);

				if(secondTexture.Texture != nullptr)
					secondScope.Release(*secondTexture.Texture, noUses);

				firstAllocator.EndScope();
				secondAllocator.EndScope();
			}

			device->EndFrame();
		}

		// Validation: a resource not released before the end of its scope is reported
		{
			GpuTransientScope& scope = allocator.BeginScope(timeline);
			const GpuTransientTexture texture = scope.AllocateTexture(textureInformation, 0);
			B3D_TEST_ASSERT(texture.Texture != nullptr)
			{
				LoggingScope logs(*this);
				logs.ExpectError("A transient resource was not released before the end of the scope that placed it.");
				allocator.EndScope();
			}

			device->EndFrame();
		}

		// Validation: resources accessed by the CPU, and textures first used on a transfer queue, cannot be transient
		{
			GpuSubmissionTimeline transferTimeline;
			transferTimeline.AddSubmission(GpuQueueId(GQT_TRANSFER, 0), GpuQueueMask::kNone);

			TextureCreateInformation cpuTextureInformation = textureInformation;
			cpuTextureInformation.Name = "Transient CPU texture";
			cpuTextureInformation.Usage = TextureUsageFlag::StoreOnCPUWithGPUAccess;

			GpuTransientScope& scope = allocator.BeginScope(transferTimeline);
			{
				LoggingScope logs(*this);
				logs.ExpectError("The first use of transient texture 'Transient texture' cannot be on a transfer queue.");
				logs.ExpectError("Transient texture 'Transient CPU texture' must only be accessed by the GPU.");
				logs.ExpectError("Transient buffers must only be accessed by the GPU.");

				B3D_TEST_ASSERT(scope.AllocateTexture(textureInformation, 0).Texture == nullptr)
				B3D_TEST_ASSERT(scope.AllocateTexture(cpuTextureInformation, 0).Texture == nullptr)
				B3D_TEST_ASSERT(scope.AllocateBuffer(GpuBufferCreateInformation::CreateStagingWrite(1024), 0).Buffer == nullptr)
			}

			allocator.EndScope();
			device->EndFrame();
		}

		// With transient resources disabled, textures are created in persistent memory and never cached
		{
			gGpuTransientResources.Set(false);
			const u32 cachedTextureCount = allocator.GetStatistics().CachedTextures;

			GpuTransientScope& scope = allocator.BeginScope(timeline);
			const GpuTransientTexture texture = scope.AllocateTexture(textureInformation, 0);
			B3D_TEST_ASSERT(texture.Texture != nullptr)
			if(texture.Texture != nullptr)
			{
				B3D_TEST_ASSERT(!texture.Texture->HasFixedLocation())
				scope.Release(*texture.Texture, noUses);
			}

			allocator.EndScope();
			device->EndFrame();
			gGpuTransientResources.Set(true);

			const GpuTransientStatistics statistics = allocator.GetStatistics();
			B3D_TEST_ASSERT(statistics.PlacementCount == 1)
			B3D_TEST_ASSERT(statistics.CacheHits == 0)
			B3D_TEST_ASSERT(statistics.CacheMisses == 0)

			// The scope may have released textures that went unused for the idle frames
			B3D_TEST_ASSERT(statistics.CachedTextures <= cachedTextureCount)
		}
	}, "GpuBackendTestSuite::TestTransientResourceAllocator", true);
}

void GpuBackendTestSuite::TestTransientResourceExecution()
{
	GpuBackend& backend = GpuBackend::Instance();
	if(backend.GetDeviceCount() == 0)
		return;

	const TShared<GpuDevice> device = backend.GetDevice(0);
	GetRenderThread().PostCommand([this, &device]()
	{
		static constexpr u32 kSize = 64;
		const GpuQueueId graphics(GQT_GRAPHICS, 0);
		const GpuQueueId compute(GQT_COMPUTE, 0);
		B3D_TEST_ASSERT(device->GetQueueCount(GQT_COMPUTE) > 0)

		TextureCreateInformation textureInformation;
		textureInformation.Name = "Transient storage texture";
		textureInformation.Width = kSize;
		textureInformation.Height = kSize;
		textureInformation.Format = PF_RGBA8;
		textureInformation.Usage = TextureUsageFlag::AllowUnorderedAccessOnTheGPU;

		const GpuBufferCreateInformation bufferInformation = GpuBufferCreateInformation::CreateStructuredStorage(sizeof(u32), kSize * kSize);
		const u32 bufferBytes = kSize * kSize * sizeof(u32);

		// Staging buffers hold one pattern per resource. Their size covers a texture's staging pitch.
		const TShared<render::Texture> pitchTexture = device->CreateTexture(textureInformation);
		const ImageSubresourcePitch pitch = pitchTexture->GetStagingBufferPitchForSubresource(0, 0);
		const u32 stagingElementCount = pitch.RowPitch * pitch.SliceHeight;

		// Each pattern stores the pattern index in the high bits, and the element index in the low bits
		auto fnPatternValue = [](u32 pattern, u32 elementIndex) { return (pattern << 24) | elementIndex; };

		constexpr u32 kResourceCount = 4;
		TShared<render::GpuBuffer> uploads[kResourceCount];
		TShared<render::GpuBuffer> readbacks[kResourceCount];
		for(u32 resourceIndex = 0; resourceIndex < kResourceCount; resourceIndex++)
		{
			uploads[resourceIndex] = device->CreateGpuBuffer(GpuBufferCreateInformation::CreateStagingWrite(stagingElementCount * sizeof(u32)));
			readbacks[resourceIndex] = device->CreateGpuBuffer(GpuBufferCreateInformation::CreateStagingRead(stagingElementCount * sizeof(u32)));

			const render::GpuBufferMappedScope mapping = uploads[resourceIndex]->Map(GpuMapOption::Write);
			B3D_TEST_ASSERT(mapping.IsValid())
			if(!mapping.IsValid())
				return;

			u32* elements = static_cast<u32*>(mapping.GetMappedMemory());
			for(u32 elementIndex = 0; elementIndex < stagingElementCount; elementIndex++)
				elements[elementIndex] = fnPatternValue(resourceIndex + 1, elementIndex);
		}

		// Checks that the readback of a resource holds the resource's pattern in every element
		auto fnCheckReadback = [this, &readbacks, &pitch, &fnPatternValue](u32 resourceIndex, bool isTexture)
		{
			const render::GpuBufferMappedScope mapping = readbacks[resourceIndex]->Map(GpuMapOption::Read);
			B3D_TEST_ASSERT(mapping.IsValid())
			if(!mapping.IsValid())
				return;

			const u32* elements = static_cast<const u32*>(mapping.GetMappedMemory());
			bool matches = true;
			for(u32 row = 0; row < kSize; row++)
			{
				for(u32 column = 0; column < kSize; column++)
				{
					const u32 elementIndex = isTexture ? row * pitch.RowPitch + column : row * kSize + column;
					matches &= elements[elementIndex] == fnPatternValue(resourceIndex + 1, elementIndex);
				}
			}

			B3D_TEST_ASSERT(matches)
		};

		const GpuAccessState transferWrite(GpuStageFlag::Transfer, GpuAccessFlag::Write, GpuImageLayout::TransferDestination);
		const GpuAccessState transferRead(GpuStageFlag::Transfer, GpuAccessFlag::Read, GpuImageLayout::TransferSource);
		const GpuAccessState hostRead(GpuStageFlag::Host, GpuAccessFlag::Read);

		// Returns the source of the alias acquire that starts a new lifetime of a resource. Earlier work on the queue may still access memory
		// that no earlier resource of the scope used.
		auto fnGetAcquireSource = [](const render::GpuCommandBuffer& commands, const GpuAliasAcquire& acquire)
		{
			GpuAccessState source = GetAliasAcquireSource(acquire);
			if(acquire.HasUnknownSourceAccess)
			{
				source.Stages |= GpuBackendUtility::GetQueueStageFlags(commands.GetQueueType()) & ~GpuStageFlags(GpuStageFlag::Host);
				source.Access |= GpuAccessFlag::Write;
			}

			return source;
		};

		// Starts a new lifetime of a buffer, writes it with the resource's pattern and copies it to the resource's readback
		auto fnWriteAndReadBuffer = [&](render::GpuCommandBuffer& commands, const GpuTransientBuffer& buffer, u32 resourceIndex)
		{
			const GpuBufferBarrier acquireBarriers[] = { GpuBufferBarrier(buffer.Buffer, fnGetAcquireSource(commands, buffer.Acquire), transferWrite, GpuBarrierFlag::AliasAcquire),
				GpuBufferBarrier(readbacks[resourceIndex], GpuAccessState(), transferWrite) };
			commands.IssueBarriers(GpuBarriers(TArrayView<const GpuBufferBarrier>(acquireBarriers, 2)));
			commands.CopyBufferToBuffer(uploads[resourceIndex], buffer.Buffer, 0, 0, bufferBytes);
			commands.IssueBarriers(GpuBufferBarrier(buffer.Buffer, transferWrite, transferRead));
			commands.CopyBufferToBuffer(buffer.Buffer, readbacks[resourceIndex], 0, 0, bufferBytes);
			commands.IssueBarriers(GpuBufferBarrier(readbacks[resourceIndex], transferWrite, hostRead));
		};

		// The same for a texture
		auto fnWriteAndReadTexture = [&](render::GpuCommandBuffer& commands, const GpuTransientTexture& texture, u32 resourceIndex)
		{
			commands.IssueBarriers(GpuTextureBarrier(texture.Texture, fnGetAcquireSource(commands, texture.Acquire), transferWrite, GpuTextureSubresourceRange::AllSubresources(),
				GpuBarrierFlag::AliasAcquire));
			commands.IssueBarriers(GpuBufferBarrier(readbacks[resourceIndex], GpuAccessState(), transferWrite));
			commands.CopyBufferToTexture(uploads[resourceIndex], texture.Texture, 0, 0, 0);
			commands.IssueBarriers(GpuTextureBarrier(texture.Texture, transferWrite, transferRead));
			commands.CopyTextureToBuffer(texture.Texture, readbacks[resourceIndex], 0, 0);
			commands.IssueBarriers(GpuBufferBarrier(readbacks[resourceIndex], transferWrite, hostRead));
		};

		GpuAccessScope transferAccess;
		transferAccess.Add(GpuStageFlag::Transfer, GpuAccessFlag::Read | GpuAccessFlag::Write);

		const TShared<GpuWorkContext> context = GpuWorkContext::Create(*device);
		const TShared<render::GpuCommandBufferPool> graphicsPool = GpuBarrierTestUtility::CreateCommandBufferPool(*device, GQT_GRAPHICS);
		const TShared<render::GpuCommandBufferPool> producerPool = GpuBarrierTestUtility::CreateCommandBufferPool(*device, GQT_COMPUTE);

		// The producer scope uses the compute queue first, then the graphics queue after a wait for the compute queue. The consumer scope
		// uses the graphics queue only.
		GpuSubmissionTimeline producerTimeline;
		producerTimeline.AddSubmission(compute, GpuQueueMask::kNone);
		producerTimeline.AddSubmission(graphics, GpuQueueMask(compute));

		GpuSubmissionTimeline consumerTimeline;
		consumerTimeline.AddSubmission(graphics, GpuQueueMask::kNone);

		const TransientConfigurationScope configurationScope(4, gGpuTransientIdleFrames, gGpuTransientMaxCachedTextures);
		GpuTransientResourceAllocator& allocator = context->GetTransientResourceAllocator();

		for(u32 frame = 0; frame < 2; frame++)
		{
			// The second buffer and the texture are placed over the memory released by the resources before them
			GpuTransientScope& producerScope = allocator.BeginScope(producerTimeline);
			const GpuTransientLastUse producerLastUse(0, transferAccess);
			const GpuTransientLastUse graphicsLastUse(1, transferAccess);

			const GpuTransientBuffer first = producerScope.AllocateBuffer(bufferInformation, 0);
			if(first.Buffer != nullptr)
				producerScope.Release(*first.Buffer, TArrayView<const GpuTransientLastUse>(&producerLastUse, 1));

			const GpuTransientBuffer second = producerScope.AllocateBuffer(bufferInformation, 1);
			if(second.Buffer != nullptr)
				producerScope.Release(*second.Buffer, TArrayView<const GpuTransientLastUse>(&graphicsLastUse, 1));

			const GpuTransientTexture texture = producerScope.AllocateTexture(textureInformation, 1);
			if(texture.Texture != nullptr)
				producerScope.Release(*texture.Texture, TArrayView<const GpuTransientLastUse>(&graphicsLastUse, 1));

			const bool isAllocated = first.Buffer != nullptr && second.Buffer != nullptr && texture.Texture != nullptr;
			B3D_TEST_ASSERT(isAllocated)
			if(!isAllocated)
			{
				allocator.EndScope();
				return;
			}

			IGpuTransientHeapPool* bufferPool = device->GetTransientHeapPool(device->GetMemoryRequirements(bufferInformation).MemoryType);
			IGpuTransientHeapPool* texturePool = device->GetTransientHeapPool(device->GetMemoryRequirements(textureInformation).MemoryType);

			const bool isBufferAliased = first.Buffer->HasFixedLocation();
			B3D_TEST_ASSERT(isBufferAliased == (bufferPool != nullptr))
			B3D_TEST_ASSERT(first.Buffer != second.Buffer)

			// Accesses on the compute queue are ordered by the queue wait, so they are not part of the acquire's source
			if(isBufferAliased)
				B3D_TEST_ASSERT(second.Acquire.Source.GetStages() == GpuStageFlag::None)

			// A texture in the same pool as the buffers is placed over the second buffer, after its transfers on the same queue
			if(isBufferAliased && bufferPool == texturePool)
				B3D_TEST_ASSERT(texture.Acquire.Source.GetStages() == GpuStageFlag::Transfer)

			const TShared<render::GpuCommandBuffer> producerCommands = producerPool->Create(GpuCommandBufferCreateInformation::Create("Transient producer"));
			fnWriteAndReadBuffer(*producerCommands, first, 0);
			context->SubmitCommandBuffer(producerCommands, GpuQueueMask::kNone);

			const TShared<render::GpuCommandBuffer> graphicsCommands = graphicsPool->Create(GpuCommandBufferCreateInformation::Create("Transient producer on graphics"));
			fnWriteAndReadBuffer(*graphicsCommands, second, 1);
			fnWriteAndReadTexture(*graphicsCommands, texture, 2);
			context->SubmitCommandBuffer(graphicsCommands, GpuQueueMask(compute));
			allocator.EndScope();

			// Aliasing keeps the peak memory use of the scope below the total size of its allocations. The second frame reuses every cached
			// resource.
			GpuTransientStatistics statistics = allocator.GetStatistics();
			B3D_TEST_ASSERT(statistics.PlacementCount == 3)
			if(isBufferAliased)
				B3D_TEST_ASSERT(statistics.LastScope.PeakUsed < statistics.LastScope.RequestedBytes)

			if(frame == 1)
				B3D_TEST_ASSERT(statistics.CacheMisses == 0)

			// The consumer scope reuses the memory of the producer scope, and with it the first buffer's cached resource
			GpuTransientScope& consumerScope = allocator.BeginScope(consumerTimeline);
			const GpuTransientLastUse consumerLastUse(0, transferAccess);
			const GpuTransientBuffer consumer = consumerScope.AllocateBuffer(bufferInformation, 0);
			B3D_TEST_ASSERT(consumer.Buffer != nullptr)
			if(consumer.Buffer == nullptr)
			{
				allocator.EndScope();
				return;
			}

			consumerScope.Release(*consumer.Buffer, TArrayView<const GpuTransientLastUse>(&consumerLastUse, 1));
			B3D_TEST_ASSERT(!isBufferAliased || consumer.Buffer == first.Buffer)

			const TShared<render::GpuCommandBuffer> consumerCommands = graphicsPool->Create(GpuCommandBufferCreateInformation::Create("Transient consumer"));
			fnWriteAndReadBuffer(*consumerCommands, consumer, 3);
			context->SubmitCommandBuffer(consumerCommands, GpuQueueMask::kNone);
			allocator.EndScope();

			statistics = allocator.GetStatistics();
			B3D_TEST_ASSERT(statistics.PlacementCount == 1)
			B3D_TEST_ASSERT(statistics.CacheHits == (isBufferAliased ? 1u : 0u))

			device->WaitUntilIdle();
			device->EndFrame();

			fnCheckReadback(0, false);
			fnCheckReadback(1, false);
			fnCheckReadback(2, true);
			fnCheckReadback(3, false);
		}
	}, "GpuBackendTestSuite::TestTransientResourceExecution", true);
}

void GpuBackendTestSuite::TestTransientResourcePlacement()
{
	GpuBackend& backend = GpuBackend::Instance();
	if(backend.GetDeviceCount() == 0)
		return;

	const TShared<GpuDevice> device = backend.GetDevice(0);
	GetRenderThread().PostCommand([this, &device]()
	{
		const TransientConfigurationScope configurationScope(4, 2, 2);

		GpuSubmissionTimeline timeline;
		timeline.AddSubmission(GpuQueueId(GQT_GRAPHICS, 0), GpuQueueMask::kNone);

		GpuTransientResourceAllocator allocator(*device);

		TextureCreateInformation textureInformation;
		textureInformation.Name = "Placed texture";
		textureInformation.Width = 64;
		textureInformation.Height = 64;
		textureInformation.Format = PF_RGBA8;
		textureInformation.Usage = TextureUsageFlag::RenderTarget;

		const TArrayView<const GpuTransientLastUse> noUses;
		const bool isAliased = device->GetTransientHeapPool(device->GetMemoryRequirements(textureInformation).MemoryType) != nullptr;

		// Unplaced textures have no memory. Placement creates the native resource of a new texture.
		TShared<render::Texture> textures[2];
		{
			for(TShared<render::Texture>& texture : textures)
			{
				texture = allocator.GetOrCreateTexture(textureInformation);
				B3D_TEST_ASSERT(texture != nullptr)
				if(texture == nullptr)
					return;

#if B3D_BUILD_TYPE_DEVELOPMENT
				B3D_TEST_ASSERT(texture->IsUnplaced())
#endif
			}

			B3D_TEST_ASSERT(textures[0] != textures[1])

			GpuTransientScope& scope = allocator.BeginScope(timeline);
			for(const TShared<render::Texture>& texture : textures)
			{
				const GpuTransientPlacement placement = scope.PlaceTexture(*texture, 0);
				B3D_TEST_ASSERT(!placement.Recreated)
				B3D_TEST_ASSERT(texture->HasFixedLocation() == isAliased)

#if B3D_BUILD_TYPE_DEVELOPMENT
				B3D_TEST_ASSERT(!texture->IsUnplaced())
#endif

				scope.Release(*texture, noUses);
			}

			allocator.EndScope();
			device->EndFrame();

			const GpuTransientStatistics statistics = allocator.GetStatistics();
			B3D_TEST_ASSERT(statistics.PlacementCount == 2)
			B3D_TEST_ASSERT(statistics.CacheMisses == (isAliased ? 2u : 0u))
			B3D_TEST_ASSERT(statistics.CachedTextures == (isAliased ? 2u : 0u))
		}

		// Validation: only unplaced textures can be placed, and their first use cannot be on a transfer queue
		{
			GpuSubmissionTimeline transferTimeline;
			transferTimeline.AddSubmission(GpuQueueId(GQT_TRANSFER, 0), GpuQueueMask::kNone);

			const TShared<render::Texture> texture = allocator.GetOrCreateTexture(textureInformation);
			B3D_TEST_ASSERT(texture != nullptr)

			LoggingScope logs(*this);
			logs.ExpectError("Only an unplaced transient resource can be placed.");
			logs.ExpectError("The first use of transient texture 'Placed texture' cannot be on a transfer queue.");

			GpuTransientScope& scope = allocator.BeginScope(timeline);
			scope.PlaceTexture(*device->CreateTexture(textureInformation), 0);
			allocator.EndScope();

			// The texture remains unplaced
			GpuTransientScope& transferScope = allocator.BeginScope(transferTimeline);
			if(texture != nullptr)
				transferScope.PlaceTexture(*texture, 0);

			allocator.EndScope();
			if(texture != nullptr)
				allocator.Return(*texture);

			device->EndFrame();
		}

		if(!isAliased)
			return;

		// The same requests in the same order return the same textures, which keep their memory
		{
			const TShared<render::Texture> first = allocator.GetOrCreateTexture(textureInformation);
			const TShared<render::Texture> second = allocator.GetOrCreateTexture(textureInformation);
			B3D_TEST_ASSERT(first == textures[0] && second == textures[1])

			GpuTransientScope& scope = allocator.BeginScope(timeline);
			for(const TShared<render::Texture>& texture : textures)
			{
				B3D_TEST_ASSERT(!scope.PlaceTexture(*texture, 0).Recreated)
				scope.Release(*texture, noUses);
			}

			allocator.EndScope();
			device->EndFrame();

			const GpuTransientStatistics statistics = allocator.GetStatistics();
			B3D_TEST_ASSERT(statistics.CacheHits == 2)
			B3D_TEST_ASSERT(statistics.CacheMisses == 0)
			B3D_TEST_ASSERT(statistics.Relocations == 0)
		}

		// Both textures were cached at the memory the first one released. While the first one holds it, the second one is recreated
		// at other memory.
		{
			for(const TShared<render::Texture>& texture : textures)
				B3D_TEST_ASSERT(allocator.GetOrCreateTexture(textureInformation) == texture)

			GpuTransientScope& scope = allocator.BeginScope(timeline);
			B3D_TEST_ASSERT(!scope.PlaceTexture(*textures[0], 0).Recreated)
			B3D_TEST_ASSERT(scope.PlaceTexture(*textures[1], 0).Recreated)
			B3D_TEST_ASSERT(textures[1]->HasFixedLocation())

			for(const TShared<render::Texture>& texture : textures)
				scope.Release(*texture, noUses);

			allocator.EndScope();
			device->EndFrame();

			const GpuTransientStatistics statistics = allocator.GetStatistics();
			B3D_TEST_ASSERT(statistics.CacheHits == 1)
			B3D_TEST_ASSERT(statistics.Relocations == 1)
			B3D_TEST_ASSERT(statistics.CacheMisses == 0)
			B3D_TEST_ASSERT(statistics.CachedTextures == 2)
		}

		// A held texture is neither returned again nor evicted, even though it was cached first. Eviction beyond the cache size
		// releases the oldest of the others.
		TShared<render::Texture> survivor;
		{
			const TShared<render::Texture> held = allocator.GetOrCreateTexture(textureInformation);
			B3D_TEST_ASSERT(held == textures[0])

			TShared<render::Texture> placed[3];
			for(TShared<render::Texture>& texture : placed)
			{
				texture = allocator.GetOrCreateTexture(textureInformation);
				B3D_TEST_ASSERT(texture != nullptr && texture != held)
				if(texture == nullptr)
					return;
			}

			B3D_TEST_ASSERT(placed[0] == textures[1])

			GpuTransientScope& scope = allocator.BeginScope(timeline);
			for(const TShared<render::Texture>& texture : placed)
			{
				scope.PlaceTexture(*texture, 0);
				scope.Release(*texture, noUses);
			}

			allocator.EndScope();
			device->EndFrame();
			B3D_TEST_ASSERT(allocator.GetStatistics().CachedTextures == 2)

			// The held texture and the last placed one remain
			allocator.Return(*held);
			B3D_TEST_ASSERT(allocator.GetOrCreateTexture(textureInformation) == held)
			B3D_TEST_ASSERT(allocator.GetOrCreateTexture(textureInformation) == placed[2])
			allocator.Return(*held);
			allocator.Return(*placed[2]);
			survivor = placed[2];
		}

		// A detached texture leaves the cache, and keeps its native resource until it is moved off transient memory
		{
			const TShared<render::Texture> detached = allocator.GetOrCreateTexture(textureInformation);
			B3D_TEST_ASSERT(detached == textures[0])
			allocator.Detach(*detached);
			B3D_TEST_ASSERT(allocator.GetStatistics().CachedTextures == 1)
			B3D_TEST_ASSERT(detached->HasFixedLocation())

#if B3D_BUILD_TYPE_DEVELOPMENT
			B3D_TEST_ASSERT(!detached->IsUnplaced())
#endif

			const u32 memoryType = device->GetMemoryRequirements(textureInformation).MemoryType;
			detached->SetAllocation(GpuAllocation::CreatePending(device->GetPersistentAllocator(memoryType)), GpuObjectCreateFlag::None);
			B3D_TEST_ASSERT(!detached->HasFixedLocation())

			const TShared<render::Texture> next = allocator.GetOrCreateTexture(textureInformation);
			B3D_TEST_ASSERT(next == survivor)
			allocator.Return(*next);

			// A texture created anew has nothing in the cache to remove
			const TShared<render::Texture> held = allocator.GetOrCreateTexture(textureInformation);
			const TShared<render::Texture> created = allocator.GetOrCreateTexture(textureInformation);
			B3D_TEST_ASSERT(created != nullptr && created != held && created != detached)
			if(created != nullptr)
				allocator.Detach(*created);

			allocator.Return(*held);
			B3D_TEST_ASSERT(allocator.GetStatistics().CachedTextures == 1)
		}

		device->WaitUntilIdle();
	}, "GpuBackendTestSuite::TestTransientResourcePlacement", true);
}

#if B3D_BUILD_TYPE_DEVELOPMENT
void GpuBackendTestSuite::TestBarrierResourceValidation()
{
	GpuBackend& backend = GpuBackend::Instance();
	if(backend.GetDeviceCount() == 0)
		return;

	// Shaders are resources, which are created outside of the render thread
	const TShared<GpuDevice> device = backend.GetDevice(0);
	const TShared<GpuComputePipelineState> pipeline = CreateTestDispatchPipeline(*device);

	// The D3D12 debug layer tracks layouts natively and fails to close a command list that copies into a texture in another layout
	const bool recordsMisdeclaredLayouts = String(backend.GetBackendName()) != "bsfD3D12GpuBackend";

	GetRenderThread().PostCommand([this, &device, &pipeline, recordsMisdeclaredLayouts]()
	{
		static constexpr u32 kSize = 16;
		const GpuAccessFlags readWrite = GpuAccessFlag::Read | GpuAccessFlag::Write;
		const GpuAccessState transferWrite(GpuStageFlag::Transfer, GpuAccessFlag::Write, GpuImageLayout::TransferDestination);
		const GpuAccessState transferRead(GpuStageFlag::Transfer, GpuAccessFlag::Read, GpuImageLayout::TransferSource);
		const GpuAccessState computeReadWrite(GpuStageFlag::ComputeShaderNonUniform, readWrite, GpuImageLayout::General);

		// Declares accesses in other stages than the ones the commands access in
		const GpuAccessState host(GpuStageFlag::Host, readWrite);

		const GpuBufferCreateInformation bufferInformation = GpuBufferCreateInformation::CreateStructuredStorage(sizeof(u32), kSize * kSize,
			GpuBufferFlag::StoreOnGPU | GpuBufferFlag::AllowUnorderedAccessOnTheGPU);

		TextureCreateInformation textureInformation;
		textureInformation.Width = kSize;
		textureInformation.Height = kSize;
		textureInformation.Format = PF_R32U;
		textureInformation.Usage = TextureUsageFlag::AllowUnorderedAccessOnTheGPU;

		const auto fnCreateBuffer = [&device, &bufferInformation](const char* name)
		{
			const TShared<render::GpuBuffer> buffer = device->CreateGpuBuffer(bufferInformation);
			buffer->SetName(name);
			return buffer;
		};

		const auto fnCreateTexture = [&device, &textureInformation](const char* name)
		{
			TextureCreateInformation information = textureInformation;
			information.Name = name;
			return device->CreateTexture(information);
		};

		const TShared<GpuWorkContext> context = GpuWorkContext::Create(*device);
		const TShared<GpuCommandBufferPool> pool = GpuBarrierTestUtility::CreateCommandBufferPool(*device);

		// The command buffers share one validator, as a frame graph's do, so resource states carry over from one to the next
		GpuBarrierValidator validator;
		const TShared<GpuCommandBuffer> commandBuffer = pool->Create(GpuCommandBufferCreateInformation::Create("Barrier resource validation"));
		commandBuffer->SetValidator(&validator);

		GpuSubmissionTimeline timeline;
		timeline.AddSubmission(GpuQueueId(GQT_GRAPHICS, 0), GpuQueueMask::kNone);

		GpuTransientResourceAllocator allocator(*device);
		const TArrayView<const GpuTransientLastUse> noUses;

		GpuAccessScope transferAccess;
		transferAccess.Add(GpuStageFlag::Transfer, GpuAccessFlag::Write);
		const GpuTransientLastUse transferLastUse(0, transferAccess);

		// A cached texture returned again keeps its native resource, but has no memory until it is placed
		TShared<render::Texture> unplaced;
		if(device->GetTransientHeapPool(device->GetMemoryRequirements(textureInformation).MemoryType) != nullptr)
		{
			TextureCreateInformation unplacedInformation = textureInformation;
			unplacedInformation.Name = "Unplaced";

			GpuTransientScope& scope = allocator.BeginScope(timeline);
			const GpuTransientTexture cached = scope.AllocateTexture(unplacedInformation, 0);
			if(cached.Texture != nullptr)
				scope.Release(*cached.Texture, noUses);

			allocator.EndScope();

			unplaced = allocator.GetOrCreateTexture(unplacedInformation);
			B3D_TEST_ASSERT(unplaced != nullptr && unplaced == cached.Texture)
		}

		// Three buffers on the same memory, each placed over the previous one
		GpuTransientScope& scope = allocator.BeginScope(timeline);
		const GpuTransientBuffer superseded = scope.AllocateBuffer(bufferInformation, 0);
		if(superseded.Buffer != nullptr)
			scope.Release(*superseded.Buffer, TArrayView<const GpuTransientLastUse>(&transferLastUse, 1));

		const GpuTransientBuffer acquired = scope.AllocateBuffer(bufferInformation, 0);
		if(acquired.Buffer != nullptr)
			scope.Release(*acquired.Buffer, TArrayView<const GpuTransientLastUse>(&transferLastUse, 1));

		const GpuTransientBuffer misacquired = scope.AllocateBuffer(bufferInformation, 0);
		if(misacquired.Buffer != nullptr)
			scope.Release(*misacquired.Buffer, noUses);

		B3D_TEST_ASSERT(superseded.Buffer != nullptr && acquired.Buffer != nullptr && misacquired.Buffer != nullptr)
		const bool hasAliasedBuffers = superseded.Buffer != nullptr && acquired.Buffer != nullptr && misacquired.Buffer != nullptr &&
			superseded.Buffer->HasFixedLocation() && superseded.Buffer->GetGpuResource() != nullptr;

		if(hasAliasedBuffers)
		{
			superseded.Buffer->SetName("Superseded");
			acquired.Buffer->SetName("Acquired");
			misacquired.Buffer->SetName("AliasAcquireSource");
			B3D_TEST_ASSERT(acquired.Acquire.Source.WriteStages == GpuStageFlag::Transfer)
			B3D_TEST_ASSERT(misacquired.Acquire.Source.WriteStages == GpuStageFlag::Transfer)

			validator.DeclareAliasAcquire(*superseded.Buffer, superseded.Acquire);
			validator.DeclareAliasAcquire(*acquired.Buffer, acquired.Acquire);
			validator.DeclareAliasAcquire(*misacquired.Buffer, misacquired.Acquire);
		}

		// Two resources on the same memory, outside of the allocator. The successor supersedes the predecessor.
		const TShared<render::GpuBuffer> predecessor = fnCreateBuffer("TogetherPredecessor");
		const TShared<render::GpuBuffer> successor = fnCreateBuffer("TogetherSuccessor");
		IGpuResource* const successorPredecessors[] = { predecessor->GetGpuResource() };
		GpuAliasAcquire successorAcquire;
		successorAcquire.Predecessors = TArrayView<IGpuResource* const>(successorPredecessors, 1);
		validator.DeclareAliasAcquire(*successor, successorAcquire);

		const TShared<render::GpuBuffer> beforeAliasAcquire = fnCreateBuffer("BeforeAliasAcquire");
		validator.DeclareAliasAcquire(*beforeAliasAcquire, GpuAliasAcquire());

		LoggingScope logs(*this);
		if(hasAliasedBuffers)
		{
			logs.ExpectError("The resource was used after an alias acquire of another resource took over its memory. Buffer: 'Superseded'.");
			logs.ExpectError("The source of an alias acquire is missing stages or writes of the source of its GpuAliasAcquire. Buffer: 'AliasAcquireSource'.");

			// Vulkan synchronization validation reports the copy into the superseded buffer
			logs.IgnoreError("[Superseded], which was previously written by another vkCmdCopyBuffer command");
		}

		logs.ExpectError("A barrier of an earlier resource on the memory is issued together with the alias acquire that supersedes it. Resource: 'TogetherSuccessor'.");
		logs.ExpectError("The resource was used before its alias acquire. Buffer: 'BeforeAliasAcquire'.");

		if(unplaced != nullptr)
			logs.ExpectError("The resource was used while it is unplaced, without memory from its transient resource allocator. Place it first. Image: 'Unplaced'.");

		logs.ExpectError("The source of a barrier is missing stages that accessed the resource since the previous barrier. Buffer: 'CarriedState'.");
		logs.ExpectError("The resource was accessed in stages its resting state does not allow, without a barrier. Buffer: 'RestingBuffer'.");
		if(recordsMisdeclaredLayouts)
			logs.ExpectError("The resource was accessed in stages its resting state does not allow, without a barrier. Image: 'RestingTexture'.");

		if(pipeline != nullptr)
		{
			const String hazard = "The resource was accessed by a draw or dispatch after another draw or dispatch wrote it, without a barrier in between.";
			logs.ExpectError(hazard + " Buffer: 'InPassHazard'.");
			logs.ExpectError(hazard + " Image: 'InPassHazardTexture'.");
		}

		const TShared<render::GpuBuffer> source = fnCreateBuffer("Source");
		commandBuffer->IssueBarriers(GpuBufferBarrier(source, GpuAccessState(), transferRead));

		// An alias acquire ends the lifetime of the earlier resources on the memory, and must cover the accesses they were released with
		if(hasAliasedBuffers)
		{
			commandBuffer->IssueBarriers(CreateAliasAcquire(superseded.Buffer, superseded.Acquire, transferWrite));
			commandBuffer->CopyBufferToBuffer(source, superseded.Buffer, 0, 0, sizeof(u32));
			commandBuffer->IssueBarriers(CreateAliasAcquire(acquired.Buffer, acquired.Acquire, transferWrite));
			commandBuffer->CopyBufferToBuffer(source, acquired.Buffer, 0, 0, sizeof(u32));
			commandBuffer->CopyBufferToBuffer(source, superseded.Buffer, 0, 0, sizeof(u32));

			commandBuffer->IssueBarriers(GpuBufferBarrier(misacquired.Buffer, GpuAccessState(), transferWrite, GpuBarrierFlag::AliasAcquire));
		}

		// The barriers of one call are not ordered against each other, so an alias acquire must not share a call with a barrier of a
		// resource it supersedes
		{
			const GpuBufferBarrier barriers[] = { GpuBufferBarrier(predecessor, GpuAccessState(), transferWrite),
				GpuBufferBarrier(successor, GpuAccessState(), transferWrite, GpuBarrierFlag::AliasAcquire) };
			commandBuffer->IssueBarriers(GpuBarriers(TArrayView<const GpuBufferBarrier>(barriers, 2)));
		}

		// A resource with a declared alias acquire must not be used before it
		commandBuffer->CopyBufferToBuffer(source, beforeAliasAcquire, 0, 0, sizeof(u32));

		// An unplaced texture must be placed before its first use
		if(unplaced != nullptr)
			commandBuffer->IssueBarriers(GpuTextureBarrier(unplaced, GpuAccessState(), transferWrite));

		// With resting state validation, resources that no barrier covers must be accessed within their resting state. The
		// validator identifies resources by address, so every resource must outlive the recording.
		const TShared<render::GpuBuffer> restingBuffer = fnCreateBuffer("RestingBuffer");
		const TShared<render::Texture> restingTexture = fnCreateTexture("RestingTexture");
		const ImageSubresourcePitch pitch = restingTexture->GetStagingBufferPitchForSubresource(0, 0);
		const TShared<render::GpuBuffer> upload = device->CreateGpuBuffer(GpuBufferCreateInformation::CreateStagingWrite(pitch.RowPitch * pitch.SliceHeight * sizeof(u32)));
		upload->SetName("Upload");
		commandBuffer->IssueBarriers(GpuBufferBarrier(upload, GpuAccessState(), transferRead));

		const bool validateRestingStates = gGpuValidateRestingStates;
		gGpuValidateRestingStates.Set(true);

		commandBuffer->CopyBufferToBuffer(source, restingBuffer, 0, 0, sizeof(u32));
		if(recordsMisdeclaredLayouts)
			commandBuffer->CopyBufferToTexture(upload, restingTexture, 0, 0, 0);

		gGpuValidateRestingStates.Set(validateRestingStates);

		// A dispatch must not access what an earlier dispatch wrote without a barrier in between
		if(pipeline != nullptr)
		{
			const TShared<render::GpuBuffer> hazardBuffer = fnCreateBuffer("InPassHazard");
			const TShared<render::Texture> hazardTexture = fnCreateTexture("InPassHazardTexture");
			const auto fnIssueBarriers = [&commandBuffer, &hazardBuffer, &hazardTexture](const GpuAccessState& source, const GpuAccessState& textureSource, const GpuAccessState& destination)
			{
				const GpuBufferBarrier bufferBarrier(hazardBuffer, source, destination);
				const GpuTextureBarrier textureBarrier(hazardTexture, textureSource, destination);
				commandBuffer->IssueBarriers(GpuBarriers(TArrayView<const GpuBufferBarrier>(&bufferBarrier, 1), TArrayView<const GpuTextureBarrier>(&textureBarrier, 1)));
			};

			// The new texture starts at rest, and the dispatches overwrite it
			const GpuAccessState discardedRest = GpuBarrierTestUtility::GetDiscardedRestingState(*hazardTexture);

			const TShared<render::GpuParameterSet> parameters = context->GetParameterSetPool().Create(pipeline->GetParameterLayout()->GetSet(0), 0);
			parameters->SetStorageBuffer("OutputBuffer", hazardBuffer);
			parameters->SetStorageTexture("OutputTexture", hazardTexture, TextureSurface());

			fnIssueBarriers(GpuAccessState(), discardedRest, computeReadWrite);
			commandBuffer->SetGpuComputePipelineState(pipeline);
			commandBuffer->SetGpuParameterSet(parameters);
			commandBuffer->DispatchCompute(1, 1, 1);
			commandBuffer->DispatchCompute(1, 1, 1);

			// A barrier orders the next dispatch after the earlier writes
			fnIssueBarriers(computeReadWrite, computeReadWrite, computeReadWrite);
			commandBuffer->DispatchCompute(1, 1, 1);
		}

		// A barrier on the next command buffer must cover the accesses on the previous one
		const TShared<render::GpuBuffer> carried = fnCreateBuffer("CarriedState");
		commandBuffer->IssueBarriers(GpuBufferBarrier(carried, GpuAccessState(), transferWrite));
		commandBuffer->CopyBufferToBuffer(source, carried, 0, 0, sizeof(u32));
		commandBuffer->End();

		const TShared<GpuCommandBuffer> nextCommandBuffer = pool->Create(GpuCommandBufferCreateInformation::Create("Barrier resource validation, continued"));
		nextCommandBuffer->SetValidator(&validator);
		nextCommandBuffer->IssueBarriers(GpuBufferBarrier(carried, host, transferRead));
		nextCommandBuffer->End();

		// The command buffers are never submitted
		if(unplaced != nullptr)
			allocator.Return(*unplaced);

		allocator.EndScope();
		pool->Reset();

		// Backends may only report their validation messages when the device next goes idle, which must happen within the logging scope
		device->WaitUntilIdle();
	}, "GpuBackendTestSuite::TestBarrierResourceValidation", true);
}
#endif

void GpuBackendTestSuite::TestTransientAliasing()
{
	GpuBackend& backend = GpuBackend::Instance();
	if(backend.GetDeviceCount() == 0)
		return;

	// Shaders are resources, which are created outside of the render thread
	const TShared<GpuDevice> device = backend.GetDevice(0);
	const TShared<GpuComputePipelineState> pipeline = CreateTestDispatchPipeline(*device);
	GetRenderThread().PostCommand([this, &device, &pipeline]()
	{
		static constexpr u32 kSize = 16;
		static constexpr u32 kDispatchPattern = 0x5A000000;
		static constexpr u32 kBufferBytes = kSize * kSize * sizeof(u32);
		const GpuAccessFlags readWrite = GpuAccessFlag::Read | GpuAccessFlag::Write;
		const GpuAccessState transferWrite(GpuStageFlag::Transfer, GpuAccessFlag::Write, GpuImageLayout::TransferDestination);
		const GpuAccessState transferRead(GpuStageFlag::Transfer, GpuAccessFlag::Read, GpuImageLayout::TransferSource);
		const GpuAccessState hostRead(GpuStageFlag::Host, GpuAccessFlag::Read);
		const GpuAccessState computeReadWrite(GpuStageFlag::ComputeShaderNonUniform, readWrite, GpuImageLayout::General);

		const GpuBufferCreateInformation bufferInformation = GpuBufferCreateInformation::CreateStructuredStorage(sizeof(u32), kSize * kSize,
			GpuBufferFlag::StoreOnGPU | GpuBufferFlag::AllowUnorderedAccessOnTheGPU);

		TextureCreateInformation textureInformation;
		textureInformation.Name = "Transient aliased texture";
		textureInformation.Width = kSize;
		textureInformation.Height = kSize;
		textureInformation.Format = PF_R32U;
		textureInformation.Usage = TextureUsageFlag::AllowUnorderedAccessOnTheGPU;

		const ImageSubresourcePitch pitch = device->CreateTexture(textureInformation)->GetStagingBufferPitchForSubresource(0, 0);
		const u32 stagingElementCount = pitch.RowPitch * pitch.SliceHeight;

		// Each resource gets its own pattern, with the resource index in the high bits and the element index in the low bits
		static constexpr u32 kResourceCount = 4;
		TShared<render::GpuBuffer> uploads[kResourceCount];
		TShared<render::GpuBuffer> readbacks[kResourceCount];
		for(u32 resourceIndex = 0; resourceIndex < kResourceCount; resourceIndex++)
		{
			uploads[resourceIndex] = device->CreateGpuBuffer(GpuBufferCreateInformation::CreateStagingWrite(stagingElementCount * sizeof(u32)));
			readbacks[resourceIndex] = device->CreateGpuBuffer(GpuBufferCreateInformation::CreateStagingRead(stagingElementCount * sizeof(u32)));

			const render::GpuBufferMappedScope mapping = uploads[resourceIndex]->Map(GpuMapOption::Write);
			B3D_TEST_ASSERT(mapping.IsValid())
			if(!mapping.IsValid())
				return;

			u32* elements = static_cast<u32*>(mapping.GetMappedMemory());
			for(u32 elementIndex = 0; elementIndex < stagingElementCount; elementIndex++)
				elements[elementIndex] = ((resourceIndex + 1) << 24) | elementIndex;
		}

		// Checks that a readback holds @p pattern in the high bits of every element. The low bits hold the element index of a copy,
		// or the thread index of a dispatch.
		auto fnCheckReadback = [this, &pitch](const TShared<render::GpuBuffer>& readback, bool isTexture, u32 pattern, bool isDispatch, const String& caseName)
		{
			const render::GpuBufferMappedScope mapping = readback->Map(GpuMapOption::Read);
			B3D_TEST_ASSERT(mapping.IsValid())
			if(!mapping.IsValid())
				return;

			const u32* elements = static_cast<const u32*>(mapping.GetMappedMemory());
			bool matches = true;
			for(u32 row = 0; row < kSize; row++)
			{
				for(u32 column = 0; column < kSize; column++)
				{
					const u32 elementIndex = isTexture ? row * pitch.RowPitch + column : row * kSize + column;
					matches &= elements[elementIndex] == (pattern | (isDispatch ? row * kSize + column : elementIndex));
				}
			}

			B3D_TEST_ASSERT_MSG(matches, caseName + ": the readback does not hold the written values.")
		};

		const auto fnIssueReadbackBarriers = [&readbacks](render::GpuCommandBuffer& commands, const GpuAccessState& source, const GpuAccessState& destination)
		{
			GpuBufferBarrier barriers[] = { GpuBufferBarrier(readbacks[0], source, destination), GpuBufferBarrier(readbacks[1], source, destination),
				GpuBufferBarrier(readbacks[2], source, destination), GpuBufferBarrier(readbacks[3], source, destination) };
			commands.IssueBarriers(GpuBarriers(TArrayView<const GpuBufferBarrier>(barriers, kResourceCount)));
		};

		const TShared<GpuWorkContext> context = GpuWorkContext::Create(*device);
		const TShared<GpuCommandBufferPool> pool = GpuBarrierTestUtility::CreateCommandBufferPool(*device);

		GpuSubmissionTimeline timeline;
		timeline.AddSubmission(GpuQueueId(GQT_GRAPHICS, 0), GpuQueueMask::kNone);

		GpuTransientResourceAllocator allocator(*device);
		const TArrayView<const GpuTransientLastUse> noUses;

		GpuAccessScope transferAccess;
		transferAccess.Add(GpuStageFlag::Transfer, readWrite);

		// Two buffers, then two textures, each placed over the memory the one before it released. Passes count the releases.
		GpuTransientScope& scope = allocator.BeginScope(timeline);
		const GpuTransientLastUse lastUses[] = { GpuTransientLastUse(0, transferAccess, 1), GpuTransientLastUse(0, transferAccess, 2),
			GpuTransientLastUse(0, transferAccess, 3), GpuTransientLastUse(0, transferAccess, 4) };

		const GpuTransientBuffer firstBuffer = scope.AllocateBuffer(bufferInformation, 0);
		if(firstBuffer.Buffer != nullptr)
			scope.Release(*firstBuffer.Buffer, TArrayView<const GpuTransientLastUse>(&lastUses[0], 1));

		const GpuTransientBuffer secondBuffer = scope.AllocateBuffer(bufferInformation, 0);
		if(secondBuffer.Buffer != nullptr)
			scope.Release(*secondBuffer.Buffer, TArrayView<const GpuTransientLastUse>(&lastUses[1], 1));

		const GpuTransientTexture firstTexture = scope.AllocateTexture(textureInformation, 0);
		if(firstTexture.Texture != nullptr)
			scope.Release(*firstTexture.Texture, TArrayView<const GpuTransientLastUse>(&lastUses[2], 1));

		const GpuTransientTexture secondTexture = scope.AllocateTexture(textureInformation, 0);
		if(secondTexture.Texture != nullptr)
			scope.Release(*secondTexture.Texture, TArrayView<const GpuTransientLastUse>(&lastUses[3], 1));

		const bool isAllocated = firstBuffer.Buffer != nullptr && secondBuffer.Buffer != nullptr && firstTexture.Texture != nullptr && secondTexture.Texture != nullptr;
		B3D_TEST_ASSERT(isAllocated)
		if(!isAllocated)
		{
			allocator.EndScope();
			return;
		}

		// The second resource of each pair orders its first use after the transfers of the first one
		const bool isBufferAliased = firstBuffer.Buffer->HasFixedLocation();
		const bool isTextureAliased = firstTexture.Texture->HasFixedLocation();
		if(isBufferAliased)
		{
			B3D_TEST_ASSERT(secondBuffer.Acquire.Source.GetStages() == GpuStageFlag::Transfer)
			B3D_TEST_ASSERT(secondBuffer.LatestPredecessorPassIndex == 1u)
		}

		if(isTextureAliased)
		{
			B3D_TEST_ASSERT(secondTexture.Acquire.Source.GetStages() == GpuStageFlag::Transfer)
			B3D_TEST_ASSERT(secondTexture.LatestPredecessorPassIndex == 3u)
		}

		const TShared<render::GpuBuffer> unrelated = device->CreateGpuBuffer(bufferInformation);
		const TShared<GpuCommandBuffer> commands = pool->Create(GpuCommandBufferCreateInformation::Create("Transient explicit aliasing"));

#if B3D_BUILD_TYPE_DEVELOPMENT
		// Development builds validate the alias acquires as a frame graph does, with one validator per scope
		GpuBarrierValidator validator;
		commands->SetValidator(&validator);
		validator.DeclareAliasAcquire(*firstBuffer.Buffer, firstBuffer.Acquire);
		validator.DeclareAliasAcquire(*secondBuffer.Buffer, secondBuffer.Acquire);
		validator.DeclareAliasAcquire(*firstTexture.Texture, firstTexture.Acquire);
		validator.DeclareAliasAcquire(*secondTexture.Texture, secondTexture.Acquire);
#endif

		fnIssueReadbackBarriers(*commands, GpuAccessState(), transferWrite);
		commands->IssueBarriers(GpuBufferBarrier(unrelated, GpuAccessState(), transferWrite));

		// Full alias acquires of the buffers
		const auto fnWriteAndReadBuffer = [&](const GpuTransientBuffer& buffer, u32 resourceIndex)
		{
			commands->IssueBarriers(CreateAliasAcquire(buffer.Buffer, buffer.Acquire, transferWrite));
			commands->CopyBufferToBuffer(uploads[resourceIndex], buffer.Buffer, 0, 0, kBufferBytes);
			commands->IssueBarriers(GpuBufferBarrier(buffer.Buffer, transferWrite, transferRead));
			commands->CopyBufferToBuffer(buffer.Buffer, readbacks[resourceIndex], 0, 0, kBufferBytes);
		};

		fnWriteAndReadBuffer(firstBuffer, 0);
		fnWriteAndReadBuffer(secondBuffer, 1);

		// A full alias acquire of the first texture, which may be placed over the buffers
		commands->IssueBarriers(CreateAliasAcquire(firstTexture.Texture, firstTexture.Acquire, transferWrite));
		commands->CopyBufferToTexture(uploads[2], firstTexture.Texture, 0, 0, 0);
		commands->IssueBarriers(GpuTextureBarrier(firstTexture.Texture, transferWrite, transferRead));
		commands->CopyTextureToBuffer(firstTexture.Texture, readbacks[2], 0, 0);

		// A split alias acquire of the second texture, with unrelated work between the halves
		const GpuTextureBarrier splitAcquire = CreateAliasAcquire(secondTexture.Texture, secondTexture.Acquire, transferWrite);
		const TShared<GpuSplitBarrier> split = device->CreateSplitBarrier();
		commands->ReleaseBarriers(splitAcquire, split);
		commands->CopyBufferToBuffer(uploads[3], unrelated, 0, 0, kBufferBytes);
		commands->AcquireBarriers(splitAcquire, split);
		commands->CopyBufferToTexture(uploads[3], secondTexture.Texture, 0, 0, 0);
		commands->IssueBarriers(GpuTextureBarrier(secondTexture.Texture, transferWrite, transferRead));
		commands->CopyTextureToBuffer(secondTexture.Texture, readbacks[3], 0, 0);

		fnIssueReadbackBarriers(*commands, transferWrite, hostRead);
		context->SubmitCommandBuffer(commands, GpuQueueMask::kNone);
		allocator.EndScope();
		device->WaitUntilIdle();

		fnCheckReadback(readbacks[0], false, 1 << 24, false, "Full alias acquire of a buffer");
		fnCheckReadback(readbacks[1], false, 2 << 24, false, "Full alias acquire of a buffer over another buffer");
		fnCheckReadback(readbacks[2], true, 3 << 24, false, "Full alias acquire of a texture");
		fnCheckReadback(readbacks[3], true, 4 << 24, false, "Split alias acquire of a texture over another texture");

		// Aliasing keeps the peak memory use of the scope below the total size of its allocations
		const GpuTransientStatistics statistics = allocator.GetStatistics();
		B3D_TEST_ASSERT(statistics.PlacementCount == 4)
		if(isBufferAliased)
			B3D_TEST_ASSERT(statistics.LastScope.PeakUsed < statistics.LastScope.RequestedBytes)

		device->EndFrame();

		// Resources bound before they are placed, and then recreated at other memory by the placement, are written by a dispatch
		if(pipeline != nullptr)
		{
			// The first resources come out of the cache again
			const TShared<render::GpuBuffer> movedBuffer = allocator.GetOrCreateBuffer(bufferInformation);
			const TShared<render::Texture> movedTexture = allocator.GetOrCreateTexture(textureInformation);
			B3D_TEST_ASSERT(movedBuffer != nullptr && movedTexture != nullptr)
			if(movedBuffer == nullptr || movedTexture == nullptr)
				return;

			B3D_TEST_ASSERT(!isBufferAliased || movedBuffer == firstBuffer.Buffer)
			B3D_TEST_ASSERT(!isTextureAliased || movedTexture == firstTexture.Texture)

			const TShared<render::GpuParameterSet> parameters = context->GetParameterSetPool().Create(pipeline->GetParameterLayout()->GetSet(0), 0);
			parameters->SetStorageBuffer("OutputBuffer", movedBuffer);
			parameters->SetStorageTexture("OutputTexture", movedTexture, TextureSurface());

			// The second resources take the memory the first ones were cached at, so the first ones move
			GpuTransientScope& movedScope = allocator.BeginScope(timeline);
			const GpuTransientBuffer bufferBlocker = movedScope.AllocateBuffer(bufferInformation, 0);
			const GpuTransientTexture textureBlocker = movedScope.AllocateTexture(textureInformation, 0);
			const GpuTransientPlacement bufferPlacement = movedScope.PlaceBuffer(*movedBuffer, 0);
			const GpuTransientPlacement texturePlacement = movedScope.PlaceTexture(*movedTexture, 0);
			B3D_TEST_ASSERT(bufferPlacement.Recreated == isBufferAliased)
			B3D_TEST_ASSERT(texturePlacement.Recreated == isTextureAliased)

			if(bufferBlocker.Buffer != nullptr)
				movedScope.Release(*bufferBlocker.Buffer, noUses);

			if(textureBlocker.Texture != nullptr)
				movedScope.Release(*textureBlocker.Texture, noUses);

			GpuAccessScope dispatchAccess;
			dispatchAccess.Add(GpuStageFlag::ComputeShaderNonUniform, readWrite);
			dispatchAccess.Add(GpuStageFlag::Transfer, GpuAccessFlag::Read);
			const GpuTransientLastUse dispatchLastUse(0, dispatchAccess);
			movedScope.Release(*movedBuffer, TArrayView<const GpuTransientLastUse>(&dispatchLastUse, 1));
			movedScope.Release(*movedTexture, TArrayView<const GpuTransientLastUse>(&dispatchLastUse, 1));

			const GpuBufferBarrier bufferAcquire = CreateAliasAcquire(movedBuffer, bufferPlacement.Acquire, computeReadWrite);
			const GpuTextureBarrier textureAcquire = CreateAliasAcquire(movedTexture, texturePlacement.Acquire, computeReadWrite);

			const TShared<GpuCommandBuffer> dispatchCommands = pool->Create(GpuCommandBufferCreateInformation::Create("Transient dispatch after placement"));

#if B3D_BUILD_TYPE_DEVELOPMENT
			GpuBarrierValidator dispatchValidator;
			dispatchCommands->SetValidator(&dispatchValidator);
			dispatchValidator.DeclareAliasAcquire(*movedBuffer, bufferPlacement.Acquire);
			dispatchValidator.DeclareAliasAcquire(*movedTexture, texturePlacement.Acquire);
#endif

			fnIssueReadbackBarriers(*dispatchCommands, GpuAccessState(), transferWrite);
			dispatchCommands->IssueBarriers(GpuBarriers(TArrayView<const GpuBufferBarrier>(&bufferAcquire, 1), TArrayView<const GpuTextureBarrier>(&textureAcquire, 1)));
			dispatchCommands->SetGpuComputePipelineState(pipeline);
			dispatchCommands->SetGpuParameterSet(parameters);
			dispatchCommands->DispatchCompute(1, 1, 1);

			const GpuBufferBarrier bufferRead(movedBuffer, computeReadWrite, transferRead);
			const GpuTextureBarrier textureRead(movedTexture, computeReadWrite, transferRead);
			dispatchCommands->IssueBarriers(GpuBarriers(TArrayView<const GpuBufferBarrier>(&bufferRead, 1), TArrayView<const GpuTextureBarrier>(&textureRead, 1)));
			dispatchCommands->CopyBufferToBuffer(movedBuffer, readbacks[0], 0, 0, kBufferBytes);
			dispatchCommands->CopyTextureToBuffer(movedTexture, readbacks[2], 0, 0);
			fnIssueReadbackBarriers(*dispatchCommands, transferWrite, hostRead);
			context->SubmitCommandBuffer(dispatchCommands, GpuQueueMask::kNone);
			allocator.EndScope();
			device->WaitUntilIdle();

			fnCheckReadback(readbacks[0], false, kDispatchPattern, true, "Dispatch to a buffer recreated after it was bound");
			fnCheckReadback(readbacks[2], true, kDispatchPattern, true, "Dispatch to a texture recreated after it was bound");

			if(isBufferAliased && isTextureAliased)
				B3D_TEST_ASSERT(allocator.GetStatistics().Relocations == 2)

			device->EndFrame();
		}
	}, "GpuBackendTestSuite::TestTransientAliasing", true);
}
