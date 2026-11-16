//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DVulkanGpuShaderBindingTable.h"
#include "B3DVulkanGpuBuffer.h"
#include "B3DVulkanGpuDevice.h"
#include "B3DVulkanGpuPipelineState.h"
#include "GpuBackend/B3DGpuDeviceCapabilities.h"

using namespace b3d;
using namespace b3d::render;

VulkanGpuShaderBindingTable::VulkanGpuShaderBindingTable(VulkanGpuDevice& device, const GpuShaderBindingTableCreateInformation& createInformation)
	: GpuShaderBindingTable(createInformation, device.GetCapabilities().RayTracing)
{
	const VulkanGpuRayTracingPipelineState& pipeline = static_cast<const VulkanGpuRayTracingPipelineState&>(*mInformation.Pipeline);

	// Read on any queue without ownership transfers, like other buffers
	const TInlineArray<u32, GQT_COUNT>& queueFamilies = device.GetQueueFamilies();
	const bool usesConcurrentSharing = queueFamilies.Size() > 1;

	VulkanBufferCreateInformation bufferCreateInformation;
	bufferCreateInformation.VkCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInformation.VkCreateInfo.pNext = nullptr;
	bufferCreateInformation.VkCreateInfo.flags = 0;
	bufferCreateInformation.VkCreateInfo.size = GetSize();
	bufferCreateInformation.VkCreateInfo.usage = VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
	bufferCreateInformation.VkCreateInfo.sharingMode = usesConcurrentSharing ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE;
	bufferCreateInformation.VkCreateInfo.queueFamilyIndexCount = usesConcurrentSharing ? (u32)queueFamilies.Size() : 0;
	bufferCreateInformation.VkCreateInfo.pQueueFamilyIndices = usesConcurrentSharing ? queueFamilies.data() : nullptr;
	bufferCreateInformation.Type = GpuBufferType::StructuredStorage;
	bufferCreateInformation.DebugName = "Shader binding table";
	bufferCreateInformation.MinimumAlignment = device.GetCapabilities().RayTracing.ShaderTableAlignment;

	// The CPU writes the table once, through a mapping, into the GPU's local memory if it can
	VulkanMemoryFlags memoryFlags;
	TOptional<VulkanMemoryFlags> fallbackMemoryFlags;
	VulkanGpuBuffer::GetMemoryFlags(device, bufferCreateInformation.Type, GpuBufferMemory::Mapped, bufferCreateInformation.Flags, memoryFlags,
		fallbackMemoryFlags);

	mBuffer = device.CreateBuffer(bufferCreateInformation, memoryFlags, fallbackMemoryFlags);

	// The host writes are visible to the GPU once a command buffer that uses the table is submitted
	WriteRecords(mBuffer->Map(0, GetSize()), [&pipeline](u32 groupIndex) { return pipeline.GetShaderGroupHandle(groupIndex); });
	mBuffer->Unmap(true);
}

VulkanGpuShaderBindingTable::~VulkanGpuShaderBindingTable()
{
	// Freed once no command buffer that dispatched with the table still runs
	mBuffer->Destroy();
}

VkStridedDeviceAddressRegionKHR VulkanGpuShaderBindingTable::GetRayGenerationRecordRegion(u32 recordIndex) const
{
	const GpuShaderBindingTableRegion& records = GetRayGenerationRegion();

	VkStridedDeviceAddressRegionKHR region;
	region.deviceAddress = mBuffer->GetDeviceAddress() + records.Offset + (VkDeviceSize)recordIndex * records.Stride;
	region.stride = records.Stride;
	region.size = records.Stride;

	return region;
}

VkStridedDeviceAddressRegionKHR VulkanGpuShaderBindingTable::GetDeviceAddressRegion(const GpuShaderBindingTableRegion& region) const
{
	VkStridedDeviceAddressRegionKHR output{};
	if(region.Size == 0)
		return output;

	output.deviceAddress = mBuffer->GetDeviceAddress() + region.Offset;
	output.stride = region.Stride;
	output.size = region.Size;

	return output;
}
