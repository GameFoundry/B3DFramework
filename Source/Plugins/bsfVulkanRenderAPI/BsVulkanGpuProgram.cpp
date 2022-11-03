//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsVulkanGpuProgram.h"
#include "BsVulkanRenderAPI.h"
#include "BsVulkanDevice.h"
#include "BsVulkanUtility.h"
#include "RenderAPI/BsGpuParams.h"
#include "RenderAPI/BsGpuParamDesc.h"
#include "Managers/BsGpuProgramManager.h"
#include "RenderAPI/BsVertexDeclaration.h"
#include "Managers/BsHardwareBufferManager.h"
#include "Profiling/BsRenderStats.h"
#include "FileSystem/BsFileSystem.h"
#include "FileSystem/BsDataStream.h"

#if B3D_PLATFORM == B3D_PLATFORM_ID_MACOS
#	include <MoltenVK/vk_mvk_moltenvk.h>
#endif

using namespace bs;
using namespace bs::ct;

VulkanShaderModule::VulkanShaderModule(VulkanResourceManager* owner, VkShaderModule module)
	: VulkanResource(owner, true), mModule(module)
{}

VulkanShaderModule::~VulkanShaderModule()
{
	vkDestroyShaderModule(mOwner->GetDevice().GetLogical(), mModule, gVulkanAllocator);
}

VulkanGpuProgram::VulkanGpuProgram(const GPU_PROGRAM_DESC& desc, GpuDeviceFlags deviceMask)
	: GpuProgram(desc, deviceMask), mDeviceMask(deviceMask), mModules()
{
}

VulkanGpuProgram::~VulkanGpuProgram()
{
	for(u32 i = 0; i < BS_MAX_DEVICES; i++)
	{
		if(mModules[i] != nullptr)
			mModules[i]->Destroy();
	}

	BS_INC_RENDER_STAT_CAT(ResDestroyed, RenderStatObject_GpuProgram);
}

void VulkanGpuProgram::Initialize()
{
	if(!IsSupported())
	{
		mIsCompiled = false;
		mCompileMessages = "Specified program is not supported by the current render system.";

		GpuProgram::Initialize();
		return;
	}

	if(!mBytecode ||
#if B3D_PLATFORM == B3D_PLATFORM_ID_MACOS
	   mBytecode->CompilerId != MOLTENVK_COMPILER_ID || mBytecode->CompilerVersion != MOLTENVK_COMPILER_VERSION)
#else
	   mBytecode->CompilerId != VULKAN_COMPILER_ID || mBytecode->CompilerVersion != VULKAN_COMPILER_VERSION)
#endif
	{
		GPU_PROGRAM_DESC desc;
		desc.Type = mType;
		desc.EntryPoint = mEntryPoint;
#if B3D_PLATFORM == B3D_PLATFORM_ID_MACOS
		desc.language = "mvksl";
#else
		desc.Language = "vksl";
#endif
		desc.Source = mSource;

		mBytecode = CompileBytecode(desc);
	}

	mCompileMessages = mBytecode->Messages;
	mIsCompiled = mBytecode->Instructions.Data != nullptr;

	if(mIsCompiled)
	{
		VulkanRenderAPI& rapi = static_cast<VulkanRenderAPI&>(RenderAPI::Instance());
		VulkanDevice* devices[BS_MAX_DEVICES];

		u32 codeSize = mBytecode->Instructions.Size;
		u8* code = mBytecode->Instructions.Data;

#if B3D_PLATFORM == B3D_PLATFORM_ID_MACOS
		u32 workgroupSize[3] = { 1, 1, 1 };
		if(mType == GPT_COMPUTE_PROGRAM)
		{
			B3D_ASSERT(codeSize > sizeof(workgroupSize));

			memcpy(workgroupSize, code, sizeof(workgroupSize));
			code += sizeof(workgroupSize);
			codeSize -= sizeof(workgroupSize);
		}
#endif

		// Create Vulkan module
		VkShaderModuleCreateInfo moduleCI;
		moduleCI.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		moduleCI.pNext = nullptr;
		moduleCI.flags = 0;
		moduleCI.codeSize = codeSize;
		moduleCI.pCode = (uint32_t*)code;

		VulkanUtility::GetDevices(rapi, mDeviceMask, devices);

		for(u32 i = 0; i < BS_MAX_DEVICES; i++)
		{
			if(devices[i] != nullptr)
			{
				VkDevice vkDevice = devices[i]->GetLogical();
				VulkanResourceManager& rescManager = devices[i]->GetResourceManager();

				VkShaderModule shaderModule;
				VkResult result = vkCreateShaderModule(vkDevice, &moduleCI, gVulkanAllocator, &shaderModule);
				B3D_ASSERT(result == VK_SUCCESS);

#if B3D_PLATFORM == B3D_PLATFORM_ID_MACOS
				if(mType == GPT_COMPUTE_PROGRAM)
					vkSetWorkgroupSizeMVK(shaderModule, workgroupSize[0], workgroupSize[1], workgroupSize[2]);
#endif
				mModules[i] = rescManager.Create<VulkanShaderModule>(shaderModule);
			}
		}

		mParametersDesc = mBytecode->ParamDesc;

		if(mType == GPT_VERTEX_PROGRAM)
		{
			mInputDeclaration = HardwareBufferManager::Instance().CreateVertexDeclaration(
				mBytecode->VertexInput, mDeviceMask);
		}
	}

	BS_INC_RENDER_STAT_CAT(ResCreated, RenderStatObject_GpuProgram);

	GpuProgram::Initialize();
}
