//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DVulkanBarrierTestSuite.h"
#include "B3DVulkanGpuBackend.h"
#include "B3DVulkanGpuDevice.h"
#include "B3DVulkanTexture.h"
#include "B3DVulkanGpuBuffer.h"
#include "B3DIVulkanRenderWindowSurface.h"
#include "B3DApplication.h"
#include "Image/B3DPixelData.h"
#include "CoreObject/B3DRenderThread.h"
#include "GpuBackend/B3DGpuBuffer.h"
#include "GpuBackend/B3DRenderTexture.h"
#include "GpuBackend/B3DGpuCommandBuffer.h"
#include "GpuBackend/B3DGpuWorkContext.h"
#include "GpuBackend/B3DGpuProgram.h"
#include "GpuBackend/B3DGpuPipelineState.h"
#include "GpuBackend/B3DGpuPipelineParameterLayout.h"
#include "GpuBackend/B3DGpuParameterSetPool.h"
#include "GpuBackend/B3DGpuParameterSet.h"
#include "GpuBackend/B3DGpuSubmitThread.h"
#include "Image/B3DTexture.h"

using namespace b3d;
using namespace b3d::render;

namespace
{
	VulkanGpuDevice* GetActiveVulkanDevice()
	{
		VulkanGpuBackend& backend = GetVulkanGpuBackend();
		if(backend.GetDeviceCount() == 0)
			return nullptr;

		return backend.GetVulkanDevice(0).get();
	}

	void RunBufferHandoff(TestSuite& testSuite, GpuQueueType sourceQueueType, GpuQueueType destinationQueueType,
		bool waitForSourceCompletion = false)
	{
		VulkanGpuDevice* const device = GetActiveVulkanDevice();
		if(device == nullptr || device->GetQueueCount(sourceQueueType) == 0 || device->GetQueueCount(destinationQueueType) == 0)
			return;

		constexpr u32 kBufferSize = 256;
		std::array<u8, kBufferSize> expected;
		for(u32 byteIndex = 0; byteIndex < kBufferSize; ++byteIndex)
			expected[byteIndex] = (u8)(byteIndex ^ 0xA5);

		const TShared<render::GpuBuffer> uploadBuffer = device->CreateGpuBuffer(GpuBufferCreateInformation::CreateStagingWrite(kBufferSize));
		const TShared<render::GpuBuffer> gpuBuffer = device->CreateGpuBuffer(GpuBufferCreateInformation::CreateVertex(1, kBufferSize));
		const TShared<render::GpuBuffer> readbackBuffer = device->CreateGpuBuffer(GpuBufferCreateInformation::CreateStagingRead(kBufferSize));
		B3D_TEST_ASSERT_EXTERNAL(testSuite, uploadBuffer != nullptr)
		B3D_TEST_ASSERT_EXTERNAL(testSuite, gpuBuffer != nullptr)
		B3D_TEST_ASSERT_EXTERNAL(testSuite, readbackBuffer != nullptr)
		if(uploadBuffer == nullptr || gpuBuffer == nullptr || readbackBuffer == nullptr)
			return;

		render::GpuBufferMappedScope uploadMapping = uploadBuffer->Map(GpuMapOption::Write);
		B3D_TEST_ASSERT_EXTERNAL(testSuite, uploadMapping.IsValid())
		if(!uploadMapping.IsValid())
			return;
		memcpy(uploadMapping.GetMappedMemory(), expected.data(), expected.size());
		uploadMapping.Unmap();

		const GpuCommandBufferPoolCreateInformation sourcePoolInformation = GpuCommandBufferPoolCreateInformation::CreateForThisThread(sourceQueueType);
		const GpuCommandBufferPoolCreateInformation destinationPoolInformation = GpuCommandBufferPoolCreateInformation::CreateForThisThread(destinationQueueType);
		const TShared<render::GpuCommandBufferPool> sourcePool = device->CreateGpuCommandBufferPool(sourcePoolInformation);
		const TShared<render::GpuCommandBufferPool> destinationPool = device->CreateGpuCommandBufferPool(destinationPoolInformation);

		const TShared<render::GpuCommandBuffer> sourceCommandBuffer = sourcePool->Create(GpuCommandBufferCreateInformation::Create("Vulkan barrier test source"));
		sourceCommandBuffer->CopyBufferToBuffer(uploadBuffer, gpuBuffer, 0, 0, kBufferSize);

		const TShared<render::GpuCommandBuffer> destinationCommandBuffer = destinationPool->Create(GpuCommandBufferCreateInformation::Create("Vulkan barrier test destination"));
		destinationCommandBuffer->CopyBufferToBuffer(gpuBuffer, readbackBuffer, 0, 0, kBufferSize);

		const TShared<GpuWorkContext> context = GpuWorkContext::Create(*device);
		context->SubmitCommandBuffer(sourceCommandBuffer, GpuQueueMask::kNone);
		if(waitForSourceCompletion)
			device->WaitUntilIdle();

		context->SubmitCommandBuffer(destinationCommandBuffer, GpuQueueMask::kNone);
		device->WaitUntilIdle();

		render::GpuBufferMappedScope readbackMapping = readbackBuffer->Map(GpuMapOption::Read);
		B3D_TEST_ASSERT_EXTERNAL(testSuite, readbackMapping.IsValid())
		if(readbackMapping.IsValid())
			B3D_TEST_ASSERT_EXTERNAL(testSuite, memcmp(readbackMapping.GetMappedMemory(), expected.data(), expected.size()) == 0)
	}

	void CreateResolveTextures(VulkanGpuDevice& device, TShared<render::Texture>& outSource,
		TShared<render::Texture>& outDestination)
	{
		TextureCreateInformation sourceCreateInformation;
		sourceCreateInformation.Name = "Vulkan resolve test source";
		sourceCreateInformation.Format = PF_RGBA8;
		sourceCreateInformation.Width = 16;
		sourceCreateInformation.Height = 16;
		sourceCreateInformation.SampleCount = 4;
		sourceCreateInformation.Usage = TextureUsageFlag::RenderTarget;
		outSource = device.CreateTexture(sourceCreateInformation);

		TextureCreateInformation destinationCreateInformation(sourceCreateInformation);
		destinationCreateInformation.Name = "Vulkan resolve test destination";
		destinationCreateInformation.SampleCount = 1;
		outDestination = device.CreateTexture(destinationCreateInformation);
	}
}

VulkanBarrierTestSuite::VulkanBarrierTestSuite()
	: TestSuite("VulkanBarrierTestSuite")
{
	B3D_ADD_TEST(VulkanBarrierTestSuite::TestGraphicsToComputeBufferHandoff)
	B3D_ADD_TEST(VulkanBarrierTestSuite::TestComputeToGraphicsBufferHandoff)
	B3D_ADD_TEST(VulkanBarrierTestSuite::TestCompletedGraphicsToComputeBufferHandoff)
	B3D_ADD_TEST(VulkanBarrierTestSuite::TestCompletedQueueProgressFanOut)
	B3D_ADD_TEST(VulkanBarrierTestSuite::TestSameQueueBufferBoundary)
	B3D_ADD_TEST(VulkanBarrierTestSuite::TestQueueSharing)
	B3D_ADD_TEST(VulkanBarrierTestSuite::TestRenderPassAttachmentTransitions)
	B3D_ADD_TEST(VulkanBarrierTestSuite::TestSwapChainTransitions)
	B3D_ADD_TEST(VulkanBarrierTestSuite::TestMultisampleResolve)
	B3D_ADD_TEST(VulkanBarrierTestSuite::TestRepeatedStorageImageDispatch)
	B3D_ADD_TEST(VulkanBarrierTestSuite::TestFrameFence)
}

void VulkanBarrierTestSuite::TestGraphicsToComputeBufferHandoff()
{
	RunBufferHandoff(*this, GQT_GRAPHICS, GQT_COMPUTE);
}

void VulkanBarrierTestSuite::TestComputeToGraphicsBufferHandoff()
{
	RunBufferHandoff(*this, GQT_COMPUTE, GQT_GRAPHICS);
}

void VulkanBarrierTestSuite::TestCompletedGraphicsToComputeBufferHandoff()
{
	RunBufferHandoff(*this, GQT_GRAPHICS, GQT_COMPUTE, true);
}

void VulkanBarrierTestSuite::TestCompletedQueueProgressFanOut()
{
	VulkanGpuDevice* const device = GetActiveVulkanDevice();
	if(device == nullptr || device->GetQueueCount(GQT_GRAPHICS) == 0 || device->GetQueueCount(GQT_COMPUTE) == 0 ||
		device->GetQueueCount(GQT_TRANSFER) == 0)
		return;

	const TShared<render::GpuCommandBufferPool> graphicsPool = device->CreateGpuCommandBufferPool(
		GpuCommandBufferPoolCreateInformation::CreateForThisThread(GQT_GRAPHICS));
	const TShared<render::GpuCommandBufferPool> computePool = device->CreateGpuCommandBufferPool(
		GpuCommandBufferPoolCreateInformation::CreateForThisThread(GQT_COMPUTE));
	const TShared<render::GpuCommandBufferPool> transferPool = device->CreateGpuCommandBufferPool(
		GpuCommandBufferPoolCreateInformation::CreateForThisThread(GQT_TRANSFER));

	const TShared<render::GpuCommandBuffer> sourceCommandBuffer =
		graphicsPool->Create(GpuCommandBufferCreateInformation::Create("Vulkan queue progress fan-out source"));
	const TShared<render::GpuCommandBuffer> computeCommandBuffer =
		computePool->Create(GpuCommandBufferCreateInformation::Create("Vulkan queue progress fan-out compute"));
	const TShared<render::GpuCommandBuffer> transferCommandBuffer =
		transferPool->Create(GpuCommandBufferCreateInformation::Create("Vulkan queue progress fan-out transfer"));

	const TShared<GpuWorkContext> context = GpuWorkContext::Create(*device);
	context->SubmitCommandBuffer(sourceCommandBuffer, GpuQueueMask::kNone);
	device->WaitUntilIdle();

	const GpuQueueMask graphicsProgress(GpuQueueId(GQT_GRAPHICS, 0));
	context->SubmitCommandBuffer(computeCommandBuffer, graphicsProgress);
	context->SubmitCommandBuffer(transferCommandBuffer, graphicsProgress);
	device->WaitUntilIdle();
}

void VulkanBarrierTestSuite::TestSameQueueBufferBoundary()
{
	RunBufferHandoff(*this, GQT_GRAPHICS, GQT_GRAPHICS);
}

void VulkanBarrierTestSuite::TestQueueSharing()
{
	VulkanGpuDevice* const device = GetActiveVulkanDevice();
	if(device == nullptr)
		return;

	struct TextureCase
	{
		const char* Name;
		PixelFormat Format;
		TextureUsageFlags Usage;
		bool IsShared;
	};

	const TextureCase textureCases[] =
	{
		{ "Vulkan sampleable texture", PF_RGBA8, TextureUsageFlag::StoreOnGPU, true },
		{ "Vulkan concurrent-read render target", PF_RGBA8, TextureUsageFlag::RenderTarget | TextureUsageFlag::AllowConcurrentQueueReads, true },
		{ "Vulkan render target", PF_RGBA8, TextureUsageFlag::RenderTarget, false },
		{ "Vulkan depth-stencil target", PF_D32, TextureUsageFlag::DepthStencil, false },
		{ "Vulkan UAV texture", PF_RGBA8, TextureUsageFlag::AllowUnorderedAccessOnTheGPU, false },
		{ "Vulkan CPU-writable texture", PF_RGBA8, TextureUsageFlag::StoreOnCPUWithGPUAccess, false },
	};

	static constexpr u32 kTextureCaseCount = sizeof(textureCases) / sizeof(textureCases[0]);
	bool texturesCreated[kTextureCaseCount] = {};
	bool texturesExclusive[kTextureCaseCount] = {};
	bool bufferCreated = false;
	bool bufferExclusive = false;
	GetRenderThread().PostCommand([device, &textureCases, &texturesCreated, &texturesExclusive, &bufferCreated, &bufferExclusive]()
	{
		for(u32 caseIndex = 0; caseIndex < kTextureCaseCount; ++caseIndex)
		{
			TextureCreateInformation createInformation;
			createInformation.Name = textureCases[caseIndex].Name;
			createInformation.Format = textureCases[caseIndex].Format;
			createInformation.Width = 8;
			createInformation.Height = 8;
			createInformation.Usage = textureCases[caseIndex].Usage;

			const TShared<render::Texture> texture = device->CreateTexture(createInformation);
			texturesCreated[caseIndex] = texture != nullptr;
			if(texture != nullptr)
				texturesExclusive[caseIndex] = std::static_pointer_cast<VulkanTexture>(texture)->GetVulkanResource()->IsExclusive();
		}

		const TShared<render::GpuBuffer> buffer = device->CreateGpuBuffer(GpuBufferCreateInformation::CreateVertex(1, 256));
		bufferCreated = buffer != nullptr;
		if(buffer != nullptr)
			bufferExclusive = std::static_pointer_cast<VulkanGpuBuffer>(buffer)->GetVulkanResource()->IsExclusive();
	}, "VulkanBarrierTestSuite::TestQueueSharing", true);

	// With a single queue family there is nothing to share between, so every resource is exclusive
	const bool hasMultipleFamilies = device->GetQueueFamilies().Size() > 1;
	for(u32 caseIndex = 0; caseIndex < kTextureCaseCount; ++caseIndex)
	{
		B3D_TEST_ASSERT(texturesCreated[caseIndex])
		B3D_TEST_ASSERT(texturesExclusive[caseIndex] == !(textureCases[caseIndex].IsShared && hasMultipleFamilies))
	}

	B3D_TEST_ASSERT(bufferCreated)
	B3D_TEST_ASSERT(bufferExclusive == !hasMultipleFamilies)
}

void VulkanBarrierTestSuite::TestMultisampleResolve()
{
	VulkanGpuDevice* const device = GetActiveVulkanDevice();
	if(device == nullptr || device->GetQueueCount(GQT_GRAPHICS) == 0)
		return;

	bool texturesCreated = false;
	bool resolveRecorded = false;
	GetRenderThread().PostCommand([device, &texturesCreated, &resolveRecorded]()
	{
		TShared<render::Texture> source;
		TShared<render::Texture> destination;
		CreateResolveTextures(*device, source, destination);
		texturesCreated = source != nullptr && destination != nullptr;
		if(!texturesCreated)
			return;

		const TShared<render::GpuCommandBufferPool> commandBufferPool = device->CreateGpuCommandBufferPool(
			GpuCommandBufferPoolCreateInformation::CreateForThisThread(GQT_GRAPHICS));
		const TShared<render::GpuCommandBuffer> commandBuffer = commandBufferPool->Create(
			GpuCommandBufferCreateInformation::Create("Vulkan MSAA resolve test"));
		resolveRecorded = commandBuffer->CopyTexture(source, destination);
		if(!resolveRecorded)
			return;

		const TShared<GpuWorkContext> workContext = GpuWorkContext::Create(*device);
		workContext->SubmitCommandBuffer(commandBuffer, GpuQueueMask::kNone);
		device->WaitUntilIdle();
	}, "VulkanBarrierTestSuite::TestMultisampleResolve", true);

	B3D_TEST_ASSERT(texturesCreated)
	B3D_TEST_ASSERT(resolveRecorded)
}

void VulkanBarrierTestSuite::TestRepeatedStorageImageDispatch()
{
	GetRenderThread().PostCommand([this]()
	{
		VulkanGpuDevice* const device = GetActiveVulkanDevice();
		if(device == nullptr)
			return;

		GpuProgramCreateInformation programInformation;
		programInformation.Name = "Repeated storage-image dispatch";
		programInformation.Type = GPT_COMPUTE_PROGRAM;
		programInformation.Language = VulkanGpuDevice::kGpuProgramLanguageName;
		programInformation.EntryPoint = "main";
		programInformation.Source = R"(#version 450
layout(local_size_x = 8, local_size_y = 8) in;
layout(set = 0, binding = 0, r32ui) uniform uimage2D OutputImage;
layout(push_constant) uniform Operation { uint Increment; } operation;
void main()
{
	ivec2 pixel = ivec2(gl_GlobalInvocationID.xy);
	uint value = operation.Increment == 0 ? 1 : imageLoad(OutputImage, pixel).r + 1;
	imageStore(OutputImage, pixel, uvec4(value));
}
)";
		const TShared<GpuProgram> program = device->CreateGpuProgram(programInformation);
		B3D_TEST_ASSERT(program != nullptr && program->IsCompiled())
		if(program == nullptr || !program->IsCompiled())
			return;

		GpuComputePipelineStateCreateInformation pipelineInformation;
		pipelineInformation.Program = program;
		const TShared<GpuComputePipelineState> pipeline = device->CreateGpuComputePipelineState(pipelineInformation);
		TextureCreateInformation textureInformation;
		textureInformation.Width = 64;
		textureInformation.Height = 64;
		textureInformation.Format = PF_R32U;
		textureInformation.Usage = TextureUsageFlag::AllowUnorderedAccessOnTheGPU;
		const TShared<render::Texture> texture = device->CreateTexture(textureInformation);
		const TUnique<GpuParameterSetPool> parameterPool = device->CreateParameterSetPool(GpuParameterSetPoolCreateInformation());
		const TShared<render::GpuParameterSet> parameters = parameterPool->Create(pipeline->GetParameterLayout()->GetSet(0), 0);
		B3D_TEST_ASSERT(parameters->SetStorageTexture(0, texture, TextureSurface(0, 1, 0, 1)))

		const TShared<render::GpuBuffer> readback = device->CreateGpuBuffer(GpuBufferCreateInformation::CreateStagingRead(64 * 64 * sizeof(u32)));
		const TShared<render::GpuCommandBufferPool> commandPool = device->CreateGpuCommandBufferPool(GpuCommandBufferPoolCreateInformation::CreateForThisThread(GQT_COMPUTE));
		const TShared<render::GpuCommandBuffer> commands = commandPool->Create(GpuCommandBufferCreateInformation::Create("Repeated storage-image dispatch"));
		commands->SetGpuComputePipelineState(pipeline);
		commands->SetGpuParameterSet(parameters);
		u32 increment = 0;
		commands->SetPushConstants(0, sizeof(increment), &increment);
		commands->DispatchCompute(8, 8, 1);
		increment = 1;
		commands->SetPushConstants(0, sizeof(increment), &increment);
		commands->DispatchCompute(8, 8, 1);
		commands->CopyTextureToBuffer(texture, readback, 0, 0);
		const TShared<GpuWorkContext> context = GpuWorkContext::Create(*device);
		context->SubmitCommandBuffer(commands, GpuQueueMask::kNone);
		device->WaitUntilIdle();

		const render::GpuBufferMappedScope mapping = readback->Map(GpuMapOption::Read);
		B3D_TEST_ASSERT(mapping.IsValid())
		if(mapping.IsValid())
		{
			const u32* pixels = static_cast<const u32*>(mapping.GetMappedMemory());
			for(u32 pixelIndex = 0; pixelIndex < 64 * 64; pixelIndex++)
				B3D_TEST_ASSERT(pixels[pixelIndex] == 2)
		}
	}, "VulkanBarrierTestSuite::TestRepeatedStorageImageDispatch", true);
}

void VulkanBarrierTestSuite::TestRenderPassAttachmentTransitions()
{
	GetRenderThread().PostCommand([this]()
	{
		VulkanGpuDevice* const device = GetActiveVulkanDevice();
		if(device == nullptr)
			return;

		const TShared<render::GpuCommandBufferPool> commandPool = device->CreateGpuCommandBufferPool(GpuCommandBufferPoolCreateInformation::CreateForThisThread(GQT_GRAPHICS));
		const TShared<GpuWorkContext> context = GpuWorkContext::Create(*device);
		for(PixelFormat format : { PF_RGBA8, PF_D32, PF_D32_S8X24 })
		{
			const bool depth = format != PF_RGBA8;
			TextureCreateInformation textureInformation;
			textureInformation.Name = "Render-pass attachment transitions";
			textureInformation.Width = 16;
			textureInformation.Height = 16;
			textureInformation.Format = format;
			textureInformation.Usage = depth ? TextureUsageFlag::DepthStencil : TextureUsageFlag::RenderTarget;
			textureInformation.ClearColor = Color(0, 1, 0, 1);
			textureInformation.ClearDepth = 0.25f;
			textureInformation.ClearStencil = 37;
			const TShared<render::Texture> texture = device->CreateTexture(textureInformation);
			render::RenderTextureCreateInformation targetInformation;
			if(depth)
				targetInformation.DepthStencilSurface.Texture = texture;
			else
				targetInformation.ColorSurfaces[0].Texture = texture;

			const TShared<render::RenderTexture> target = render::RenderTexture::Create(targetInformation);
			const RenderSurfaceMask surfaces = depth ? RT_DEPTH | RT_STENCIL : RT_COLOR0;
			const ImageSubresourcePitch pitch = texture->GetStagingBufferPitchForSubresource(0, 0);
			const TShared<render::GpuBuffer> readback = device->CreateGpuBuffer(GpuBufferCreateInformation::CreateStagingRead(pitch.RowPitch * pitch.SliceHeight * sizeof(u32)));
			for(RenderSurfaceMask readOnlyMask : { RenderSurfaceMask(RT_NONE), surfaces, RenderSurfaceMask(RT_DEPTH), RenderSurfaceMask(RT_STENCIL) })
			{
				if(!depth && (readOnlyMask == RT_DEPTH || readOnlyMask == RT_STENCIL))
					continue;

				const TShared<render::GpuCommandBuffer> commands = commandPool->Create(GpuCommandBufferCreateInformation::Create("Render-pass clear, load and readback"));
				RenderPassCreateInformation pass(target);

				// Discard followed by clear also exercises reuse of an attachment without preserving its contents.
				commands->BeginRenderPass(pass);
				commands->EndRenderPass();
				pass.ClearMask = surfaces;
				commands->BeginRenderPass(pass);
				commands->EndRenderPass();
				// Return to the attachment layout within one batch; no intermediate transition may survive.
				GpuBarriers mergedBarriers;
				mergedBarriers.TextureBarriers.Add(GpuTextureBarrier(texture, GpuResourceUseFlag::Transfer, GpuAccessFlag::Read, GpuImageLayout::TransferSource));
				mergedBarriers.TextureBarriers.Add(GpuTextureBarrier(texture, depth ? GpuResourceUseFlag::DepthStencilAttachment : GpuResourceUseFlag::ColorAttachment, GpuAccessFlag::Read | GpuAccessFlag::Write, depth ? GpuImageLayout::DepthStencilAttachment : GpuImageLayout::ColorAttachment));
				commands->IssueBarriers(mergedBarriers);

				pass.ClearMask = RT_NONE;
				pass.LoadMask = surfaces;
				pass.ReadOnlyMask = readOnlyMask;
				commands->BeginRenderPass(pass);
				commands->EndRenderPass();

				// Consecutive read-only passes must not hide a write in a final layout transition.
				commands->BeginRenderPass(pass);
				commands->EndRenderPass();
				commands->CopyTextureToBuffer(texture, readback, 0, 0);
				context->SubmitCommandBuffer(commands, GpuQueueMask::kNone);
				device->WaitUntilIdle();

				const render::GpuBufferMappedScope mapping = readback->Map(GpuMapOption::Read);
				B3D_TEST_ASSERT(mapping.IsValid())
				if(!mapping.IsValid())
					return;

				for(u32 row = 0; row < textureInformation.Height; row++)
				{
					for(u32 column = 0; column < textureInformation.Width; column++)
					{
						const u32 pixelIndex = row * pitch.RowPitch + column;
						if(depth)
							B3D_TEST_ASSERT(static_cast<const float*>(mapping.GetMappedMemory())[pixelIndex] == textureInformation.ClearDepth)
						else
						{
							const u8* pixel = static_cast<const u8*>(mapping.GetMappedMemory()) + pixelIndex * 4;
							B3D_TEST_ASSERT(pixel[0] == 0 && pixel[1] == 255 && pixel[2] == 0 && pixel[3] == 255)
						}
					}
				}
			}
		}
	}, "VulkanBarrierTestSuite::TestRenderPassAttachmentTransitions", true);
}

void VulkanBarrierTestSuite::TestSwapChainTransitions()
{
	const TShared<render::RenderWindow> window = B3DGetRenderProxy(GetApplication().GetPrimaryWindow());
	GetRenderThread().PostCommand([this, window]()
	{
		VulkanGpuDevice* const device = GetActiveVulkanDevice();
		if(device == nullptr || window == nullptr)
			return;

		IVulkanRenderWindowSurface& surface = static_cast<IVulkanRenderWindowSurface&>(*window->GetRenderWindowSurface());
		const TShared<GpuQueue> queue = device->GetQueue(GQT_GRAPHICS, 0);
		const TShared<GpuWorkContext> context = GpuWorkContext::Create(*device);
		const TShared<render::GpuCommandBufferPool> graphicsPool = device->CreateGpuCommandBufferPool(GpuCommandBufferPoolCreateInformation::CreateForThisThread(GQT_GRAPHICS));
		const TShared<render::GpuCommandBufferPool> computePool = device->GetQueueCount(GQT_COMPUTE) > 0 ? device->CreateGpuCommandBufferPool(GpuCommandBufferPoolCreateInformation::CreateForThisThread(GQT_COMPUTE)) : graphicsPool;
		for(u32 generation = 0; generation < 2; generation++)
		{
			// A newly acquired image must wait for acquisition even when nothing is drawn before presenting it.
			B3D_TEST_ASSERT(surface.GetActiveFramebuffer() != nullptr)
			surface.SwapBuffers(*queue, GpuQueueMask::kNone);
			for(u32 readbackMode = 0; readbackMode < 3; readbackMode++)
			{
				const TShared<render::GpuCommandBuffer> commands = graphicsPool->Create(GpuCommandBufferCreateInformation::Create("Swapchain clear and load"));
				RenderPassCreateInformation pass(window);
				pass.ClearMask = RT_COLOR0;
				commands->BeginRenderPass(pass);
				commands->EndRenderPass();
				pass.ClearMask = RT_NONE;
				pass.LoadMask = RT_COLOR0;
				commands->BeginRenderPass(pass);
				commands->EndRenderPass();
				context->SubmitCommandBuffer(commands, GpuQueueMask::kNone);

				TAsyncOp<TShared<PixelData>> readback;
				if(readbackMode != 0)
				{
					const TShared<render::GpuCommandBufferPool>& readbackPool = readbackMode == 1 ? graphicsPool : computePool;
					const TShared<render::GpuCommandBuffer> readCommands = readbackPool->Create(GpuCommandBufferCreateInformation::Create("Swapchain readback before present"));
					// Read the surface directly so presentation must transition from TransferSource, including a queue handoff.
					readback = surface.ReadAsync(*readCommands);
					context->SubmitCommandBuffer(readCommands, GpuQueueMask::kNone);
				}

				surface.SwapBuffers(*queue, GpuQueueMask::kNone);
				device->WaitUntilIdle();
				if(readbackMode != 0)
				{
					const TShared<PixelData> pixels = readback.GetReturnValue();
					B3D_TEST_ASSERT(pixels != nullptr)
					if(pixels != nullptr)
						B3D_TEST_ASSERT(pixels->GetColorAt(0, 0) == window->GetClearValues().Colors[0])
				}
			}

			// Retirement must also wait for presentation's explicit transition before destroying the old images.
			surface.MarkSwapChainAsInvalid();
			window->RebuildSwapChain();
		}
	}, "VulkanBarrierTestSuite::TestSwapChainTransitions", true);
}

void VulkanBarrierTestSuite::TestFrameFence()
{
	GetRenderThread().PostCommand([this]()
	{
		VulkanGpuDevice* const device = GetActiveVulkanDevice();
		if(device == nullptr || device->GetQueueCount(GQT_GRAPHICS) == 0 || device->GetQueueCount(GQT_COMPUTE) == 0)
			return;

		const GpuQueue& graphicsQueue = *device->GetQueue(GQT_GRAPHICS, 0);
		const GpuQueue& computeQueue = *device->GetQueue(GQT_COMPUTE, 0);
		const GpuSubmitThread& submitThread = device->GetSubmitThread();

		const TShared<GpuWorkContext> context = GpuWorkContext::Create(*device);
		const auto fnSubmit = [device, &context](GpuQueueType queueType)
		{
			const TShared<render::GpuCommandBufferPool> pool = device->CreateGpuCommandBufferPool(GpuCommandBufferPoolCreateInformation::CreateForThisThread(queueType));
			context->SubmitCommandBuffer(pool->Create(GpuCommandBufferCreateInformation::Create("Vulkan frame boundary test")), GpuQueueMask::kNone);
			device->WaitUntilIdle();
		};

		// Give both queues work in the ending frame, so the frame fence has non-zero values to wait on
		fnSubmit(GQT_GRAPHICS);
		fnSubmit(GQT_COMPUTE);

		device->EndFrame();
		device->WaitUntilIdle();
		B3D_TEST_ASSERT(submitThread.IsFrameFencePending(graphicsQueue))
		B3D_TEST_ASSERT(submitThread.IsFrameFencePending(computeQueue))

		// The first submission on a queue consumes the frame fence, and later submissions don't re-arm it
		fnSubmit(GQT_GRAPHICS);
		B3D_TEST_ASSERT(!submitThread.IsFrameFencePending(graphicsQueue))
		B3D_TEST_ASSERT(submitThread.IsFrameFencePending(computeQueue))

		fnSubmit(GQT_GRAPHICS);
		B3D_TEST_ASSERT(!submitThread.IsFrameFencePending(graphicsQueue))

		fnSubmit(GQT_COMPUTE);
		B3D_TEST_ASSERT(!submitThread.IsFrameFencePending(computeQueue))
	}, "VulkanBarrierTestSuite::TestFrameFence", true);
}
