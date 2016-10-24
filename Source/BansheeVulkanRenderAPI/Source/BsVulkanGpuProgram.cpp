//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsVulkanGpuProgram.h"
#include "BsGpuParams.h"
#include "BsVulkanRenderAPI.h"
#include "BsGpuProgramManager.h"
#include "BsRenderStats.h"

namespace BansheeEngine
{
	VulkanGpuProgramCore::VulkanGpuProgramCore(const GPU_PROGRAM_DESC& desc, GpuDeviceFlags deviceMask)
		: GpuProgramCore(desc, deviceMask)
	{

	}

	VulkanGpuProgramCore::~VulkanGpuProgramCore()
	{
		BS_INC_RENDER_STAT_CAT(ResDestroyed, RenderStatObject_GpuProgram);
	}

	void VulkanGpuProgramCore::initialize()
	{
		if (!isSupported())
		{
			mIsCompiled = false;
			mCompileError = "Specified program is not supported by the current render system.";

			GpuProgramCore::initialize();
			return;
		}
		
		BS_INC_RENDER_STAT_CAT(ResCreated, RenderStatObject_GpuProgram);

		GpuProgramCore::initialize();
	}
}