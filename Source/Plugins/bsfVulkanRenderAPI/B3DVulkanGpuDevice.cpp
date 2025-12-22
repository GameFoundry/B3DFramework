//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DVulkanGpuDevice.h"
#include "B3DVulkanGpuQueue.h"
#include "B3DVulkanGpuCommandBuffer.h"
#include "B3DVulkanUtility.h"
#include "B3DVulkanGpuBackend.h"
#include "B3DVulkanSubmitThread.h"
#include "Managers/B3DVulkanDescriptorManager.h"
#include "Managers/B3DVulkanQueries.h"

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

#define VMA_IMPLEMENTATION
#include "B3DVulkanEventQuery.h"
#include "B3DVulkanGLSLToSPIRV.h"
#include "B3DVulkanGpuBuffer.h"
#include "B3DVulkanGpuParameterSet.h"
#include "B3DVulkanGpuPipelineParameterLayout.h"
#include "B3DVulkanGpuProgram.h"
#include "B3DVulkanSamplerState.h"
#include "B3DVulkanTexture.h"
#include "RenderAPI/B3DGpuProgramParameterDescription.h"
#include "ThirdParty/vk_mem_alloc.h"
#include "Utility/B3DBitwise.h"

using namespace b3d;
using namespace b3d::render;

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

	// Set up extensions
	const char* extensions[6];
	uint32_t extensionCount = 0;

	extensions[extensionCount++] = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
	extensions[extensionCount++] = VK_KHR_MAINTENANCE1_EXTENSION_NAME;
	extensions[extensionCount++] = VK_KHR_MAINTENANCE2_EXTENSION_NAME;

	// Enumerate supported extensions
	bool dedicatedAllocExt = false;
	bool getMemReqExt = false;

	uint32_t availableExtensionCount = 0;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &availableExtensionCount, nullptr);
	if(availableExtensionCount > 0)
	{
		Vector<VkExtensionProperties> availableExtensions(availableExtensionCount);
		if(vkEnumerateDeviceExtensionProperties(device, nullptr, &availableExtensionCount, availableExtensions.data()) == VK_SUCCESS)
		{
			for(auto entry : availableExtensions)
			{
				if(strcmp(entry.extensionName, VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME) == 0)
				{
					extensions[extensionCount++] = VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME;
					dedicatedAllocExt = true;
				}
				else if(strcmp(entry.extensionName, VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME) == 0)
				{
					extensions[extensionCount++] = VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME;
					getMemReqExt = true;
				}
				else if(strcmp(entry.extensionName, VK_EXT_SHADER_VIEWPORT_INDEX_LAYER_EXTENSION_NAME) == 0)
				{
					extensions[extensionCount++] = VK_EXT_SHADER_VIEWPORT_INDEX_LAYER_EXTENSION_NAME;
				}
			}
		}
	}

	VkDeviceCreateInfo deviceInfo;
	deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceInfo.pNext = nullptr;
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

	// Set up the memory allocator
	VmaAllocatorCreateInfo allocatorCI = {};
	allocatorCI.physicalDevice = device;
	allocatorCI.device = mLogicalDevice;
	allocatorCI.instance = GetVulkanGpuBackend().GetVkInstance();
	allocatorCI.pAllocationCallbacks = gVulkanAllocator;

	if(dedicatedAllocExt && getMemReqExt)
		allocatorCI.flags |= VMA_ALLOCATOR_CREATE_KHR_DEDICATED_ALLOCATION_BIT;

	vmaCreateAllocator(&allocatorCI, &mAllocator);

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
}

VulkanGpuDevice::~VulkanGpuDevice()
{
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

	vmaDestroyAllocator(mAllocator);
	vkDestroyDevice(mLogicalDevice, gVulkanAllocator);
}

SPtr<GpuProgramBytecode> VulkanGpuDevice::CompileGpuProgramBytecode(const GpuProgramCreateInformation& createInformation) const
{
	if(!IsGpuProgramLanguageSupported(createInformation.Language))
		return nullptr;

	SPtr<GpuProgramBytecode> spirv = GLSLToSPIRV::Instance().Convert(createInformation);

#if B3D_PLATFORM_MACOS
	// We'll just re-purpose the existing data structure
	SPtr<GpuProgramBytecode> msl = spirv;
	msl->compilerId = MOLTENVK_COMPILER_ID;
	msl->compilerVersion = MOLTENVK_COMPILER_VERSION;

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

SPtr<GpuQueue> VulkanGpuDevice::GetQueue(GpuQueueType usage, u32 index) const
{
	if (index >= GetQueueCount(usage))
		return nullptr;

	return mQueueInfos[(u32)usage].Queues[index];
}

SPtr<GpuCommandBufferPool> VulkanGpuDevice::CreateGpuCommandBufferPool(const render::GpuCommandBufferPoolCreateInformation& createInformation)
{
	return B3DMakeSharedFromExisting(new(B3DAllocate<VulkanGpuCommandBufferPool>()) VulkanGpuCommandBufferPool(*this, createInformation));
}

SPtr<render::Texture> VulkanGpuDevice::CreateTexture(const TextureCreateInformation& createInformation, bool deferredInitialize)
{
	SPtr<Texture> output = B3DMakeSharedFromExisting(new(B3DAllocate<VulkanTexture>()) VulkanTexture(*this, createInformation));
	output->SetShared(output);

	if(!deferredInitialize)
		output->Initialize();

	return output;
}

SPtr<render::GpuBuffer> VulkanGpuDevice::CreateGpuBuffer(const GpuBufferCreateInformation& createInformation, bool deferredInitialize)
{
	SPtr<GpuBuffer> output = B3DMakeSharedFromExisting(new(B3DAllocate<VulkanGpuBuffer>()) VulkanGpuBuffer(*this, createInformation));
	output->SetShared(output);

	if(!deferredInitialize)
		output->Initialize();

	return output;
}

SPtr<GpuQueryPool> VulkanGpuDevice::CreateQueryPool(const GpuQueryPoolCreateInformation& createInformation)
{
	return B3DMakeShared<VulkanGpuQueryPool>(GetResourceManager(), createInformation);
}

SPtr<SamplerState> VulkanGpuDevice::CreateSamplerState(const SamplerStateCreateInformation& createInformation, bool deferredInitialize)
{
	SPtr<SamplerState> output = B3DMakeSharedFromExisting(new (B3DAllocate<VulkanSamplerState>()) VulkanSamplerState(*this, createInformation));

	if(!deferredInitialize)
		output->Initialize();

	return output;
}

SPtr<EventQuery> VulkanGpuDevice::CreateEventQuery()
{
	return B3DMakeSharedFromExisting(new (B3DAllocate<VulkanEventQuery>()) VulkanEventQuery(*this));
}

SPtr<GpuProgram> VulkanGpuDevice::CreateGpuProgram(const GpuProgramCreateInformation& createInformation, bool deferredInitialize)
{
	SPtr<GpuProgram> output = B3DMakeSharedFromExisting(new(B3DAllocate<VulkanGpuProgram>()) VulkanGpuProgram(*this, createInformation));

	if(!deferredInitialize)
		output->Initialize();

	return output;
}

SPtr<render::GpuParameterSet> VulkanGpuDevice::CreateGpuParameterSet(const SPtr<GpuPipelineParameterSetLayout>& parameterSetLayout, u32 setIndex, bool deferredInitialize)
{
	SPtr<GpuParameterSet> output = B3DMakeSharedFromExisting(new(B3DAllocate<VulkanGpuParameterSet>()) VulkanGpuParameterSet(*this, parameterSetLayout, setIndex));
	output->SetShared(output);

	if(!deferredInitialize)
		output->Initialize();

	return output;
}

SPtr<GpuGraphicsPipelineState> VulkanGpuDevice::CreateGpuGraphicsPipelineState(const GpuGraphicsPipelineStateCreateInformation& createInformation, bool deferredInitialize)
{
	SPtr<VulkanGpuGraphicsPipelineState> output = B3DMakeSharedFromExisting<VulkanGpuGraphicsPipelineState>(new(B3DAllocate<VulkanGpuGraphicsPipelineState>()) VulkanGpuGraphicsPipelineState(*this, createInformation));

	if(!deferredInitialize)
		output->Initialize();

	return output;
}

SPtr<GpuComputePipelineState> VulkanGpuDevice::CreateGpuComputePipelineState(const GpuComputePipelineStateCreateInformation& createInformation, bool deferredInitialize)
{
	SPtr<VulkanGpuComputePipelineState> output = B3DMakeSharedFromExisting<VulkanGpuComputePipelineState>(new(B3DAllocate<VulkanGpuComputePipelineState>()) VulkanGpuComputePipelineState(*this, createInformation));

	if(!deferredInitialize)
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

void VulkanGpuDevice::WaitUntilIdle()
{
	GetVulkanSubmitThread().WaitUntilIdle();
}

void VulkanGpuDevice::BeginFrame()
{
}

void VulkanGpuDevice::EndFrame()
{
	SubmitTransferCommandBuffers();

	DoForEachQueue([](VulkanGpuQueue& queue)
	{
		queue.EndFrame();
	});

	GetVulkanSubmitThread().QueueRefreshCommandBufferCompletionStates(this);
}

void VulkanGpuDevice::SubmitTransferCommandBuffers(bool wait)
{
	DoForEachQueue([](VulkanGpuQueue& queue)
	{
		queue.SubmitTransferCommandBuffer(false);
	});
	
	if (wait)
	{
		GetVulkanSubmitThread().WaitUntilIdle();
	}
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
			size = VulkanUtility::CalcInterfaceBlockElementSizeAndOffset(member.Type, member.ArraySize, uniformBufferInformation.Size);

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

GpuQueueMask VulkanGpuDevice::GetQueueMask(GpuQueueType type, u32 queueIdx) const
{
	const u32 queueCount = GetQueueCount(type);
	if(queueCount == 0)
		return 0;

	GpuQueueMask mask;
	u32 localIndex = queueIdx % queueCount;
	while(localIndex < B3D_MAX_QUEUES_PER_TYPE)
	{
		mask |= GpuQueueMask(GpuQueueId(type, localIndex));
		localIndex += queueCount;
	}

	return mask;
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
				B3D_LOG(Error, RenderBackend, "Cannot find a valid sRGB format for a render window surface, "
											 "falling back to a default format.");
			}
		}
	}

	B3DStackFree(surfaceFormats);
	return output;
}

VulkanAllocationResult VulkanGpuDevice::AllocateMemory(VkImage image, VmaMemoryUsage usage)
{
	VmaAllocationCreateInfo allocationCreateInformation = {};
	allocationCreateInformation.usage = usage;
	allocationCreateInformation.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

	VmaAllocationInfo allocationInfo;
	VmaAllocation allocation;
	VkResult result = vmaAllocateMemoryForImage(mAllocator, image, &allocationCreateInformation, &allocation, &allocationInfo);
	B3D_ASSERT(result == VK_SUCCESS);

	result = vkBindImageMemory(mLogicalDevice, image, allocationInfo.deviceMemory, allocationInfo.offset);
	B3D_ASSERT(result == VK_SUCCESS);

	VulkanAllocationResult output;
	output.Handle = allocation;
	output.MappedMemory = allocationInfo.pMappedData;

	return output;
}

VulkanAllocationResult VulkanGpuDevice::AllocateMemory(VkBuffer buffer, VmaMemoryUsage usage)
{
	VmaAllocationCreateInfo allocationCreateInformation = {};
	allocationCreateInformation.usage = usage;
	allocationCreateInformation.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

	VmaAllocationInfo allocationInfo;
	VmaAllocation allocation;
	VkResult result = vmaAllocateMemoryForBuffer(mAllocator, buffer, &allocationCreateInformation, &allocation, &allocationInfo);
	B3D_ASSERT(result == VK_SUCCESS);

	result = vkBindBufferMemory(mLogicalDevice, buffer, allocationInfo.deviceMemory, allocationInfo.offset);
	B3D_ASSERT(result == VK_SUCCESS);

	VulkanAllocationResult output;
	output.Handle = allocation;
	output.MappedMemory = allocationInfo.pMappedData;

	return output;
}

void VulkanGpuDevice::FreeMemory(VmaAllocation allocation)
{
	vmaFreeMemory(mAllocator, allocation);
}

void* VulkanGpuDevice::MapMemory(const VmaAllocation& allocation) const
{
	void* data;
	VkResult result = vmaMapMemory(mAllocator, allocation, &data);
	B3D_ASSERT(result == VK_SUCCESS);

	return data;
}

void VulkanGpuDevice::UnmapMemory(const VmaAllocation& allocation) const
{
	vmaUnmapMemory(mAllocator, allocation);
}

void VulkanGpuDevice::InvalidateMemory(const VmaAllocation& allocation, VkDeviceSize offset, VkDeviceSize size) const
{
	VkResult result = vmaInvalidateAllocation(mAllocator, allocation, offset, size);
	B3D_ASSERT(result == VK_SUCCESS);
}

void VulkanGpuDevice::FlushMemory(const VmaAllocation& allocation, VkDeviceSize offset, VkDeviceSize size) const
{
	VkResult result = vmaFlushAllocation(mAllocator, allocation, offset, size);
	B3D_ASSERT(result == VK_SUCCESS);
}

void VulkanGpuDevice::GetAllocationInfo(VmaAllocation allocation, VkDeviceMemory& memory, VkDeviceSize& offset)
{
	VmaAllocationInfo allocInfo;
	vmaGetAllocationInfo(mAllocator, allocation, &allocInfo);

	memory = allocInfo.deviceMemory;
	offset = allocInfo.offset;
}

uint32_t VulkanGpuDevice::FindMemoryType(uint32_t requirementBits, VkMemoryPropertyFlags wantedFlags)
{
	for(uint32_t i = 0; i < mMemoryProperties.memoryTypeCount; i++)
	{
		if(requirementBits & (1 << i))
		{
			if((mMemoryProperties.memoryTypes[i].propertyFlags & wantedFlags) == wantedFlags)
				return i;
		}
	}

	return -1;
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

	mCapabilities.MaxBoundVertexBuffers = deviceLimits.maxVertexInputBindings;
	mCapabilities.NumMultiRenderTargets = deviceLimits.maxColorAttachments;

	mCapabilities.NumTextureUnitsPerStage[GPT_FRAGMENT_PROGRAM] = deviceLimits.maxPerStageDescriptorSampledImages;
	mCapabilities.NumTextureUnitsPerStage[GPT_VERTEX_PROGRAM] = deviceLimits.maxPerStageDescriptorSampledImages;
	mCapabilities.NumTextureUnitsPerStage[GPT_COMPUTE_PROGRAM] = deviceLimits.maxPerStageDescriptorSampledImages;

	mCapabilities.NumGpuParamBlockBuffersPerStage[GPT_FRAGMENT_PROGRAM] = deviceLimits.maxPerStageDescriptorUniformBuffers;
	mCapabilities.NumGpuParamBlockBuffersPerStage[GPT_VERTEX_PROGRAM] = deviceLimits.maxPerStageDescriptorUniformBuffers;
	mCapabilities.NumGpuParamBlockBuffersPerStage[GPT_COMPUTE_PROGRAM] = deviceLimits.maxPerStageDescriptorUniformBuffers;

	mCapabilities.NumLoadStoreTextureUnitsPerStage[GPT_FRAGMENT_PROGRAM] = deviceLimits.maxPerStageDescriptorStorageImages;
	mCapabilities.NumLoadStoreTextureUnitsPerStage[GPT_COMPUTE_PROGRAM] = deviceLimits.maxPerStageDescriptorStorageImages;

	if(deviceFeatures.geometryShader)
	{
		mCapabilities.SetCapability(RSC_GEOMETRY_PROGRAM);
		mCapabilities.AddShaderProfile("gs_5_0");
		mCapabilities.NumTextureUnitsPerStage[GPT_GEOMETRY_PROGRAM] = deviceLimits.maxPerStageDescriptorSampledImages;
		mCapabilities.NumGpuParamBlockBuffersPerStage[GPT_GEOMETRY_PROGRAM] = deviceLimits.maxPerStageDescriptorUniformBuffers;
		mCapabilities.GeometryProgramNumOutputVertices = deviceLimits.maxGeometryOutputVertices;
	}

	if(deviceFeatures.tessellationShader)
	{
		mCapabilities.SetCapability(RSC_TESSELLATION_PROGRAM);

		mCapabilities.NumTextureUnitsPerStage[GPT_HULL_PROGRAM] = deviceLimits.maxPerStageDescriptorSampledImages;
		mCapabilities.NumTextureUnitsPerStage[GPT_DOMAIN_PROGRAM] = deviceLimits.maxPerStageDescriptorSampledImages;

		mCapabilities.NumGpuParamBlockBuffersPerStage[GPT_HULL_PROGRAM] = deviceLimits.maxPerStageDescriptorUniformBuffers;
		mCapabilities.NumGpuParamBlockBuffersPerStage[GPT_DOMAIN_PROGRAM] = deviceLimits.maxPerStageDescriptorUniformBuffers;
	}

	mCapabilities.NumCombinedTextureUnits = mCapabilities.NumTextureUnitsPerStage[GPT_FRAGMENT_PROGRAM] + mCapabilities.NumTextureUnitsPerStage[GPT_VERTEX_PROGRAM] + mCapabilities.NumTextureUnitsPerStage[GPT_GEOMETRY_PROGRAM] + mCapabilities.NumTextureUnitsPerStage[GPT_HULL_PROGRAM] + mCapabilities.NumTextureUnitsPerStage[GPT_DOMAIN_PROGRAM] + mCapabilities.NumTextureUnitsPerStage[GPT_COMPUTE_PROGRAM];

	mCapabilities.NumCombinedParamBlockBuffers = mCapabilities.NumGpuParamBlockBuffersPerStage[GPT_FRAGMENT_PROGRAM] + mCapabilities.NumGpuParamBlockBuffersPerStage[GPT_VERTEX_PROGRAM] + mCapabilities.NumGpuParamBlockBuffersPerStage[GPT_GEOMETRY_PROGRAM] + mCapabilities.NumGpuParamBlockBuffersPerStage[GPT_HULL_PROGRAM] + mCapabilities.NumGpuParamBlockBuffersPerStage[GPT_DOMAIN_PROGRAM] + mCapabilities.NumGpuParamBlockBuffersPerStage[GPT_COMPUTE_PROGRAM];

	mCapabilities.NumCombinedLoadStoreTextureUnits = mCapabilities.NumLoadStoreTextureUnitsPerStage[GPT_FRAGMENT_PROGRAM] + mCapabilities.NumLoadStoreTextureUnitsPerStage[GPT_COMPUTE_PROGRAM];
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
		B3D_LOG(Error, RenderBackend, "Failed to allocate semaphores for a command buffer sync. This means some of the "
									 "dependency requests will not be fulfilled. This happened because a command buffer has too many "
									 "dependant command buffers. The maximum allowed number is {0} but can be increased by incrementing the "
									 "value of B3D_MAX_COMMAND_BUFFER_DEPENDENCIES.",
			   B3D_MAX_COMMAND_BUFFER_DEPENDENCIES);
	}
}
