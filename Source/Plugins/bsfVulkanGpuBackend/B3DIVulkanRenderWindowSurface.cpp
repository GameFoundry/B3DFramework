//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DIVulkanRenderWindowSurface.h"
#include "B3DVulkanFramebuffer.h"
#include "B3DVulkanGpuBackend.h"
#include "B3DVulkanGpuBuffer.h"
#include "B3DVulkanGpuCommandBuffer.h"
#include "B3DVulkanTexture.h"
#include "Image/B3DPixelUtility.h"

using namespace b3d;
using namespace b3d::render;

TAsyncOp<TShared<PixelData>> IVulkanRenderWindowSurface::ReadAsync(GpuCommandBuffer& commandBuffer)
{
	VulkanImage* colorImage = GetCurrentColorImage();
	if(colorImage == nullptr)
		return {};

	// Get image info from the framebuffer
	VulkanFramebuffer* framebuffer = GetActiveFramebuffer(false);
	if(framebuffer == nullptr)
		return {};

	const u32 width = framebuffer->GetWidth();
	const u32 height = framebuffer->GetHeight();
	const PixelFormat pixelFormat = GetColorPixelFormat();

	// Create pixel data for the result
	const TShared<PixelData> pixelData = B3DMakeShared<PixelData>(width, height, 1, pixelFormat);

	// Calculate buffer size needed
	const u32 bufferSize = PixelUtility::GetMemorySize(width, height, 1, pixelFormat);

	// Create staging buffer using the present device
	GpuBufferCreateInformation bufferCreateInfo;
	bufferCreateInfo.Type = GpuBufferType::StagingRead;
	bufferCreateInfo.Staging.Size = bufferSize;

	TShared<VulkanGpuDevice> presentDevice = GetVulkanGpuBackend().GetPresentDevice();
	TShared<GpuBuffer> stagingBuffer = presentDevice->CreateGpuBuffer(bufferCreateInfo);
	VulkanBuffer* vulkanBuffer = static_cast<VulkanGpuBuffer*>(stagingBuffer.get())->GetVulkanResource();

	// Set up image copy parameters
	VkExtent3D extent;
	extent.width = width;
	extent.height = height;
	extent.depth = 1;

	GpuTextureSubresourceRange subresourceRange;
	subresourceRange.AspectMask = GpuTextureAspectFlag::Color;
	subresourceRange.BaseMipLevel = 0;
	subresourceRange.MipLevelCount = 1;
	subresourceRange.BaseArrayLayer = 0;
	subresourceRange.ArrayLayerCount = 1;

	// Calculate row pitch in pixels (not bytes)
	const u32 rowPitch = width;
	const u32 sliceHeight = height;

	// Issue the copy command
	VulkanGpuCommandBuffer& vulkanCmdBuffer = static_cast<VulkanGpuCommandBuffer&>(commandBuffer);
	vulkanCmdBuffer.CopyImageToBuffer(colorImage, vulkanBuffer, extent, subresourceRange, GpuImageLayout::TransferSource, rowPitch, sliceHeight);

	// Set up async completion
	TAsyncOp<TShared<PixelData>> op;

	auto fnOnCommandBufferCompleted = [stagingBuffer, op, pixelData]() mutable
	{
		GpuBufferMappedScope mapping = stagingBuffer->Map(GpuMapOption::Read);

		pixelData->AllocateInternalBuffer();
		memcpy(pixelData->GetData(), mapping.GetMappedMemory(), pixelData->GetSize());

		op.CompleteOperation(pixelData);
	};

	auto fnOnCommandBufferDestroyed = [op](bool isSubmitted) mutable
	{
		if(isSubmitted)
			return;

		op.CompleteOperation(nullptr);
	};

	commandBuffer.OnDidComplete.Connect(fnOnCommandBufferCompleted);
	commandBuffer.OnDestroyed.Connect(fnOnCommandBufferDestroyed);

	return op;
}
