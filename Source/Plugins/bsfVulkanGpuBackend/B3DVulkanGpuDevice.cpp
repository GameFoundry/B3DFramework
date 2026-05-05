//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DVulkanGpuDevice.h"

#include <algorithm>
#include "B3DVulkanGpuQueue.h"
#include "B3DVulkanGpuCommandBuffer.h"
#include "B3DVulkanGpuTimelineFence.h"
#include "B3DVulkanHeapBackend.h"
#include "B3DVulkanUtility.h"
#include "B3DVulkanGpuBackend.h"
#include "B3DVulkanSubmitThread.h"
#include "CoreObject/B3DRenderThread.h"
#include "Managers/B3DVulkanDescriptorManager.h"
#include "Managers/B3DVulkanQueries.h"
#include "GpuBackend/B3DGpuTransferBufferHelper.h"

#if B3D_PLATFORM_WIN32
#	include "Private/Win32/B3DWin32VideoModeInfo.h"
#elif B3D_PLATFORM_LINUX
#	include "Private/Linux/B3DLinuxVideoModeInfo.h"
#elif B3D_PLATFORM_MACOS
#	include "Private/MacOS/B3DMacOSVideoModeInfo.h"
#	include <MoltenVK/vk_mvk_moltenvk.h>
#else
static_assert(false, "Other platform includes go here.");
#endif

#include "B3DVulkanEventQuery.h"
#include "B3DGLSLToSPIRV.h"
#include "B3DVulkanGpuBuffer.h"
#include "B3DVulkanGpuParameterSetPool.h"
#include "B3DVulkanGpuParameterSet.h"
#include "B3DVulkanGpuPipelineParameterLayout.h"
#include "B3DVulkanGpuProgram.h"
#include "B3DVulkanSamplerState.h"
#include "B3DVulkanTexture.h"
#include "GpuBackend/B3DGpuProgramParameterDescription.h"
#include "GpuBackend/B3DGpuBackendUtility.h"
#include "Utility/B3DBitwise.h"

using namespace b3d;
using namespace b3d::render;

namespace
{
	/**
	 * Builds a VkMappedMemoryRange for @p [offset, offset+size) within @p allocation, expanding
	 * the range outward so the resulting offset and size satisfy
	 * - offset must be a multiple of nonCoherentAtomSize
	 * - size must be a multiple of nonCoherentAtomSize, OR offset+size must equal the device memory size.
	 *
	 * The allocator places allocations at offsets driven by the resource's required alignment, which
	 * is generally smaller than nonCoherentAtomSize (typically 64–256 bytes). Rounding the offset
	 * down and the end up always stays within the bounds of the parent device memory because the heap
	 * itself is sized to whole multiples of nonCoherentAtomSize per Vulkan spec.
	 */
	VkMappedMemoryRange BuildNonCoherentMappedMemoryRange(const VulkanAllocationResult& allocation, VkDeviceSize offset, VkDeviceSize size, VkDeviceSize atom)
	{
		const VkDeviceSize absOffset = allocation.Location.Offset + offset;
		const VkDeviceSize absEnd = (size == VK_WHOLE_SIZE) ? (allocation.Location.Offset + allocation.Location.Size) : (absOffset + size);

		const VkDeviceSize atomMask = atom - 1;
		const VkDeviceSize alignedOffset = atom > 1 ? (absOffset & ~atomMask) : absOffset;
		VkDeviceSize alignedEnd = atom > 1 ? ((absEnd + atomMask) & ~atomMask) : absEnd;

		// Clamp the end against the underlying device memory in case the upward round overruns its tail.
		alignedEnd = std::min(alignedEnd, allocation.Location.Heap.Size);

		VkMappedMemoryRange range = {};
		range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
		range.memory = allocation.Location.Heap.Memory;
		range.offset = alignedOffset;
		range.size = alignedEnd - alignedOffset;

		return range;
	}
}

VulkanGpuDevice::VulkanGpuDevice(VkPhysicalDevice device)
	: mPhysicalDevice(device), mQueueInfos(), mBuiltinResources(*this)
{
	// Set to default
	for(u32 i = 0; i < GQT_COUNT; i++)
		mQueueInfos[i].FamilyIndex = (u32)-1;

	vkGetPhysicalDeviceProperties(device, &mDeviceProperties);
	vkGetPhysicalDeviceFeatures(device, &mDeviceFeatures);
	vkGetPhysicalDeviceMemoryProperties(device, &mMemoryProperties);

	uint32_t numQueueFamilies;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &numQueueFamilies, nullptr);

	Vector<VkQueueFamilyProperties> queueFamilyProperties(numQueueFamilies);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &numQueueFamilies, queueFamilyProperties.data());

	// Create queues
	const float defaultQueuePriorities[B3D_MAX_QUEUES_PER_TYPE] = { 0.0f };
	Vector<VkDeviceQueueCreateInfo> queueCreateInfos;

	auto fnPopulateQueueInfo = [&](GpuQueueType type, uint32_t familyIdx)
	{
		queueCreateInfos.push_back(VkDeviceQueueCreateInfo());

		VkDeviceQueueCreateInfo& createInfo = queueCreateInfos.back();
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		createInfo.pNext = nullptr;
		createInfo.flags = 0;
		createInfo.queueFamilyIndex = familyIdx;
		createInfo.queueCount = std::min(queueFamilyProperties[familyIdx].queueCount, (uint32_t)B3D_MAX_QUEUES_PER_TYPE);
		createInfo.pQueuePriorities = defaultQueuePriorities;

		mQueueInfos[type].FamilyIndex = familyIdx;
		mQueueInfos[type].Queues.resize(createInfo.queueCount, nullptr);
	};

	auto fnFindQueueWithMinimalMatchingSubset = [this, &queueFamilyProperties](VkQueueFlags requiredFlags)
	{
		static constexpr VkQueueFlags kAllQueueFlags = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT | VK_QUEUE_SPARSE_BINDING_BIT | VK_QUEUE_PROTECTED_BIT;

		u32 bestScore = Bitwise::CountSetBits(kAllQueueFlags) + 1; // Lower is better
		u32 bestScoreFamilyIndex = ~0u;

		// Look for dedicated compute queues
		for(u32 i = 0; i < (u32)queueFamilyProperties.size(); i++)
		{
			// Skip queue families that don't have the minimum set of required flags
			if(Bitwise::CountSetBits(queueFamilyProperties[i].queueFlags & requiredFlags) != Bitwise::CountSetBits(requiredFlags))
				continue;

			// Skip already assigned queue families
			bool familyAlreadyInUse = false;
			for(u32 queueUsageIndex = 0; queueUsageIndex < GQT_COUNT; ++queueUsageIndex)
			{
				if(mQueueInfos[queueUsageIndex].FamilyIndex == i)
				{
					familyAlreadyInUse = true;
					break;
				}
			}

			if (familyAlreadyInUse)
				continue;

			const VkQueueFlags kExtraFlags = kAllQueueFlags & ~requiredFlags;
			const u32 score = Bitwise::CountSetBits(queueFamilyProperties[i].queueFlags & kExtraFlags);

			if(score < bestScore)
			{
				bestScore = score;
				bestScoreFamilyIndex = i;
			}
		}

		return bestScoreFamilyIndex;
	};

	const u32 graphicsQueueFamilyIndex = fnFindQueueWithMinimalMatchingSubset(VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT);
	if(B3D_ENSURE(graphicsQueueFamilyIndex != ~0u))
		fnPopulateQueueInfo(GQT_GRAPHICS, graphicsQueueFamilyIndex);

	const u32 computeQueueFamilyIndex = fnFindQueueWithMinimalMatchingSubset(VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT);
	if(computeQueueFamilyIndex != ~0u)
		fnPopulateQueueInfo(GQT_COMPUTE, computeQueueFamilyIndex);

	const u32 transferQueueFamilyIndex = fnFindQueueWithMinimalMatchingSubset(VK_QUEUE_TRANSFER_BIT);
	if(transferQueueFamilyIndex != ~0u)
		fnPopulateQueueInfo(GQT_TRANSFER, transferQueueFamilyIndex);

	// Set up extensions. Slots: swapchain, maintenance1, maintenance2, timeline_semaphore (optional),
	// plus up to three more discovered below (dedicated_allocation, get_memory_requirements_2,
	// shader_viewport_index_layer).
	const char* extensions[8];
	uint32_t extensionCount = 0;

	extensions[extensionCount++] = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
	extensions[extensionCount++] = VK_KHR_MAINTENANCE1_EXTENSION_NAME;
	extensions[extensionCount++] = VK_KHR_MAINTENANCE2_EXTENSION_NAME;

	// Enumerate supported extensions
	uint32_t availableExtensionCount = 0;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &availableExtensionCount, nullptr);
	if(availableExtensionCount > 0)
	{
		Vector<VkExtensionProperties> availableExtensions(availableExtensionCount);
		if(vkEnumerateDeviceExtensionProperties(device, nullptr, &availableExtensionCount, availableExtensions.data()) == VK_SUCCESS)
		{
			for(auto entry : availableExtensions)
			{
				if(strcmp(entry.extensionName, VK_EXT_SHADER_VIEWPORT_INDEX_LAYER_EXTENSION_NAME) == 0)
				{
					extensions[extensionCount++] = VK_EXT_SHADER_VIEWPORT_INDEX_LAYER_EXTENSION_NAME;
				}
				else if(strcmp(entry.extensionName, VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME) == 0)
				{
					extensions[extensionCount++] = VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME;
					mSupportsTimelineSemaphore = true;
				}
			}
		}
	}

#if !B3D_PLATFORM_MACOS
	if(mSupportsTimelineSemaphore)
	{
		VkPhysicalDeviceTimelineSemaphoreFeaturesKHR timelineFeatureProbe = {};
		timelineFeatureProbe.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_FEATURES_KHR;

		VkPhysicalDeviceFeatures2 features2 = {};
		features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
		features2.pNext = &timelineFeatureProbe;

		vkGetPhysicalDeviceFeatures2(device, &features2);

		if(timelineFeatureProbe.timelineSemaphore != VK_TRUE)
			mSupportsTimelineSemaphore = false;
	}
#endif

	VkPhysicalDeviceTimelineSemaphoreFeaturesKHR timelineFeatures = {};
	if(mSupportsTimelineSemaphore)
	{
		timelineFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_FEATURES_KHR;
		timelineFeatures.pNext = nullptr;
		timelineFeatures.timelineSemaphore = VK_TRUE;
	}

	VkDeviceCreateInfo deviceInfo;
	deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceInfo.pNext = mSupportsTimelineSemaphore ? &timelineFeatures : nullptr;
	deviceInfo.flags = 0;
	deviceInfo.queueCreateInfoCount = (uint32_t)queueCreateInfos.size();
	deviceInfo.pQueueCreateInfos = queueCreateInfos.data();
	deviceInfo.pEnabledFeatures = &mDeviceFeatures;
	deviceInfo.enabledExtensionCount = extensionCount;
	deviceInfo.ppEnabledExtensionNames = extensions;
	deviceInfo.enabledLayerCount = 0;
	deviceInfo.ppEnabledLayerNames = nullptr;

	VkResult result = vkCreateDevice(device, &deviceInfo, gVulkanAllocator, &mLogicalDevice);
	B3D_ASSERT(result == VK_SUCCESS);

	if(mSupportsTimelineSemaphore)
		GET_DEVICE_PROC_ADDR(mLogicalDevice, GetSemaphoreCounterValueKHR)

	// Retrieve queues
	for(u32 i = 0; i < GQT_COUNT; i++)
	{
		u32 queueCount = (u32)mQueueInfos[i].Queues.size();
		for(u32 j = 0; j < queueCount; j++)
		{
			VkQueue queue;
			vkGetDeviceQueue(mLogicalDevice, mQueueInfos[i].FamilyIndex, j, &queue);

			mQueueInfos[i].Queues[j] = B3DMakeSharedFromExisting(new (B3DAllocate<VulkanGpuQueue>()) VulkanGpuQueue(*this, (GpuQueueType)i, j, queue));
		}
	}

	mHeapBackend = B3DMakeUnique<VulkanHeapBackend>(*this);

	// Initialize capabilities
	InitializeCapabilities();

	mDescriptorManager = B3DNew<VulkanDescriptorManager>(*this);
	mResourceManager = B3DNew<VulkanResourceManager>(*this);
	mBuiltinResources.Initialize();

	// Initialize video mode information
#if B3D_PLATFORM_WIN32
	mVideoModeInfo = B3DMakeShared<Win32VideoModeInfo>();
#elif B3D_PLATFORM_LINUX
	mVideoModeInfo = B3DMakeShared<LinuxVideoModeInfo>();
#elif B3D_PLATFORM_MACOS
	mVideoModeInfo = B3DMakeShared<MacOSVideoModeInfo>();
#else
	static_assert(false, "mVideoModeInfo needs to be created.");
#endif

	mTransferBufferHelper = B3DMakeUnique<GpuTransferBufferHelper>(*this, GpuQueueId(GQT_GRAPHICS, 0));
}

VulkanGpuDevice::~VulkanGpuDevice()
{
	// Destroy transfer buffer helper before other cleanup
	mTransferBufferHelper.reset();

	mCachedSamplerStates.clear();
	mBuiltinResources.Cleanup();

	for (u32 queueUsageIndex = 0; queueUsageIndex < GQT_COUNT; queueUsageIndex++)
	{
		for (auto& queue : mQueueInfos[queueUsageIndex].Queues)
			queue = nullptr;
	}

	B3DDelete(mDescriptorManager);

	// Needs to happen after query pool & command buffer pool shutdown, to ensure their resources are destroyed
	B3DDelete(mResourceManager);

	// Drain every per-memory-type allocator before tearing down the heap backend; the allocator
	// destructor releases all owned heaps via mHeapBackend, so the heap backend must outlive it.
	for (u32 typeIndex = 0; typeIndex < VK_MAX_MEMORY_TYPES; typeIndex++)
	{
		UPtr<TGpuTlsfAllocator<VulkanHeapBackend>>& allocator = mGpuMemoryAllocators[typeIndex];
		if (allocator != nullptr)
		{
			allocator->Flush(0, true);
			allocator.reset();
		}
	}

	mHeapBackend.reset();

	vkDestroyDevice(mLogicalDevice, gVulkanAllocator);
}

SPtr<GpuProgramBytecode> VulkanGpuDevice::CompileGpuProgramBytecode(const GpuProgramCreateInformation& createInformation) const
{
	if(!IsGpuProgramLanguageSupported(createInformation.Language))
		return nullptr;

#if B3D_PLATFORM_MACOS
	SPtr<GpuProgramBytecode> spirv = GLSLToSPIRV::Instance().Convert(createInformation, MOLTENVK_COMPILER_ID, MOLTENVK_COMPILER_VERSION);
	// We'll just re-purpose the existing data structure
	SPtr<GpuProgramBytecode> msl = spirv;
#else
	SPtr<GpuProgramBytecode> spirv = GLSLToSPIRV::Instance().Convert(createInformation, VULKAN_COMPILER_ID, VULKAN_COMPILER_VERSION);
#endif

#if B3D_PLATFORM_MACOS

	// SPIR-V failed to compile, just pass along the data structure with updated compiler ID
	if(spirv->instructions.size == 0 || !spirv->instructions.data)
	{
		msl->instructions = DataBlob();
		return msl;
	}

	B3D_ASSERT((spirv->instructions.size % sizeof(u32)) == 0);

	// Compile to MSL
	spirv_cross::CompilerMSL compiler((u32*)spirv->instructions.data, spirv->instructions.size / sizeof(u32));

	// Remap resource bindings
	if(msl->paramDesc)
	{
		spv::ExecutionModel stage;
		switch(createInformation.type)
		{
		case GPT_VERTEX_PROGRAM:
			stage = spv::ExecutionModelVertex;
			break;
		case GPT_FRAGMENT_PROGRAM:
			stage = spv::ExecutionModelFragment;
			break;
		case GPT_GEOMETRY_PROGRAM:
			stage = spv::ExecutionModelGeometry;
			break;
		case GPT_DOMAIN_PROGRAM:
			stage = spv::ExecutionModelTessellationEvaluation;
			break;
		case GPT_HULL_PROGRAM:
			stage = spv::ExecutionModelTessellationControl;
			break;
		case GPT_COMPUTE_PROGRAM:
			stage = spv::ExecutionModelGLCompute;
			break;
		default:
			B3D_ASSERT(false);
			break;
		}

		auto count = msl->paramDesc->paramBlocks.size() + msl->paramDesc->textures.size() + msl->paramDesc->samplers.size() + msl->paramDesc->loadStoreTextures.size() + msl->paramDesc->buffers.size();

		auto sortedEntries = B3DManagedStackAllocate<spirv_cross::MSLResourceBinding>((u32)count);
		size_t i = 0;

		for(auto& entry : msl->paramDesc->paramBlocks)
		{
			spirv_cross::MSLResourceBinding& binding = sortedEntries[i++];
			binding.stage = stage;
			binding.desc_set = entry.second.set;
			binding.binding = entry.second.slot;
			binding.msl_buffer = 2;
		}

		for(auto& entry : msl->paramDesc->textures)
		{
			spirv_cross::MSLResourceBinding& binding = sortedEntries[i++];
			binding.stage = stage;
			binding.desc_set = entry.second.set;
			binding.binding = entry.second.slot;
			binding.msl_buffer = 0;
		}

		for(auto& entry : msl->paramDesc->samplers)
		{
			spirv_cross::MSLResourceBinding& binding = sortedEntries[i++];
			binding.stage = stage;
			binding.desc_set = entry.second.set;
			binding.binding = entry.second.slot;
			binding.msl_buffer = 1;
		}

		for(auto& entry : msl->paramDesc->loadStoreTextures)
		{
			spirv_cross::MSLResourceBinding& binding = sortedEntries[i++];
			binding.stage = stage;
			binding.desc_set = entry.second.set;
			binding.binding = entry.second.slot;
			binding.msl_buffer = 0;
		}

		for(auto& entry : msl->paramDesc->buffers)
		{
			spirv_cross::MSLResourceBinding& binding = sortedEntries[i++];
			binding.stage = stage;
			binding.desc_set = entry.second.set;
			binding.binding = entry.second.slot;

			// Non-structured buffers treated as textures by MSL
			if(entry.second.type == GPOT_BYTE_BUFFER || entry.second.type == GPOT_RWBYTE_BUFFER)
				binding.msl_buffer = 0;
			else
				binding.msl_buffer = 2;
		}

		std::sort(sortedEntries + 0, sortedEntries + count, [](const spirv_cross::MSLResourceBinding& a, const spirv_cross::MSLResourceBinding& b)
				  {
					if(a.desc_set == b.desc_set)
						return a.binding < b.binding;

					return a.desc_set < b.desc_set; });

		u32 bufferIdx = 0;
		u32 samplerIdx = 0;
		u32 textureIdx = 0;

		for(i = 0; i < count; i++)
		{
			spirv_cross::MSLResourceBinding& binding = sortedEntries[i];
			switch(binding.msl_buffer)
			{
			default:
			case 0: // Texture
				binding.msl_sampler = binding.msl_buffer = binding.msl_texture = textureIdx++;
				break;
			case 1: // Sampler
				binding.msl_sampler = binding.msl_buffer = binding.msl_texture = samplerIdx++;
				break;
			case 2: // Buffer
				binding.msl_sampler = binding.msl_buffer = binding.msl_texture = bufferIdx++;
				break;
			}

			compiler.add_msl_resource_binding(binding);
		}
	}

	spirv_cross::CompilerMSL::Options mslOptions;
	mslOptions.msl_version = spirv_cross::CompilerMSL::Options::make_msl_version(2, 1);
	compiler.set_msl_options(mslOptions);

	spirv_cross::CompilerGLSL::Options glslOptions;
	glslOptions.separate_shader_objects = true;
	glslOptions.vulkan_semantics = true;
	glslOptions.vertex.flip_vert_y = true;

	compiler.set_common_options(glslOptions);
	std::string source = compiler.Compile();

	// Parse workgroup size for compute shaders
	u32 workgroupSize[3] = { 1, 1, 1 };
	if(createInformation.type == GPT_COMPUTE_PROGRAM)
	{
		spirv_cross::SPIREntryPoint spvEP;
		const auto& entryPoints = compiler.get_entry_points_and_stages();
		if(!entryPoints.empty())
		{
			auto& ep = entryPoints[0];
			spvEP = compiler.get_entry_point(ep.name, ep.execution_model);
		}

		workgroupSize[0] = spvEP.workgroup_size.X;
		workgroupSize[1] = spvEP.workgroup_size.Y;
		workgroupSize[2] = spvEP.workgroup_size.Z;
	}

	// Copy the source into destination buffer
	if(msl->instructions.data)
		B3DFree(msl->instructions.data);

	if(source.empty())
	{
		msl->instructions = DataBlob();
		return msl;
	}

	// Magic numbers as defined in vk_mvk_moltenvk.h
	constexpr u32 MVK_MSL_Source = 0x19960412;

	u32 size = (u32)source.size() + sizeof(MVK_MSL_Source) + 1;
	if(createInformation.type == GPT_COMPUTE_PROGRAM)
		size += sizeof(workgroupSize);

	u32 wordSize = Math::DivideAndRoundUp(size, 4U);

	u8* buffer = (u8*)B3DAllocate(wordSize * 4);
	u8* dst = buffer;

	if(createInformation.type == GPT_COMPUTE_PROGRAM)
	{
		memcpy(dst, workgroupSize, sizeof(workgroupSize));
		dst += sizeof(workgroupSize);
	}

	memcpy(dst, &MVK_MSL_Source, sizeof(MVK_MSL_Source));
	dst += sizeof(MVK_MSL_Source);

	memcpy(dst, source.data(), source.size());

	for(u32 i = size - 1; i < wordSize * 4; i++)
		buffer[i] = '\0';

	msl->instructions.size = wordSize * 4;
	msl->instructions.data = buffer;

	return msl;

	// TODO - Compile the Metal source code into intermediate representation, right now we aren't outputting bytecode,
	// just for the sake of trying to get MoltenVK port running in the first place.
	// (Ideally we can also move GLSL->SPIRV->MSL steps to the shader importer, so we just receive pure MSL here, as that
	// would make the system ready for when we have a proper MSL cross-compiler. Downside of this approach is that
	// we then need shader reflection code for MSL).
#else
	return spirv;
#endif
}

SPtr<GpuQueue> VulkanGpuDevice::GetQueue(GpuQueueType type, u32 index) const
{
	if (index >= GetQueueCount(type))
		return nullptr;

	return mQueueInfos[(u32)type].Queues[index];
}

SPtr<GpuCommandBufferPool> VulkanGpuDevice::CreateGpuCommandBufferPool(const render::GpuCommandBufferPoolCreateInformation& createInformation)
{
	return B3DMakeSharedFromExisting(new(B3DAllocate<VulkanGpuCommandBufferPool>()) VulkanGpuCommandBufferPool(*this, createInformation));
}

SPtr<render::Texture> VulkanGpuDevice::CreateTexture(const TextureCreateInformation& createInformation, GpuObjectCreateFlags flags)
{
	VulkanTexture* rawTexture = new(B3DAllocate<VulkanTexture>()) VulkanTexture(*this, createInformation);

	SPtr<Texture> output = flags.IsSet(GpuObjectCreateFlag::RenderThreadDestroy)
		? B3DMakeSharedFromExisting(rawTexture)
		: MakeSharedStandalone<Texture>(rawTexture);

	output->SetShared(output);

	if(!flags.IsSet(GpuObjectCreateFlag::DeferredInitialize))
		output->Initialize();

	return output;
}

SPtr<render::GpuBuffer> VulkanGpuDevice::CreateGpuBuffer(const GpuBufferCreateInformation& createInformation, GpuObjectCreateFlags flags)
{
	VulkanGpuBuffer* rawBuffer = new(B3DAllocate<VulkanGpuBuffer>()) VulkanGpuBuffer(*this, createInformation);

	SPtr<GpuBuffer> output = flags.IsSet(GpuObjectCreateFlag::RenderThreadDestroy)
		? B3DMakeSharedFromExisting(rawBuffer)
		: MakeSharedStandalone<GpuBuffer>(rawBuffer);

	output->SetShared(output);

	if(!flags.IsSet(GpuObjectCreateFlag::DeferredInitialize))
		output->Initialize();

	return output;
}

SPtr<GpuQueryPool> VulkanGpuDevice::CreateQueryPool(const GpuQueryPoolCreateInformation& createInformation)
{
	return B3DMakeShared<VulkanGpuQueryPool>(GetResourceManager(), createInformation);
}

SPtr<SamplerState> VulkanGpuDevice::CreateSamplerState(const SamplerStateCreateInformation& createInformation, GpuObjectCreateFlags flags)
{
	SPtr<SamplerState> output = B3DMakeSharedFromExisting(new (B3DAllocate<VulkanSamplerState>()) VulkanSamplerState(*this, createInformation));

	if(!flags.IsSet(GpuObjectCreateFlag::DeferredInitialize))
		output->Initialize();

	return output;
}

SPtr<EventQuery> VulkanGpuDevice::CreateEventQuery()
{
	return B3DMakeSharedFromExisting(new (B3DAllocate<VulkanEventQuery>()) VulkanEventQuery(*this));
}

SPtr<GpuProgram> VulkanGpuDevice::CreateGpuProgram(const GpuProgramCreateInformation& createInformation, GpuObjectCreateFlags flags)
{
	SPtr<GpuProgram> output = B3DMakeSharedFromExisting(new(B3DAllocate<VulkanGpuProgram>()) VulkanGpuProgram(*this, createInformation));

	if(!flags.IsSet(GpuObjectCreateFlag::DeferredInitialize))
		output->Initialize();

	return output;
}

SPtr<GpuGraphicsPipelineState> VulkanGpuDevice::CreateGpuGraphicsPipelineState(const GpuGraphicsPipelineStateCreateInformation& createInformation, GpuObjectCreateFlags flags)
{
	SPtr<VulkanGpuGraphicsPipelineState> output = B3DMakeSharedFromExisting<VulkanGpuGraphicsPipelineState>(new(B3DAllocate<VulkanGpuGraphicsPipelineState>()) VulkanGpuGraphicsPipelineState(*this, createInformation));

	if(!flags.IsSet(GpuObjectCreateFlag::DeferredInitialize))
		output->Initialize();

	return output;
}

SPtr<GpuComputePipelineState> VulkanGpuDevice::CreateGpuComputePipelineState(const GpuComputePipelineStateCreateInformation& createInformation, GpuObjectCreateFlags flags)
{
	SPtr<VulkanGpuComputePipelineState> output = B3DMakeSharedFromExisting<VulkanGpuComputePipelineState>(new(B3DAllocate<VulkanGpuComputePipelineState>()) VulkanGpuComputePipelineState(*this, createInformation));

	if(!flags.IsSet(GpuObjectCreateFlag::DeferredInitialize))
		output->Initialize();

	return output;
}

SPtr<GpuPipelineParameterLayout> VulkanGpuDevice::CreateGpuPipelineParameterLayout(const GpuPipelineParameterLayoutCreateInformation& createInformation)
{
	return B3DMakeSharedFromExisting<VulkanGpuPipelineParameterLayout>(new(B3DAllocate<VulkanGpuPipelineParameterLayout>()) VulkanGpuPipelineParameterLayout(*this, createInformation));
}

SPtr<GpuPipelineParameterSetLayout> VulkanGpuDevice::CreateGpuPipelineParameterSetLayout(const GpuProgramParameterDescription& parameterDescription)
{
	return B3DMakeShared<VulkanGpuPipelineParameterSetLayout>(*this, parameterDescription);
}

UPtr<GpuParameterSetPool> VulkanGpuDevice::CreateParameterSetPool(const GpuParameterSetPoolCreateInformation& createInformation)
{
	return B3DMakeUnique<VulkanGpuParameterSetPool>(*this, createInformation);
}

SPtr<GpuTimelineFence> VulkanGpuDevice::CreateTimelineFence()
{
	return B3DMakeShared<VulkanGpuTimelineFence>(*this);
}

void VulkanGpuDevice::WaitUntilIdle()
{
	GetVulkanSubmitThread().WaitUntilIdle();
}

void VulkanGpuDevice::BeginFrame()
{
}

void VulkanGpuDevice::EndFrame()
{
	RunDefragPass();
	SubmitTransferCommandBuffers();

	// Advance transfer buffer helper pools to next frame
	mTransferBufferHelper->EndFrame();

	// No per-frame deferred-free drain is required: VulkanBuffer / VulkanImage destruction is gated
	// by VulkanResource::mUsedCount and FreeMemory returns each slot to its allocator synchronously.

	// Signal end-of-frame to submit thread. This blocks until the previous frame's resources are safe to reuse.
	GetVulkanSubmitThread().QueueEndFrameAndWaitForPreviousFrame();

	GpuDevice::EndFrame();
}

void VulkanGpuDevice::RunDefragPass()
{
	if (!mDefragEnabled)
		return;

	const SPtr<render::GpuCommandBuffer>& transferCb = mTransferBufferHelper->GetOrCreateTransferCommandBuffer();
	if (transferCb == nullptr)
		return;

	typename TGpuTlsfAllocator<VulkanHeapBackend>::DefragmentationInfo info;
	info.MaxBytesPerCall = mDefragBudgetBytes;
	info.MaxAllocationsPerCall = mDefragBudgetAllocations;

	Lock lock(mGpuMemoryAllocatorMutex);
	for (UPtr<TGpuTlsfAllocator<VulkanHeapBackend>>& allocator : mGpuMemoryAllocators)
	{
		if (allocator != nullptr)
			allocator->Defrag(*transferCb, info);
	}
}

void VulkanGpuDevice::SubmitTransferCommandBuffers(bool wait)
{
	mTransferBufferHelper->SubmitTransferCommandBuffer();

	if (wait)
		GetVulkanSubmitThread().WaitUntilIdle();
}

void VulkanGpuDevice::PresentRenderWindow(const SPtr<render::RenderWindow>& renderWindow, GpuQueueMask syncMask)
{
	SPtr<GpuQueue> queue = GetQueue(GQT_GRAPHICS, 0);
	if (!B3D_ENSURE(queue))
		return;

	queue->PresentRenderWindow(renderWindow, syncMask);
}

void VulkanGpuDevice::ConvertProjectionMatrix(const Matrix4& input, Matrix4& output)
{
	output = input;

	// Flip Y axis
	output[1][1] = -output[1][1];

	// Convert depth range from [-1,1] to [0,1]
	output[2][0] = (output[2][0] + output[3][0]) / 2;
	output[2][1] = (output[2][1] + output[3][1]) / 2;
	output[2][2] = (output[2][2] + output[3][2]) / 2;
	output[2][3] = (output[2][3] + output[3][3]) / 2;
}

GpuUniformBufferInformation VulkanGpuDevice::GenerateUniformBufferInformation(const String& name, TArray<GpuUniformBufferMemberInformation>& inOutUniforms)
{
	GpuUniformBufferInformation uniformBufferInformation;
	uniformBufferInformation.Size = 0;
	uniformBufferInformation.IsShareable = true;
	uniformBufferInformation.Name = name;
	uniformBufferInformation.Slot = 0;
	uniformBufferInformation.Set = 0;

	for(auto& member : inOutUniforms)
	{
		u32 size;
		if(member.Type == GPDT_STRUCT)
		{
			// Structs are always aligned and rounded up to vec4
			size = Math::DivideAndRoundUp(member.ElementSize, 16U) * 4;
			uniformBufferInformation.Size = Math::DivideAndRoundUp(uniformBufferInformation.Size, 4U) * 4;
		}
		else
			size = GpuBackendUtility::CalcStd140MemberSizeAndOffset(member.Type, member.ArraySize, uniformBufferInformation.Size);

		member.ElementSize = size;
		member.ArrayElementStride = size;
		member.CpuOffset = uniformBufferInformation.Size;
		member.GpuOffset = 0;
		uniformBufferInformation.Size += size * member.ArraySize;
		member.ParentUniformBufferSlot = 0;
		member.ParentUniformBufferSet = 0;
	}

	// Uniform buffer size must always be a multiple of 16
	if(uniformBufferInformation.Size % 4 != 0)
		uniformBufferInformation.Size += (4 - (uniformBufferInformation.Size % 4));

	return uniformBufferInformation;
}

float VulkanGpuDevice::ConvertTimestampToMilliseconds(u64 timestamp)
{
	const double timestampToMs = (double)GetDeviceProperties().limits.timestampPeriod / 1e6; // Nano to milli
	return (float)((double)timestamp * timestampToMs);
}

void VulkanGpuDevice::DoForEachQueue(const std::function<void(VulkanGpuQueue&)>&& callback) const
{
	for(u32 queueTypeIndex = 0; queueTypeIndex < GQT_COUNT; queueTypeIndex++)
	{
		GpuQueueType queueType = (GpuQueueType)queueTypeIndex;

		const u32 queueCount = GetQueueCount(queueType);
		for(u32 queueIndex = 0; queueIndex < queueCount; queueIndex++)
		{
			const SPtr<VulkanGpuQueue>& queue = std::static_pointer_cast<VulkanGpuQueue>(GetQueue(queueType, queueIndex));
			callback(*queue);
		}
	}
}

SurfaceFormat VulkanGpuDevice::GetSurfaceFormat(const VkSurfaceKHR& surface, bool useHardwareSRGB) const
{
	uint32_t numFormats;
	VkResult result = vkGetPhysicalDeviceSurfaceFormatsKHR(mPhysicalDevice, surface, &numFormats, nullptr);
	B3D_ASSERT(result == VK_SUCCESS);
	B3D_ASSERT(numFormats > 0);

	VkSurfaceFormatKHR* surfaceFormats = B3DStackAllocate<VkSurfaceFormatKHR>(numFormats);
	result = vkGetPhysicalDeviceSurfaceFormatsKHR(mPhysicalDevice, surface, &numFormats, surfaceFormats);
	B3D_ASSERT(result == VK_SUCCESS);

	SurfaceFormat output;
	output.ColorFormat = VK_FORMAT_R8G8B8A8_UNORM;
	output.ColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;

	PixelFormat depthFormat = VulkanUtility::GetClosestSupportedPixelFormat(*this, PF_D24S8, TEX_TYPE_2D, TextureUsageFlag::DepthStencil, true, false);

	output.DepthFormat = VulkanUtility::GetPixelFormat(depthFormat);

	// If there is no preferred format, use standard RGBA
	if((numFormats == 1) && (surfaceFormats[0].format == VK_FORMAT_UNDEFINED))
	{
		if(useHardwareSRGB)
			output.ColorFormat = VK_FORMAT_R8G8B8A8_SRGB;
		else
			output.ColorFormat = VK_FORMAT_B8G8R8A8_UNORM;

		output.ColorSpace = surfaceFormats[0].colorSpace;
	}
	else
	{
		bool foundFormat = false;

		VkFormat wantedFormatsUNORM[] = {
			VK_FORMAT_R8G8B8A8_UNORM,
			VK_FORMAT_B8G8R8A8_UNORM,
			VK_FORMAT_A8B8G8R8_UNORM_PACK32,
			VK_FORMAT_A8B8G8R8_UNORM_PACK32,
			VK_FORMAT_R8G8B8_UNORM,
			VK_FORMAT_B8G8R8_UNORM
		};

		VkFormat wantedFormatsSRGB[] = {
			VK_FORMAT_R8G8B8A8_SRGB,
			VK_FORMAT_B8G8R8A8_SRGB,
			VK_FORMAT_A8B8G8R8_SRGB_PACK32,
			VK_FORMAT_A8B8G8R8_SRGB_PACK32,
			VK_FORMAT_R8G8B8_SRGB,
			VK_FORMAT_B8G8R8_SRGB
		};

		u32 numWantedFormats;
		VkFormat* wantedFormats;
		if(useHardwareSRGB)
		{
			numWantedFormats = sizeof(wantedFormatsSRGB) / sizeof(wantedFormatsSRGB[0]);
			wantedFormats = wantedFormatsSRGB;
		}
		else
		{
			numWantedFormats = sizeof(wantedFormatsUNORM) / sizeof(wantedFormatsUNORM[0]);
			wantedFormats = wantedFormatsUNORM;
		}

		for(u32 i = 0; i < numWantedFormats; i++)
		{
			for(u32 j = 0; j < numFormats; j++)
			{
				if(surfaceFormats[j].format == wantedFormats[i])
				{
					output.ColorFormat = surfaceFormats[j].format;
					output.ColorSpace = surfaceFormats[j].colorSpace;

					foundFormat = true;
					break;
				}
			}

			if(foundFormat)
				break;
		}

		// If we haven't found anything, fall back to first available
		if(!foundFormat)
		{
			output.ColorFormat = surfaceFormats[0].format;
			output.ColorSpace = surfaceFormats[0].colorSpace;

			if(useHardwareSRGB)
			{
				B3D_LOG(Error, LogRenderBackend, "Cannot find a valid sRGB format for a render window surface, "
											 "falling back to a default format.");
			}
		}
	}

	B3DStackFree(surfaceFormats);
	return output;
}

VulkanBuffer* VulkanGpuDevice::CreateBuffer(const VulkanBufferCreateInformation& createInformation, VkMemoryPropertyFlags requiredFlags, VkMemoryPropertyFlags preferredFlags, VulkanGpuBuffer* parent)
{
	VkBuffer buffer;
	const VkResult createResult = vkCreateBuffer(mLogicalDevice, &createInformation.VkCreateInfo, gVulkanAllocator, &buffer);
	B3D_ASSERT(createResult == VK_SUCCESS);
	(void)createResult;

	const VulkanAllocationResult allocation = AllocateMemory(buffer, requiredFlags, preferredFlags);

	return CreateBuffer(createInformation, allocation, parent);
}

VulkanBuffer* VulkanGpuDevice::CreateBuffer(const VulkanBufferCreateInformation& createInformation, const VulkanAllocationResult& allocation, VulkanGpuBuffer* parent)
{
	B3D_ASSERT(allocation.IsValid());

	VkBuffer buffer;
	const VkResult createResult = vkCreateBuffer(mLogicalDevice, &createInformation.VkCreateInfo, gVulkanAllocator, &buffer);
	B3D_ASSERT(createResult == VK_SUCCESS);
	(void)createResult;

	return BindBufferToAllocation(createInformation, buffer, allocation, parent);
}

VulkanBuffer* VulkanGpuDevice::BindBufferToAllocation(const VulkanBufferCreateInformation& createInformation, VkBuffer buffer, VulkanAllocationResult allocation, VulkanGpuBuffer* parent)
{
	B3D_ASSERT(buffer != VK_NULL_HANDLE);
	B3D_ASSERT(allocation.IsValid());

	const VkResult bindResult = vkBindBufferMemory(mLogicalDevice, buffer, allocation.Location.Heap.Memory, allocation.Location.Offset);
	B3D_ASSERT(bindResult == VK_SUCCESS);
	(void)bindResult;

	VulkanBuffer* wrapper = mResourceManager->Create<VulkanBuffer>(createInformation, buffer, allocation, parent);

	if (parent != nullptr)
		SetAllocationOwner(allocation, wrapper);

	return wrapper;
}

VulkanImage* VulkanGpuDevice::CreateImage(const VulkanImageCreateInformation& createInformation, VkMemoryPropertyFlags requiredFlags, VkMemoryPropertyFlags preferredFlags, GpuResourceKind kind, VulkanTexture* parent)
{
	VkImage image;
	const VkResult createResult = vkCreateImage(mLogicalDevice, &createInformation.CreateInfo, gVulkanAllocator, &image);
	B3D_ASSERT(createResult == VK_SUCCESS);
	(void)createResult;

	const VulkanAllocationResult allocation = AllocateMemory(image, requiredFlags, preferredFlags, kind);

	return BindBufferToAllocation(createInformation, image, allocation, parent);
}

VulkanImage* VulkanGpuDevice::CreateImage(const VulkanImageCreateInformation& createInformation, const VulkanAllocationResult& allocation, VulkanTexture* parent)
{
	B3D_ASSERT(allocation.IsValid());

	VkImage image;
	const VkResult createResult = vkCreateImage(mLogicalDevice, &createInformation.CreateInfo, gVulkanAllocator, &image);
	B3D_ASSERT(createResult == VK_SUCCESS);
	(void)createResult;

	return BindBufferToAllocation(createInformation, image, allocation, parent);
}

VulkanImage* VulkanGpuDevice::BindBufferToAllocation(const VulkanImageCreateInformation& info, VkImage image, VulkanAllocationResult allocation, VulkanTexture* parent)
{
	B3D_ASSERT(image != VK_NULL_HANDLE);
	B3D_ASSERT(allocation.IsValid());

	const VkResult bindResult = vkBindImageMemory(mLogicalDevice, image, allocation.Location.Heap.Memory, allocation.Location.Offset);
	B3D_ASSERT(bindResult == VK_SUCCESS);
	(void)bindResult;

	VulkanImage* wrapper = mResourceManager->Create<VulkanImage>(info, image, allocation, parent);

	if (parent != nullptr)
		SetAllocationOwner(allocation, wrapper);

	return wrapper;
}

void VulkanGpuDevice::SetAllocationOwner(const VulkanAllocationResult& allocation, IGpuResource* owner)
{
	if (!allocation.IsValid())
		return;

	auto& allocator = *static_cast<TGpuTlsfAllocator<VulkanHeapBackend>*>(allocation.Location.Allocator);
	allocator.SetAllocationOwner(allocation.Location, owner);
}

VulkanAllocationResult VulkanGpuDevice::AllocateMemory(VkImage image, VkMemoryPropertyFlags requiredFlags, VkMemoryPropertyFlags preferredFlags, GpuResourceKind kind)
{
	VkMemoryRequirements requirements = {};
	vkGetImageMemoryRequirements(mLogicalDevice, image, &requirements);

	const u32 memoryTypeIndex = PickMemoryTypeIndex(requirements.memoryTypeBits, requiredFlags, preferredFlags);
	B3D_ASSERT(memoryTypeIndex != VK_MAX_MEMORY_TYPES && "No Vulkan memory type satisfies the requested image allocation flags.");

	TGpuTlsfAllocator<VulkanHeapBackend>& allocator = GetOrCreateGpuMemoryAllocator(memoryTypeIndex);

	VulkanAllocationResult output;
	const bool ok = allocator.TryAllocate(requirements.size, (u32)requirements.alignment, kind, output.Location);
	B3D_ASSERT(ok && "TLSF allocator failed to satisfy image allocation request.");
	(void)ok;

	if (output.Location.Heap.Mapped != nullptr)
		output.MappedMemory = static_cast<u8*>(output.Location.Heap.Mapped) + output.Location.Offset;

	return output;
}

VulkanAllocationResult VulkanGpuDevice::AllocateMemory(VkBuffer buffer, VkMemoryPropertyFlags requiredFlags, VkMemoryPropertyFlags preferredFlags)
{
	VkMemoryRequirements requirements = {};
	vkGetBufferMemoryRequirements(mLogicalDevice, buffer, &requirements);

	const u32 memoryTypeIndex = PickMemoryTypeIndex(requirements.memoryTypeBits, requiredFlags, preferredFlags);
	B3D_ASSERT(memoryTypeIndex != VK_MAX_MEMORY_TYPES && "No Vulkan memory type satisfies the requested buffer allocation flags.");

	TGpuTlsfAllocator<VulkanHeapBackend>& allocator = GetOrCreateGpuMemoryAllocator(memoryTypeIndex);

	VulkanAllocationResult output;
	const bool ok = allocator.TryAllocate(requirements.size, (u32)requirements.alignment, GpuResourceKind::Linear, output.Location);
	B3D_ASSERT(ok && "TLSF allocator failed to satisfy buffer allocation request.");
	(void)ok;

	if (output.Location.Heap.Mapped != nullptr)
		output.MappedMemory = static_cast<u8*>(output.Location.Heap.Mapped) + output.Location.Offset;

	return output;
}

void VulkanGpuDevice::FreeMemory(VulkanAllocationResult& allocation)
{
	if (!allocation.IsValid())
		return;

	const u32 memoryTypeIndex = allocation.Location.Heap.MemoryTypeIndex;
	TGpuTlsfAllocator<VulkanHeapBackend>& allocator = GetOrCreateGpuMemoryAllocator(memoryTypeIndex);
	allocator.FreeImmediate(allocation.Location);
	allocation.MappedMemory = nullptr;
}

u8* VulkanGpuDevice::MapMemory(const VulkanAllocationResult& allocation, VkDeviceSize offset) const
{
	B3D_ASSERT(allocation.MappedMemory != nullptr && "Allocation does not live in a host-visible / persistently-mapped heap.");
	return static_cast<u8*>(allocation.MappedMemory) + offset;
}

void VulkanGpuDevice::UnmapMemory(const VulkanAllocationResult& /*allocation*/) const
{
	// No-op: heaps backing host-visible memory types are persistently mapped at heap creation.
}

void VulkanGpuDevice::InvalidateMemory(const VulkanAllocationResult& allocation, VkDeviceSize offset, VkDeviceSize size) const
{
	if (!allocation.IsValid())
		return;

	const VkMappedMemoryRange range = BuildNonCoherentMappedMemoryRange(allocation, offset, size, mDeviceProperties.limits.nonCoherentAtomSize);

	const VkResult result = vkInvalidateMappedMemoryRanges(mLogicalDevice, 1, &range);
	B3D_ASSERT(result == VK_SUCCESS);
	(void)result;
}

void VulkanGpuDevice::FlushMemory(const VulkanAllocationResult& allocation, VkDeviceSize offset, VkDeviceSize size) const
{
	if (!allocation.IsValid())
		return;

	const VkMappedMemoryRange range = BuildNonCoherentMappedMemoryRange(allocation, offset, size, mDeviceProperties.limits.nonCoherentAtomSize);

	const VkResult result = vkFlushMappedMemoryRanges(mLogicalDevice, 1, &range);
	B3D_ASSERT(result == VK_SUCCESS);
	(void)result;
}

u32 VulkanGpuDevice::PickMemoryTypeIndex(u32 typeBits, VkMemoryPropertyFlags required, VkMemoryPropertyFlags preferred) const
{
	u32 bestIndex = VK_MAX_MEMORY_TYPES;
	u32 bestScore = 0;
	bool bestIsValid = false;

	for (u32 typeIndex = 0; typeIndex < mMemoryProperties.memoryTypeCount; typeIndex++)
	{
		if ((typeBits & (1u << typeIndex)) == 0)
			continue;

		const VkMemoryPropertyFlags flags = mMemoryProperties.memoryTypes[typeIndex].propertyFlags;
		if ((flags & required) != required)
			continue;

		// Score by number of preferred bits matched; ties resolved by lowest index.
		const u32 score = Bitwise::CountSetBits(flags & preferred);
		if (!bestIsValid || score > bestScore)
		{
			bestIsValid = true;
			bestScore = score;
			bestIndex = typeIndex;
		}
	}

	return bestIndex;
}

TGpuTlsfAllocator<VulkanHeapBackend>& VulkanGpuDevice::GetOrCreateGpuMemoryAllocator(u32 memoryTypeIndex)
{
	B3D_ASSERT(memoryTypeIndex < mMemoryProperties.memoryTypeCount);

	{
		Lock lock(mGpuMemoryAllocatorMutex);
		UPtr<TGpuTlsfAllocator<VulkanHeapBackend>>& slot = mGpuMemoryAllocators[memoryTypeIndex];
		if (slot != nullptr)
			return *slot;

		const VkMemoryPropertyFlags flags = mMemoryProperties.memoryTypes[memoryTypeIndex].propertyFlags;
		const bool isHostVisible = (flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) != 0;
		const bool isDeviceLocalOnly = (flags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) != 0 && !isHostVisible;

		TGpuTlsfAllocator<VulkanHeapBackend>::Configuration configuration;
		configuration.InitialHeapSize = isDeviceLocalOnly ? (64ull * 1024 * 1024) : (16ull * 1024 * 1024);
		configuration.MaxHeapSize = 256ull * 1024 * 1024;
		configuration.GrowthFactor = 2;
		configuration.MaxEmptyHeapCount = 1;
		configuration.MinAllocationSize = 16;
		configuration.BufferImageGranularity = mDeviceProperties.limits.bufferImageGranularity;
		configuration.DeferralMode = GpuAllocatorFreeDeferralMode::ResourceLifecycle;
		configuration.HeapCreateInfo.MemoryTypeBits = (1u << memoryTypeIndex);
		configuration.HeapCreateInfo.PropertyFlags = flags;
		configuration.HeapCreateInfo.MapPersistently = isHostVisible;

		slot = B3DMakeUnique<TGpuTlsfAllocator<VulkanHeapBackend>>(mHeapBackend.get(), this, configuration);
		return *slot;
	}
}

void VulkanGpuDevice::InitializeCapabilities()
{
	const VkPhysicalDeviceProperties& deviceProperties = GetDeviceProperties();
	const VkPhysicalDeviceFeatures& deviceFeatures =GetDeviceFeatures();
	const VkPhysicalDeviceLimits& deviceLimits = deviceProperties.limits;

	GpuDriverVersion driverVersion;
	driverVersion.Major = ((uint32_t)(deviceProperties.apiVersion) >> 22);
	driverVersion.Minor = ((uint32_t)(deviceProperties.apiVersion) >> 12) & 0x3ff;
	driverVersion.Release = (uint32_t)(deviceProperties.apiVersion) & 0xfff;
	driverVersion.Build = 0;

	mCapabilities.DriverVersion = driverVersion;
	mCapabilities.DeviceName = deviceProperties.deviceName;

	// Determine vendor
	switch(deviceProperties.vendorID)
	{
	case 0x10DE:
		mCapabilities.DeviceVendor = GPU_NVIDIA;
		break;
	case 0x1002:
		mCapabilities.DeviceVendor = GPU_AMD;
		break;
	case 0x163C:
	case 0x8086:
		mCapabilities.DeviceVendor = GPU_INTEL;
		break;
	case 0x106B:
		mCapabilities.DeviceVendor = GPU_APPLE;
		break;
	default:
		mCapabilities.DeviceVendor = GPU_UNKNOWN;
		break;
	};

	mCapabilities.BackendName = "Vulkan";

	if(deviceFeatures.textureCompressionBC)
		mCapabilities.SetCapability(RSC_TEXTURE_COMPRESSION_BC);

	if(deviceFeatures.textureCompressionETC2)
		mCapabilities.SetCapability(RSC_TEXTURE_COMPRESSION_ETC2);

	if(deviceFeatures.textureCompressionASTC_LDR)
		mCapabilities.SetCapability(RSC_TEXTURE_COMPRESSION_ASTC);

	mCapabilities.SetCapability(RSC_COMPUTE_PROGRAM);
	mCapabilities.SetCapability(RSC_LOAD_STORE);
	mCapabilities.SetCapability(RSC_LOAD_STORE_MSAA);
	mCapabilities.SetCapability(RSC_BYTECODE_CACHING);
	mCapabilities.SetCapability(RSC_TEXTURE_VIEWS);
	mCapabilities.SetCapability(RSC_RENDER_TARGET_LAYERS);
	mCapabilities.SetCapability(RSC_MULTI_THREADED_CB);

	mCapabilities.Conventions.NdcYAxis = GpuBackendConventions::Axis::Down;
	mCapabilities.Conventions.MatrixOrder = GpuBackendConventions::MatrixOrder::ColumnMajor;

	mCapabilities.VertexBufferCount = deviceLimits.maxVertexInputBindings;
	mCapabilities.RenderTargetCount = deviceLimits.maxColorAttachments;

	mCapabilities.SampledTexturesPerStage[GPT_FRAGMENT_PROGRAM] = deviceLimits.maxPerStageDescriptorSampledImages;
	mCapabilities.SampledTexturesPerStage[GPT_VERTEX_PROGRAM] = deviceLimits.maxPerStageDescriptorSampledImages;
	mCapabilities.SampledTexturesPerStage[GPT_COMPUTE_PROGRAM] = deviceLimits.maxPerStageDescriptorSampledImages;

	mCapabilities.UniformBufferCountPerStage[GPT_FRAGMENT_PROGRAM] = deviceLimits.maxPerStageDescriptorUniformBuffers;
	mCapabilities.UniformBufferCountPerStage[GPT_VERTEX_PROGRAM] = deviceLimits.maxPerStageDescriptorUniformBuffers;
	mCapabilities.UniformBufferCountPerStage[GPT_COMPUTE_PROGRAM] = deviceLimits.maxPerStageDescriptorUniformBuffers;

	mCapabilities.StorageTexturesPerStage[GPT_FRAGMENT_PROGRAM] = deviceLimits.maxPerStageDescriptorStorageImages;
	mCapabilities.StorageTexturesPerStage[GPT_COMPUTE_PROGRAM] = deviceLimits.maxPerStageDescriptorStorageImages;

	if(deviceFeatures.geometryShader)
	{
		mCapabilities.SetCapability(RSC_GEOMETRY_PROGRAM);
		mCapabilities.AddShaderProfile("gs_5_0");
		mCapabilities.SampledTexturesPerStage[GPT_GEOMETRY_PROGRAM] = deviceLimits.maxPerStageDescriptorSampledImages;
		mCapabilities.UniformBufferCountPerStage[GPT_GEOMETRY_PROGRAM] = deviceLimits.maxPerStageDescriptorUniformBuffers;
		mCapabilities.GeometryProgramNumOutputVertices = deviceLimits.maxGeometryOutputVertices;
	}

	if(deviceFeatures.tessellationShader)
	{
		mCapabilities.SetCapability(RSC_TESSELLATION_PROGRAM);

		mCapabilities.SampledTexturesPerStage[GPT_HULL_PROGRAM] = deviceLimits.maxPerStageDescriptorSampledImages;
		mCapabilities.SampledTexturesPerStage[GPT_DOMAIN_PROGRAM] = deviceLimits.maxPerStageDescriptorSampledImages;

		mCapabilities.UniformBufferCountPerStage[GPT_HULL_PROGRAM] = deviceLimits.maxPerStageDescriptorUniformBuffers;
		mCapabilities.UniformBufferCountPerStage[GPT_DOMAIN_PROGRAM] = deviceLimits.maxPerStageDescriptorUniformBuffers;
	}

	mCapabilities.TotalSampledTexturesCount = mCapabilities.SampledTexturesPerStage[GPT_FRAGMENT_PROGRAM] + mCapabilities.SampledTexturesPerStage[GPT_VERTEX_PROGRAM] + mCapabilities.SampledTexturesPerStage[GPT_GEOMETRY_PROGRAM] + mCapabilities.SampledTexturesPerStage[GPT_HULL_PROGRAM] + mCapabilities.SampledTexturesPerStage[GPT_DOMAIN_PROGRAM] + mCapabilities.SampledTexturesPerStage[GPT_COMPUTE_PROGRAM];

	mCapabilities.TotalUniformBuffersCount = mCapabilities.UniformBufferCountPerStage[GPT_FRAGMENT_PROGRAM] + mCapabilities.UniformBufferCountPerStage[GPT_VERTEX_PROGRAM] + mCapabilities.UniformBufferCountPerStage[GPT_GEOMETRY_PROGRAM] + mCapabilities.UniformBufferCountPerStage[GPT_HULL_PROGRAM] + mCapabilities.UniformBufferCountPerStage[GPT_DOMAIN_PROGRAM] + mCapabilities.UniformBufferCountPerStage[GPT_COMPUTE_PROGRAM];

	mCapabilities.TotalStorageTexturesCount = mCapabilities.StorageTexturesPerStage[GPT_FRAGMENT_PROGRAM] + mCapabilities.StorageTexturesPerStage[GPT_COMPUTE_PROGRAM];
	mCapabilities.MinimumUniformBufferOffsetAlignment = (u32)deviceLimits.minUniformBufferOffsetAlignment;
	mCapabilities.OptimalBufferToBufferCopyOffsetAlignment = (u32)deviceLimits.optimalBufferCopyOffsetAlignment;
	mCapabilities.OptimalBufferToImageCopyOffsetAlignment = (u32)deviceLimits.optimalBufferCopyRowPitchAlignment;

	mCapabilities.AddShaderProfile("glsl");
}

void VulkanGpuDevice::GetSyncSemaphores(GpuQueueMask syncMask, TInlineArray<VulkanSemaphore*, 8> outSemaphores) const
{
	AssertIfNotVulkanSubmitThread();

	bool semaphoreRequestFailed = false;

	for(u32 queueTypeIndex = 0; queueTypeIndex < GQT_COUNT; queueTypeIndex++)
	{
		const GpuQueueType queueType = (GpuQueueType)queueTypeIndex;

		const u32 queueCount = GetQueueCount(queueType);
		for(u32 queueIndex = 0; queueIndex < queueCount; queueIndex++)
		{
			VulkanGpuQueue* queue = static_cast<VulkanGpuQueue*>(GetQueue(queueType, queueIndex).get());
			SPtr<VulkanGpuCommandBuffer> lastCommandBuffer = queue->GetLastCommandBuffer();

			// Check if a buffer is currently executing on the queue
			if(lastCommandBuffer == nullptr || (!lastCommandBuffer->IsSubmitted() && !lastCommandBuffer->IsDone()))
				continue;

			// Check if we care about this specific queue
			const GpuQueueId queueId = GpuQueueId(queueType, queueIndex);
			if(!syncMask.IsSet(queueId))
				continue;

			VulkanSemaphore* semaphore = lastCommandBuffer->RequestInterQueueSemaphore();
			if(semaphore == nullptr)
			{
				semaphoreRequestFailed = true;
				continue;
			}

			outSemaphores.Add(semaphore);
		}
	}

	if(semaphoreRequestFailed)
	{
		B3D_LOG(Error, LogRenderBackend, "Failed to allocate semaphores for a command buffer sync. This means some of the "
									 "dependency requests will not be fulfilled. This happened because a command buffer has too many "
									 "dependant command buffers. The maximum allowed number is {0} but can be increased by incrementing the "
									 "value of B3D_MAX_COMMAND_BUFFER_DEPENDENCIES.",
			   B3D_MAX_COMMAND_BUFFER_DEPENDENCIES);
	}
}
