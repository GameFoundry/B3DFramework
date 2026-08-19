//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DVulkanBarrierTestSuite.h"
#include "B3DVulkanGpuBackend.h"
#include "B3DVulkanGpuDevice.h"
#include "B3DVulkanTexture.h"
#include "CoreObject/B3DRenderThread.h"
#include "GpuBackend/B3DGpuBuffer.h"
#include "GpuBackend/B3DGpuCommandBuffer.h"
#include "GpuBackend/B3DGpuWorkContext.h"
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
	B3D_ADD_TEST(VulkanBarrierTestSuite::TestConcurrentQueueReadTexture)
	B3D_ADD_TEST(VulkanBarrierTestSuite::TestMultisampleResolve)
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

void VulkanBarrierTestSuite::TestConcurrentQueueReadTexture()
{
	VulkanGpuDevice* const device = GetActiveVulkanDevice();
	if(device == nullptr)
		return;

	TInlineArray<u32, GQT_COUNT> queueFamilies;
	for(u32 queueType = 0; queueType < GQT_COUNT; queueType++)
	{
		const GpuQueueType type = (GpuQueueType)queueType;
		if(device->GetQueueCount(type) == 0)
			continue;

		const u32 family = device->GetQueueFamily(type);
		if(std::find(queueFamilies.begin(), queueFamilies.end(), family) == queueFamilies.end())
			queueFamilies.Add(family);
	}

	TextureCreateInformation createInformation;
	createInformation.Name = "Vulkan concurrent-read texture";
	createInformation.Format = PF_RGBA8;
	createInformation.Width = 8;
	createInformation.Height = 8;
	createInformation.Usage |= TextureUsageFlag::AllowConcurrentQueueReads;

	bool textureCreated = false;
	bool exclusive = true;
	GetRenderThread().PostCommand([device, createInformation, &textureCreated, &exclusive]()
	{
		const TShared<render::Texture> texture = device->CreateTexture(createInformation);
		textureCreated = texture != nullptr;
		if(!textureCreated)
			return;

		const TShared<VulkanTexture> vulkanTexture = std::static_pointer_cast<VulkanTexture>(texture);
		exclusive = vulkanTexture->GetVulkanResource()->IsExclusive();
	}, "VulkanBarrierTestSuite::TestConcurrentQueueReadTexture", true);

	B3D_TEST_ASSERT(textureCreated)
	B3D_TEST_ASSERT(exclusive == (queueFamilies.Size() <= 1))
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
