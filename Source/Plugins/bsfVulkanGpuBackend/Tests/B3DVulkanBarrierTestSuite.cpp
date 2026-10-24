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
#include "GpuBackend/B3DGpuBackendUtility.h"
#include "Utility/B3DGpuBarrierTestUtility.h"
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
	B3D_ADD_TEST(VulkanBarrierTestSuite::TestQueueSharing)
	B3D_ADD_TEST(VulkanBarrierTestSuite::TestRenderPassAttachmentTransitions)
	B3D_ADD_TEST(VulkanBarrierTestSuite::TestSwapChainTransitions)
	B3D_ADD_TEST(VulkanBarrierTestSuite::TestMultisampleResolve)
	B3D_ADD_TEST(VulkanBarrierTestSuite::TestRepeatedStorageImageDispatch)
	B3D_ADD_TEST(VulkanBarrierTestSuite::TestFrameFence)
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
			{
				const VulkanImage* const image = std::static_pointer_cast<VulkanTexture>(texture)->GetVulkanResource();
				texturesExclusive[caseIndex] = image->IsExclusive();
			}
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

		const TShared<render::GpuCommandBufferPool> commandBufferPool = GpuBarrierTestUtility::CreateCommandBufferPool(*device, GQT_GRAPHICS);
		const TShared<render::GpuCommandBuffer> commandBuffer = commandBufferPool->Create(
			GpuCommandBufferCreateInformation::Create("Vulkan MSAA resolve test"));

		// The new textures start at rest. The resolve overwrites the destination, and reads the undefined contents of the source.
		const GpuAccessState sourceRest = GpuBarrierTestUtility::GetDiscardedRestingState(*source);
		const GpuAccessState destinationRest = GpuBarrierTestUtility::GetDiscardedRestingState(*destination);

		const GpuExplicitTextureBarrier resolveBarriers[] =
		{
			GpuExplicitTextureBarrier(source, sourceRest, GpuAccessState(GpuStageFlag::Resolve, GpuAccessFlag::Read, GpuImageLayout::ResolveSource)),
			GpuExplicitTextureBarrier(destination, destinationRest, GpuAccessState(GpuStageFlag::Resolve, GpuAccessFlag::Write, GpuImageLayout::ResolveDestination))
		};
		commandBuffer->IssueExplicitBarriers(GpuExplicitBarriers(TArrayView<const GpuExplicitBufferBarrier>(), TArrayView<const GpuExplicitTextureBarrier>(resolveBarriers, 2)));
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
		// Devices without a separate compute queue family dispatch on the graphics queue
		const GpuQueueType queueType = device->GetQueueCount(GQT_COMPUTE) > 0 ? GQT_COMPUTE : GQT_GRAPHICS;
		const TShared<render::GpuCommandBufferPool> commandPool = GpuBarrierTestUtility::CreateCommandBufferPool(*device, queueType);
		const TShared<render::GpuCommandBuffer> commands = commandPool->Create(GpuCommandBufferCreateInformation::Create("Repeated storage-image dispatch"));

		// The new texture starts at rest, and the first dispatch overwrites it. The second dispatch reads what the first one wrote.
		const GpuAccessState discardedRest = GpuBarrierTestUtility::GetDiscardedRestingState(*texture, queueType);
		const GpuAccessState computeReadWrite(GpuStageFlag::ComputeShaderNonUniform, GpuAccessFlag::Read | GpuAccessFlag::Write, GpuImageLayout::General);
		const GpuAccessState transferRead(GpuStageFlag::Transfer, GpuAccessFlag::Read, GpuImageLayout::TransferSource);
		const GpuAccessState transferWrite(GpuStageFlag::Transfer, GpuAccessFlag::Write);

		commands->IssueExplicitBarriers(GpuExplicitTextureBarrier(texture, discardedRest, computeReadWrite));
		commands->SetGpuComputePipelineState(pipeline);
		commands->SetGpuParameterSet(parameters);
		u32 increment = 0;
		commands->SetPushConstants(0, sizeof(increment), &increment);
		commands->DispatchCompute(8, 8, 1);
		commands->IssueExplicitBarriers(GpuExplicitTextureBarrier(texture, computeReadWrite, computeReadWrite));
		increment = 1;
		commands->SetPushConstants(0, sizeof(increment), &increment);
		commands->DispatchCompute(8, 8, 1);

		const GpuExplicitBufferBarrier readbackBarrier(readback, GpuAccessState(), transferWrite);
		const GpuExplicitTextureBarrier copyBarrier(texture, computeReadWrite, transferRead);
		commands->IssueExplicitBarriers(GpuExplicitBarriers(TArrayView<const GpuExplicitBufferBarrier>(&readbackBarrier, 1), TArrayView<const GpuExplicitTextureBarrier>(&copyBarrier, 1)));
		commands->CopyTextureToBuffer(texture, readback, 0, 0);
		commands->IssueExplicitBarriers(GpuBarrierTestUtility::CreateReadbackBarrier(readback));
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

		const TShared<render::GpuCommandBufferPool> commandPool = GpuBarrierTestUtility::CreateCommandBufferPool(*device, GQT_GRAPHICS);
		const TShared<GpuWorkContext> context = GpuWorkContext::Create(*device);
		const GpuAccessState transferRead(GpuStageFlag::Transfer, GpuAccessFlag::Read, GpuImageLayout::TransferSource);
		const GpuAccessState transferWrite(GpuStageFlag::Transfer, GpuAccessFlag::Write);
		const GpuAccessState hostRead(GpuStageFlag::Host, GpuAccessFlag::Read);
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

			// Returns the state of the attachment in a render pass with @p readOnlyMask
			const GpuStageFlags attachmentStages = depth ? GpuStageFlag::EarlyFragmentTests | GpuStageFlag::LateFragmentTests : GpuStageFlags(GpuStageFlag::ColorAttachment);
			const auto fnGetAttachmentState = [depth, surfaces, attachmentStages](RenderSurfaceMask readOnlyMask)
			{
				const GpuImageLayout layout = GpuBackendUtility::GetRenderPassAttachmentLayout(depth ? RT_DEPTH : RT_COLOR0, readOnlyMask);
				const bool isReadOnly = (readOnlyMask & surfaces) == surfaces;
				return GpuAccessState(attachmentStages, isReadOnly ? GpuAccessFlags(GpuAccessFlag::Read) : GpuAccessFlag::Read | GpuAccessFlag::Write, layout);
			};

			// The new texture starts at rest. Every case starts by discarding the contents.
			GpuAccessState textureState = GpuBackendUtility::GetRestingState(*texture, GQT_GRAPHICS);
			GpuAccessState readbackState;
			for(RenderSurfaceMask readOnlyMask : { RenderSurfaceMask(RT_NONE), surfaces, RenderSurfaceMask(RT_DEPTH), RenderSurfaceMask(RT_STENCIL) })
			{
				if(!depth && (readOnlyMask == RT_DEPTH || readOnlyMask == RT_STENCIL))
					continue;

				const TShared<render::GpuCommandBuffer> commands = commandPool->Create(GpuCommandBufferCreateInformation::Create("Render-pass clear, load and readback"));
				RenderPassCreateInformation pass(target);

				// Discard followed by clear also exercises reuse of an attachment without preserving its contents.
				const GpuAccessState writeState = fnGetAttachmentState(RT_NONE);
				GpuAccessState discardedState = textureState;
				discardedState.Layout = GpuImageLayout::Undefined;
				commands->IssueExplicitBarriers(GpuExplicitTextureBarrier(texture, discardedState, writeState));
				commands->BeginRenderPass(pass);
				commands->EndRenderPass();
				pass.ClearMask = surfaces;
				commands->BeginRenderPass(pass);
				commands->EndRenderPass();

				const GpuAccessState readOnlyState = fnGetAttachmentState(readOnlyMask);
				commands->IssueExplicitBarriers(GpuExplicitTextureBarrier(texture, writeState, readOnlyState));
				pass.ClearMask = RT_NONE;
				pass.LoadMask = surfaces;
				pass.ReadOnlyMask = readOnlyMask;
				commands->BeginRenderPass(pass);
				commands->EndRenderPass();

				// Consecutive read-only passes keep the contents
				commands->BeginRenderPass(pass);
				commands->EndRenderPass();

				const GpuExplicitBufferBarrier readbackBarrier(readback, readbackState, transferWrite);
				const GpuExplicitTextureBarrier copyBarrier(texture, readOnlyState, transferRead);
				commands->IssueExplicitBarriers(GpuExplicitBarriers(TArrayView<const GpuExplicitBufferBarrier>(&readbackBarrier, 1), TArrayView<const GpuExplicitTextureBarrier>(&copyBarrier, 1)));
				commands->CopyTextureToBuffer(texture, readback, 0, 0);
				commands->IssueExplicitBarriers(GpuExplicitBufferBarrier(readback, transferWrite, hostRead));
				textureState = transferRead;
				readbackState = hostRead;
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
		const TShared<render::GpuCommandBufferPool> graphicsPool = GpuBarrierTestUtility::CreateCommandBufferPool(*device, GQT_GRAPHICS);
		const TShared<render::GpuCommandBufferPool> computePool = device->GetQueueCount(GQT_COMPUTE) > 0 ? GpuBarrierTestUtility::CreateCommandBufferPool(*device, GQT_COMPUTE) : graphicsPool;
		const TShared<render::RenderTarget> target = window;
		const GpuAccessState restingState = GpuBackendUtility::GetRestingState(*window, RT_COLOR0, GQT_GRAPHICS);
		const GpuAccessState colorAttachment(GpuStageFlag::ColorAttachment, GpuAccessFlag::Read | GpuAccessFlag::Write, GpuImageLayout::ColorAttachment);
		for(u32 generation = 0; generation < 2; generation++)
		{
			// A newly acquired image must wait for acquisition even when nothing is drawn before presenting it.
			B3D_TEST_ASSERT(surface.GetActiveFramebuffer() != nullptr)
			surface.SwapBuffers(*queue, GpuQueueMask::kNone);
			for(u32 readbackMode = 0; readbackMode < 3; readbackMode++)
			{
				const TShared<render::GpuCommandBuffer> commands = graphicsPool->Create(GpuCommandBufferCreateInformation::Create("Swapchain clear and load"));
				GpuAccessState discardedRest = restingState;
				discardedRest.Layout = GpuImageLayout::Undefined;
				commands->IssueExplicitBarriers(GpuExplicitRenderTargetBarrier(target, RT_COLOR0, discardedRest, colorAttachment));

				RenderPassCreateInformation pass(window);
				pass.ClearMask = RT_COLOR0;
				commands->BeginRenderPass(pass);
				commands->EndRenderPass();
				pass.ClearMask = RT_NONE;
				pass.LoadMask = RT_COLOR0;
				commands->BeginRenderPass(pass);
				commands->EndRenderPass();
				commands->IssueExplicitBarriers(GpuExplicitRenderTargetBarrier(target, RT_COLOR0, colorAttachment, restingState));
				context->SubmitCommandBuffer(commands, GpuQueueMask::kNone);

				const TOptional<RenderWindowImageLayout> imageLayout = surface.GetImageLayout();
				B3D_TEST_ASSERT(imageLayout.has_value())

				TShared<render::GpuBuffer> readbackBuffer;
				GpuQueueMask presentWaitMask = GpuQueueMask::kNone;
				if(readbackMode != 0 && imageLayout.has_value())
				{
					GpuBufferCreateInformation readbackBufferCreateInformation;
					readbackBufferCreateInformation.Type = GpuBufferType::StagingRead;
					readbackBufferCreateInformation.Staging.Size = imageLayout->Pitch.RowPitch * PixelUtility::GetBlockSize(imageLayout->Format) * imageLayout->Pitch.SliceHeight;
					readbackBuffer = device->CreateGpuBuffer(readbackBufferCreateInformation);

					const TShared<render::GpuCommandBufferPool>& readbackPool = readbackMode == 1 ? graphicsPool : computePool;
					const TShared<render::GpuCommandBuffer> readCommands = readbackPool->Create(GpuCommandBufferCreateInformation::Create("Swapchain readback before present"));
					// A read on another queue waits for the graphics queue's writes, and presentation waits for the read
					const bool readsOnOtherQueue = readCommands->GetQueueType() != GQT_GRAPHICS;
					GpuBackendUtility::CopyToStagingBuffer(*readCommands, window, readbackBuffer);
					context->SubmitCommandBuffer(readCommands, readsOnOtherQueue ? GpuQueueMask(GpuQueueId(GQT_GRAPHICS, 0)) : GpuQueueMask::kNone);
					if(readsOnOtherQueue)
						presentWaitMask = GpuQueueMask(GpuQueueId(readCommands->GetQueueType(), 0));
				}

				surface.SwapBuffers(*queue, presentWaitMask);
				device->WaitUntilIdle();
				if(readbackBuffer != nullptr)
				{
					render::GpuBufferMappedScope mapping = readbackBuffer->Map(GpuMapOption::Read);
					PixelData pixels(imageLayout->Width, imageLayout->Height, 1, imageLayout->Format);
					pixels.SetExternalBuffer(static_cast<u8*>(mapping.GetMappedMemory()));
					B3D_TEST_ASSERT(pixels.GetColorAt(0, 0) == window->GetClearValues().Colors[0])
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
			const TShared<render::GpuCommandBufferPool> pool = GpuBarrierTestUtility::CreateCommandBufferPool(*device, queueType);
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
