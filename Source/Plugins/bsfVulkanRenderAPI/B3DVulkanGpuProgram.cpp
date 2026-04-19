//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DVulkanGpuProgram.h"
#include "B3DVulkanGpuDevice.h"
#include "B3DVulkanUtility.h"
#include "RenderAPI/B3DGpuParameterSet.h"
#include "RenderAPI/B3DGpuProgramParameterDescription.h"
#include "RenderAPI/B3DVertexDescription.h"
#include "Profiling/B3DRenderStats.h"
#include "FileSystem/B3DFileSystem.h"
#include "FileSystem/B3DDataStream.h"
#include "B3DVulkanGpuBackend.h"

#if B3D_PLATFORM_MACOS
#	include <MoltenVK/vk_mvk_moltenvk.h>
#endif

using namespace b3d;
using namespace b3d::render;

VulkanShaderModule::VulkanShaderModule(VulkanResourceManager* owner, VkShaderModule module, const StringView& name)
	: VulkanResource(owner, true, name), mModule(module)
{}

VulkanShaderModule::~VulkanShaderModule()
{
	vkDestroyShaderModule(mOwner->GetDevice().GetLogical(), mModule, gVulkanAllocator);
}

void VulkanShaderModule::SetName(const StringView& name)
{
	if(vkSetDebugUtilsObjectNameEXT == nullptr)
		return;

	VkDebugUtilsObjectNameInfoEXT objectNameInfo;
	objectNameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
	objectNameInfo.pNext = nullptr;
	objectNameInfo.objectType = VK_OBJECT_TYPE_SHADER_MODULE;
	objectNameInfo.objectHandle = (uint64_t)mModule;
	objectNameInfo.pObjectName = name.data();

	vkSetDebugUtilsObjectNameEXT(mOwner->GetDevice().GetLogical(), &objectNameInfo);
}

VulkanGpuProgram::VulkanGpuProgram(VulkanGpuDevice& gpuDevice, const GpuProgramCreateInformation& createInformation)
	: GpuProgram(createInformation), mGpuDevice(gpuDevice)
{
}

VulkanGpuProgram::~VulkanGpuProgram()
{
	if(mModule != nullptr)
		mModule->Destroy();

	B3D_INCREMENT_RENDER_STATISTIC_CATEGORY(ResDestroyed, RenderStatObject_GpuProgram);
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
#if B3D_PLATFORM_MACOS
	   mBytecode->CompilerId != MOLTENVK_COMPILER_ID || mBytecode->CompilerVersion != MOLTENVK_COMPILER_VERSION)
#else
	   mBytecode->CompilerId != VULKAN_COMPILER_ID || mBytecode->CompilerVersion != VULKAN_COMPILER_VERSION)
#endif
	{
		GpuProgramCreateInformation createInformation;
		createInformation.Name = mName;
		createInformation.Type = mType;
		createInformation.EntryPoint = mEntryPoint;
#if B3D_PLATFORM_MACOS
		desc.language = kGpuProgramLanguageMvksl;
#else
		createInformation.Language = kGpuProgramLanguageVksl;
#endif
		createInformation.Source = mSource;

		mBytecode = mGpuDevice.CompileGpuProgramBytecode(createInformation);
	}

	mCompileMessages = mBytecode->Messages;
	mIsCompiled = mBytecode->Instructions.Data != nullptr;

	if(mIsCompiled)
	{
		u32 codeSize = mBytecode->Instructions.Size;
		u8* code = mBytecode->Instructions.Data;

#if B3D_PLATFORM_MACOS
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

		VkDevice vkDevice = mGpuDevice.GetLogical();
		VulkanResourceManager& rescManager = mGpuDevice.GetResourceManager();

		VkShaderModule shaderModule;
		VkResult result = vkCreateShaderModule(vkDevice, &moduleCI, gVulkanAllocator, &shaderModule);
		B3D_ASSERT(result == VK_SUCCESS);

#if B3D_PLATFORM_MACOS
		if(mType == GPT_COMPUTE_PROGRAM)
			vkSetWorkgroupSizeMVK(shaderModule, workgroupSize[0], workgroupSize[1], workgroupSize[2]);
#endif
		mModule = rescManager.Create<VulkanShaderModule>(shaderModule);
		mModule->SetName(mName);

		mParametersDescription = mBytecode->ParameterDescription;

		if(mType == GPT_VERTEX_PROGRAM)
		{
			mVertexInputDescription = B3DMakeShared<VertexDescription>(mBytecode->VertexInput, false);
		}
	}

	B3D_INCREMENT_RENDER_STATISTIC_CATEGORY(ResCreated, RenderStatObject_GpuProgram);

	GpuProgram::Initialize();
}
