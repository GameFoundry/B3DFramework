//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DVulkanBarrierTestSuite.h"
#include "B3DVulkanGpuBackend.h"
#include "B3DVulkanGpuDevice.h"
#include "GpuBackend/B3DGpuBuffer.h"
#include "GpuBackend/B3DGpuCommandBuffer.h"
#include "GpuBackend/B3DGpuWorkContext.h"

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

	void RunBufferHandoff(TestSuite& testSuite, GpuQueueType sourceQueueType, GpuQueueType destinationQueueType)
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
		context->SubmitCommandBuffer(destinationCommandBuffer, GpuQueueMask::kNone);
		device->WaitUntilIdle();

		render::GpuBufferMappedScope readbackMapping = readbackBuffer->Map(GpuMapOption::Read);
		B3D_TEST_ASSERT_EXTERNAL(testSuite, readbackMapping.IsValid())
		if(readbackMapping.IsValid())
			B3D_TEST_ASSERT_EXTERNAL(testSuite, memcmp(readbackMapping.GetMappedMemory(), expected.data(), expected.size()) == 0)
	}
}

VulkanBarrierTestSuite::VulkanBarrierTestSuite()
	: TestSuite("VulkanBarrierTestSuite")
{
	B3D_ADD_TEST(VulkanBarrierTestSuite::TestGraphicsToComputeBufferHandoff)
	B3D_ADD_TEST(VulkanBarrierTestSuite::TestComputeToGraphicsBufferHandoff)
	B3D_ADD_TEST(VulkanBarrierTestSuite::TestSameQueueBufferBoundary)
}

void VulkanBarrierTestSuite::TestGraphicsToComputeBufferHandoff()
{
	RunBufferHandoff(*this, GQT_GRAPHICS, GQT_COMPUTE);
}

void VulkanBarrierTestSuite::TestComputeToGraphicsBufferHandoff()
{
	RunBufferHandoff(*this, GQT_COMPUTE, GQT_GRAPHICS);
}

void VulkanBarrierTestSuite::TestSameQueueBufferBoundary()
{
	RunBufferHandoff(*this, GQT_GRAPHICS, GQT_GRAPHICS);
}
