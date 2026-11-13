//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DVulkanGpuShaderBindingTable.h"
#include "B3DVulkanGpuBuffer.h"
#include "B3DVulkanGpuDevice.h"
#include "B3DVulkanGpuPipelineState.h"
#include "GpuBackend/B3DGpuDeviceCapabilities.h"
#include "Utility/B3DBitwise.h"

using namespace b3d;
using namespace b3d::render;

VulkanGpuShaderBindingTable::VulkanGpuShaderBindingTable(VulkanGpuDevice& device, const GpuShaderBindingTableCreateInformation& createInformation)
	: GpuShaderBindingTable(createInformation)
{
	const GpuRayTracingCapabilities& rayTracing = device.GetCapabilities().RayTracing;
	const VulkanGpuRayTracingPipelineState& pipeline = static_cast<const VulkanGpuRayTracingPipelineState&>(*mInformation.Pipeline);
	const GpuRayTracingPipelineStateInformation& programs = pipeline.GetInformation();

	// A record is the handle of its group. Every region starts at a multiple of the table alignment. Ray generation records are that far
	// apart too, because a dispatch passes one of them as a region of its own.
	const u32 handleSize = rayTracing.ShaderGroupHandleSize;
	const u32 tableAlignment = rayTracing.ShaderTableAlignment;
	const u32 recordStride = Bitwise::AlignUp(handleSize, rayTracing.ShaderRecordAlignment);
	const u32 rayGenerationStride = Bitwise::AlignUp(handleSize, std::max(rayTracing.ShaderRecordAlignment, tableAlignment));

	// Regions hold their offset in the buffer until the buffer exists. A region without records stays zeros.
	u32 tableSize = 0;
	const auto fnPlaceRegion = [&tableSize, tableAlignment](const Vector<u32>& records, u32 stride, VkStridedDeviceAddressRegionKHR& region)
	{
		if(records.empty())
			return;

		tableSize = Bitwise::AlignUp(tableSize, tableAlignment);
		region.deviceAddress = tableSize;
		region.stride = stride;
		region.size = (VkDeviceSize)stride * records.size();
		tableSize += (u32)region.size;
	};

	fnPlaceRegion(mInformation.RayGenerationRecords, rayGenerationStride, mRayGenerationRegion);
	fnPlaceRegion(mInformation.MissRecords, recordStride, mMissRegion);
	fnPlaceRegion(mInformation.HitGroupRecords, recordStride, mHitGroupRegion);
	fnPlaceRegion(mInformation.CallableRecords, recordStride, mCallableRegion);

	// Read on any queue without ownership transfers, like other buffers
	const TInlineArray<u32, GQT_COUNT>& queueFamilies = device.GetQueueFamilies();
	const bool usesConcurrentSharing = queueFamilies.Size() > 1;

	VulkanBufferCreateInformation bufferCreateInformation;
	bufferCreateInformation.VkCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInformation.VkCreateInfo.pNext = nullptr;
	bufferCreateInformation.VkCreateInfo.flags = 0;
	bufferCreateInformation.VkCreateInfo.size = tableSize;
	bufferCreateInformation.VkCreateInfo.usage = VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
	bufferCreateInformation.VkCreateInfo.sharingMode = usesConcurrentSharing ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE;
	bufferCreateInformation.VkCreateInfo.queueFamilyIndexCount = usesConcurrentSharing ? (u32)queueFamilies.Size() : 0;
	bufferCreateInformation.VkCreateInfo.pQueueFamilyIndices = usesConcurrentSharing ? queueFamilies.data() : nullptr;
	bufferCreateInformation.Type = GpuBufferType::StructuredStorage;
	bufferCreateInformation.Flags = GpuBufferFlag::StoreOnCPUWithGPUAccess;
	bufferCreateInformation.DebugName = "Shader binding table";
	bufferCreateInformation.MinimumAlignment = tableAlignment;

	mBuffer = device.CreateBuffer(bufferCreateInformation, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	const VkDeviceAddress tableAddress = mBuffer->GetDeviceAddress();
	for(VkStridedDeviceAddressRegionKHR* region : { &mRayGenerationRegion, &mMissRegion, &mHitGroupRegion, &mCallableRegion })
	{
		if(region->size > 0)
			region->deviceAddress += tableAddress;
	}

	// Groups are numbered ray generation programs first, then miss programs, hit groups and callable programs. An empty hit group record
	// stays zeros, which runs no program. The host writes are visible to the GPU once a command buffer that uses the table is submitted.
	u8* const tableData = mBuffer->Map(0, tableSize);
	std::memset(tableData, 0, tableSize);

	const auto fnWriteRecords = [&pipeline, tableData, tableAddress, handleSize](const Vector<u32>& records, const VkStridedDeviceAddressRegionKHR& region, u32 firstGroupIndex)
	{
		for(u32 recordIndex = 0; recordIndex < (u32)records.size(); recordIndex++)
		{
			if(records[recordIndex] == kGpuShaderBindingTableEmptyRecord)
				continue;

			const TArrayView<const u8> handle = pipeline.GetShaderGroupHandle(firstGroupIndex + records[recordIndex]);
			B3D_ASSERT(handle.size() == handleSize);

			u8* const record = tableData + (region.deviceAddress - tableAddress) + (VkDeviceSize)recordIndex * region.stride;
			std::memcpy(record, handle.data(), handleSize);
		}
	};

	const u32 firstMissGroupIndex = (u32)programs.RayGenerationPrograms.size();
	const u32 firstHitGroupIndex = firstMissGroupIndex + (u32)programs.MissPrograms.size();
	const u32 firstCallableGroupIndex = firstHitGroupIndex + (u32)programs.HitGroups.size();
	fnWriteRecords(mInformation.RayGenerationRecords, mRayGenerationRegion, 0);
	fnWriteRecords(mInformation.MissRecords, mMissRegion, firstMissGroupIndex);
	fnWriteRecords(mInformation.HitGroupRecords, mHitGroupRegion, firstHitGroupIndex);
	fnWriteRecords(mInformation.CallableRecords, mCallableRegion, firstCallableGroupIndex);

	mBuffer->Unmap(true);
}

VulkanGpuShaderBindingTable::~VulkanGpuShaderBindingTable()
{
	// Freed once no command buffer that dispatched with the table still runs
	mBuffer->Destroy();
}

VkStridedDeviceAddressRegionKHR VulkanGpuShaderBindingTable::GetRayGenerationRegion(u32 recordIndex) const
{
	VkStridedDeviceAddressRegionKHR region;
	region.deviceAddress = mRayGenerationRegion.deviceAddress + (VkDeviceSize)recordIndex * mRayGenerationRegion.stride;
	region.stride = mRayGenerationRegion.stride;
	region.size = mRayGenerationRegion.stride;

	return region;
}
