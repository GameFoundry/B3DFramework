//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DVulkanBindlessHeaps.h"
#include "B3DVulkanDescriptorLayout.h"
#include "B3DVulkanGpuBuffer.h"
#include "B3DVulkanGpuDevice.h"
#include "B3DVulkanSamplerState.h"
#include "B3DVulkanTexture.h"

using namespace b3d;
using namespace b3d::render;

VulkanBindlessHeap::VulkanBindlessHeap(VulkanResourceManager* owner, u32 capacity, VkDevice device, VkDescriptorPool pool, VkDescriptorSet set)
	: TVulkanResource<GpuBindlessHeap>(owner, false, capacity), mDevice(device), mPool(pool), mSet(set)
{ }

VulkanBindlessHeap::~VulkanBindlessHeap()
{
	// Destroying the pool frees the set
	vkDestroyDescriptorPool(mDevice, mPool, gVulkanAllocator);
}

VulkanBindlessHeaps::VulkanBindlessHeaps(VulkanGpuDevice& device, const IGpuCompletionTracker& completionTracker, VulkanDescriptorLayout& heapLayout,
	bool supportsNullDescriptor, const GpuBindlessHeapsCreateInformation& createInformation)
	: GpuBindlessHeaps(device, completionTracker, createInformation), mDevice(device), mHeapLayout(heapLayout), mSupportsNullDescriptor(supportsNullDescriptor)
	, mSamplerCapacity(createInformation.SamplerCapacity)
{ }

VulkanBindlessHeaps::~VulkanBindlessHeaps()
{
	Destroy();
}

void VulkanBindlessHeaps::WriteIntoCurrentHeap(VkWriteDescriptorSet& writeInfo)
{
	// Work using other native heaps was recorded before the write, so it never reads the slot
	writeInfo.dstSet = static_cast<VulkanBindlessHeap*>(GetResourceHeapPool().GetCurrent())->GetSet();
	vkUpdateDescriptorSets(mDevice.GetLogical(), 1, &writeInfo, 0, nullptr);

	GetResourceHeapPool().MarkSlotWritten(writeInfo.dstArrayElement);
}

void VulkanBindlessHeaps::WriteResourceSlot(u32 slotIndex, const GpuBindlessResourceView& view)
{
	WriteResourceDescriptor(slotIndex, view, nullptr);
}

void VulkanBindlessHeaps::WriteResourceDescriptor(u32 slotIndex, const GpuBindlessResourceView& view, VulkanBindlessHeap* heap)
{
	VkDescriptorImageInfo imageInfo;
	VkDescriptorBufferInfo bufferInfo;
	VkBufferView bufferView = VK_NULL_HANDLE;

	VkWriteDescriptorSet writeInfo;
	writeInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeInfo.pNext = nullptr;
	writeInfo.dstSet = VK_NULL_HANDLE;
	writeInfo.dstBinding = 1;
	writeInfo.dstArrayElement = slotIndex;
	writeInfo.descriptorCount = 1;
	writeInfo.pImageInfo = nullptr;
	writeInfo.pBufferInfo = nullptr;
	writeInfo.pTexelBufferView = nullptr;

	if(view.Texture != nullptr)
	{
		auto* texture = static_cast<VulkanTexture*>(view.Texture);

		imageInfo.sampler = VK_NULL_HANDLE;
		imageInfo.imageView = texture->GetVulkanResource()->GetView(view.Surface, false).Handle;
		imageInfo.imageLayout = texture->UsesGeneralLayout() ? VK_IMAGE_LAYOUT_GENERAL : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		writeInfo.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		writeInfo.pImageInfo = &imageInfo;
	}
	else
	{
		// Raw views of vertex and index buffers are storage buffers too
		auto* buffer = static_cast<VulkanGpuBuffer*>(view.Buffer);
		if(view.ViewType == GpuBufferViewType::Raw || buffer->GetInformation().Type == GpuBufferType::StructuredStorage)
		{
			bufferInfo.buffer = buffer->GetVulkanResource()->GetVulkanHandle();
			bufferInfo.offset = 0;
			bufferInfo.range = VK_WHOLE_SIZE;

			writeInfo.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			writeInfo.pBufferInfo = &bufferInfo;
		}
		else
		{
			bufferView = buffer->GetOrCreateView(view.Format);

			writeInfo.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
			writeInfo.pTexelBufferView = &bufferView;
		}
	}

	if(heap == nullptr)
		WriteIntoCurrentHeap(writeInfo);
	else
	{
		writeInfo.dstSet = heap->GetSet();
		vkUpdateDescriptorSets(mDevice.GetLogical(), 1, &writeInfo, 0, nullptr);
	}
}

void VulkanBindlessHeaps::WriteSamplerSlot(u32 slotIndex, const SamplerState& sampler)
{
	VkDescriptorImageInfo imageInfo;
	imageInfo.sampler = static_cast<const VulkanSamplerState&>(sampler).GetVulkanResource()->GetVulkanHandle();
	imageInfo.imageView = VK_NULL_HANDLE;
	imageInfo.imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	VkWriteDescriptorSet writeInfo;
	writeInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeInfo.pNext = nullptr;
	writeInfo.dstSet = VK_NULL_HANDLE;
	writeInfo.dstBinding = 0;
	writeInfo.dstArrayElement = slotIndex;
	writeInfo.descriptorCount = 1;
	writeInfo.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
	writeInfo.pImageInfo = &imageInfo;
	writeInfo.pBufferInfo = nullptr;
	writeInfo.pTexelBufferView = nullptr;

	// Every native heap's set holds its own copy of the sampler heap, which isn't versioned. Nothing reads the slot, so native heaps
	// command buffers still use can take it.
	GetResourceHeapPool().ForEachHeap([this, &writeInfo](GpuBindlessHeap& heap)
	{
		writeInfo.dstSet = static_cast<VulkanBindlessHeap&>(heap).GetSet();
		vkUpdateDescriptorSets(mDevice.GetLogical(), 1, &writeInfo, 0, nullptr);
	});

	mSamplerSlotCount = std::max(mSamplerSlotCount, slotIndex + 1);
}

void VulkanBindlessHeaps::ClearResourceSlot(u32 slotIndex, GpuParameterObjectType viewType)
{
	// The descriptor type must match the type shaders read the slot as. Without null descriptors, the slot views a dummy buffer
	// instead, which is only well defined for buffers read as structured or raw.
	VkDescriptorImageInfo imageInfo;
	imageInfo.sampler = VK_NULL_HANDLE;
	imageInfo.imageView = VK_NULL_HANDLE;
	imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	VkDescriptorBufferInfo bufferInfo;
	bufferInfo.buffer = mSupportsNullDescriptor ? VK_NULL_HANDLE : mDevice.GetBuiltinResources().DummyStructuredBuffer->GetVulkanResource()->GetVulkanHandle();
	bufferInfo.offset = 0;
	bufferInfo.range = VK_WHOLE_SIZE;

	const VkBufferView bufferView = VK_NULL_HANDLE;

	VkWriteDescriptorSet writeInfo;
	writeInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeInfo.pNext = nullptr;
	writeInfo.dstSet = VK_NULL_HANDLE;
	writeInfo.dstBinding = 1;
	writeInfo.dstArrayElement = slotIndex;
	writeInfo.descriptorCount = 1;
	writeInfo.pImageInfo = nullptr;
	writeInfo.pBufferInfo = nullptr;
	writeInfo.pTexelBufferView = nullptr;

	if(mSupportsNullDescriptor && GpuObjectParameterTypeInformation::IsTexture(viewType))
	{
		writeInfo.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		writeInfo.pImageInfo = &imageInfo;
	}
	else if(mSupportsNullDescriptor && viewType == GPOT_TYPED_BUFFER)
	{
		writeInfo.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
		writeInfo.pTexelBufferView = &bufferView;
	}
	else
	{
		writeInfo.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		writeInfo.pBufferInfo = &bufferInfo;
	}

	WriteIntoCurrentHeap(writeInfo);
}

GpuBindlessHeap* VulkanBindlessHeaps::CreateResourceHeap(u32 capacity)
{
	const VkDevice device = mDevice.GetLogical();

	// The pool's mutable descriptors use the same type list as the binding
	const VkDescriptorType resourceTypes[3] = { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER };

	VkMutableDescriptorTypeListEXT mutableTypeLists[2];
	mutableTypeLists[0].descriptorTypeCount = 0;
	mutableTypeLists[0].pDescriptorTypes = nullptr;
	mutableTypeLists[1].descriptorTypeCount = 3;
	mutableTypeLists[1].pDescriptorTypes = resourceTypes;

	VkMutableDescriptorTypeCreateInfoEXT mutableTypeCI;
	mutableTypeCI.sType = VK_STRUCTURE_TYPE_MUTABLE_DESCRIPTOR_TYPE_CREATE_INFO_EXT;
	mutableTypeCI.pNext = nullptr;
	mutableTypeCI.mutableDescriptorTypeListCount = 2;
	mutableTypeCI.pMutableDescriptorTypeLists = mutableTypeLists;

	VkDescriptorPoolSize poolSizes[2];
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_SAMPLER;
	poolSizes[0].descriptorCount = mSamplerCapacity;
	poolSizes[1].type = VK_DESCRIPTOR_TYPE_MUTABLE_EXT;
	poolSizes[1].descriptorCount = capacity;

	VkDescriptorPoolCreateInfo poolCI;
	poolCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolCI.pNext = &mutableTypeCI;
	poolCI.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
	poolCI.maxSets = 1;
	poolCI.poolSizeCount = 2;
	poolCI.pPoolSizes = poolSizes;

	VkDescriptorPool pool;
	if(vkCreateDescriptorPool(device, &poolCI, gVulkanAllocator, &pool) != VK_SUCCESS)
	{
		B3D_LOG(Error, LogRenderBackend, "Failed to create a descriptor pool for a bindless heap set of {0} resource slots.", capacity);
		return nullptr;
	}

	VkDescriptorSetVariableDescriptorCountAllocateInfo variableCountAI;
	variableCountAI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO;
	variableCountAI.pNext = nullptr;
	variableCountAI.descriptorSetCount = 1;
	variableCountAI.pDescriptorCounts = &capacity;

	const VkDescriptorSetLayout heapLayout = mHeapLayout.GetVulkanHandle();

	VkDescriptorSetAllocateInfo setAI;
	setAI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	setAI.pNext = &variableCountAI;
	setAI.descriptorPool = pool;
	setAI.descriptorSetCount = 1;
	setAI.pSetLayouts = &heapLayout;

	VkDescriptorSet set;
	if(vkAllocateDescriptorSets(device, &setAI, &set) != VK_SUCCESS)
	{
		B3D_LOG(Error, LogRenderBackend, "Failed to allocate a bindless heap set of {0} resource slots.", capacity);
		vkDestroyDescriptorPool(device, pool, gVulkanAllocator);
		return nullptr;
	}

	// The sampler heap is not versioned, but every native heap's set holds its own copy of it. The resource slots are copied by the caller.
	if(mSamplerSlotCount > 1)
	{
		VkCopyDescriptorSet copyInfo;
		copyInfo.sType = VK_STRUCTURE_TYPE_COPY_DESCRIPTOR_SET;
		copyInfo.pNext = nullptr;
		copyInfo.srcSet = static_cast<VulkanBindlessHeap*>(GetResourceHeapPool().GetCurrent())->GetSet();
		copyInfo.srcBinding = 0;
		copyInfo.srcArrayElement = 1;
		copyInfo.dstSet = set;
		copyInfo.dstBinding = 0;
		copyInfo.dstArrayElement = 1;
		copyInfo.descriptorCount = mSamplerSlotCount - 1;

		vkUpdateDescriptorSets(device, 0, nullptr, 1, &copyInfo);
	}

	return mDevice.GetResourceManager().Create<VulkanBindlessHeap>(capacity, device, pool, set);
}

void VulkanBindlessHeaps::CopyResourceSlots(GpuBindlessHeap& destination, TArrayView<const TIndexRange<u32>> ranges)
{
	// The current native heap holds the latest descriptor of every slot. Only the destination of a copy may not be in use.
	const VkDescriptorSet currentSet = static_cast<VulkanBindlessHeap*>(GetResourceHeapPool().GetCurrent())->GetSet();

	FrameAllocatorScope frameScope;
	FrameVector<VkCopyDescriptorSet> copies(ranges.Size());

	for(u32 rangeIndex = 0; rangeIndex < (u32)ranges.Size(); rangeIndex++)
	{
		VkCopyDescriptorSet& copyInfo = copies[rangeIndex];
		copyInfo.sType = VK_STRUCTURE_TYPE_COPY_DESCRIPTOR_SET;
		copyInfo.pNext = nullptr;
		copyInfo.srcSet = currentSet;
		copyInfo.srcBinding = 1;
		copyInfo.srcArrayElement = ranges[rangeIndex].First;
		copyInfo.dstSet = static_cast<VulkanBindlessHeap&>(destination).GetSet();
		copyInfo.dstBinding = 1;
		copyInfo.dstArrayElement = ranges[rangeIndex].First;
		copyInfo.descriptorCount = ranges[rangeIndex].Count;
	}

	vkUpdateDescriptorSets(mDevice.GetLogical(), 0, nullptr, (u32)copies.size(), copies.data());
}

void VulkanBindlessHeaps::WriteResourceSlots(GpuBindlessHeap& destination, TArrayView<const GpuBindlessSlotUpdate> updates)
{
	for(const GpuBindlessSlotUpdate& update : updates)
		WriteResourceDescriptor(update.SlotIndex, update.View, &static_cast<VulkanBindlessHeap&>(destination));
}
