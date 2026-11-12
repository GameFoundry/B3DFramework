//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DVulkanGpuAccelerationStructure.h"
#include "B3DVulkanGpuBackend.h"
#include "B3DVulkanGpuBuffer.h"
#include "B3DVulkanGpuDevice.h"

using namespace b3d;
using namespace b3d::render;

VulkanAccelerationStructure::VulkanAccelerationStructure(VulkanResourceManager* owner, VulkanBuffer& buffer, const GpuAccelerationStructureCreateInformation& createInformation)
	: VulkanResource(owner, !buffer.IsExclusive(), createInformation.Name), mBuffer(&buffer)
{
	const VkDevice device = mOwner->GetDevice().GetLogical();

	VkAccelerationStructureCreateInfoKHR accelerationStructureCI;
	accelerationStructureCI.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
	accelerationStructureCI.pNext = nullptr;
	accelerationStructureCI.createFlags = 0;
	accelerationStructureCI.buffer = buffer.GetVulkanHandle();
	accelerationStructureCI.offset = 0;
	accelerationStructureCI.size = createInformation.Size;
	accelerationStructureCI.type = createInformation.Type == GpuAccelerationStructureType::TopLevel ? VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR : VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
	accelerationStructureCI.deviceAddress = 0;

	const VkResult createResult = vkCreateAccelerationStructureKHR(device, &accelerationStructureCI, gVulkanAllocator, &mAccelerationStructure);
	B3D_ASSERT(createResult == VK_SUCCESS);
	(void)createResult;

	// Read once, so builds, instance conversion on any thread and descriptor writes read the stored value
	VkAccelerationStructureDeviceAddressInfoKHR addressInfo;
	addressInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
	addressInfo.pNext = nullptr;
	addressInfo.accelerationStructure = mAccelerationStructure;

	mDeviceAddress = vkGetAccelerationStructureDeviceAddressKHR(device, &addressInfo);

	if(vkSetDebugUtilsObjectNameEXT != nullptr && !createInformation.Name.empty())
	{
		VkDebugUtilsObjectNameInfoEXT objectNameInfo;
		objectNameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
		objectNameInfo.pNext = nullptr;
		objectNameInfo.objectType = VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_KHR;
		objectNameInfo.objectHandle = (uint64_t)mAccelerationStructure;
		objectNameInfo.pObjectName = createInformation.Name.c_str();

		vkSetDebugUtilsObjectNameEXT(device, &objectNameInfo);
		buffer.SetName(createInformation.Name);
	}
}

VulkanAccelerationStructure::~VulkanAccelerationStructure()
{
	vkDestroyAccelerationStructureKHR(mOwner->GetDevice().GetLogical(), mAccelerationStructure, gVulkanAllocator);

	// Nothing else tracks or pins the buffer, so it is freed now
	mBuffer->Destroy();
}

VulkanGpuAccelerationStructure::VulkanGpuAccelerationStructure(VulkanGpuDevice& device, const GpuAccelerationStructureCreateInformation& createInformation,
	VulkanAccelerationStructure& accelerationStructure)
	: GpuAccelerationStructure(device, createInformation, accelerationStructure), mAccelerationStructure(&accelerationStructure)
{ }
