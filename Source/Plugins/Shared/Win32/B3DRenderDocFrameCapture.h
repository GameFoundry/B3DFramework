//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "RenderAPI/B3DGpuFrameCapture.h"

// TODO - Surround if an #if so this can be included from non-Vulkan render backends (all other Vulkan specific code too)
#include <vulkan/vulkan.h>

namespace b3d
{
	/** Captures GPU commands for the RenderDoc tool. */
	class RenderDocFrameCapture : public GpuFrameCapture
	{
	public:
		explicit RenderDocFrameCapture(VkInstance vulkanInstance);
		virtual ~RenderDocFrameCapture();

		void Start() override;
		void Stop() override;

	private:
		/** Loads the RenderDoc library and initializes the API pointers. */
		void LoadRenderDocAPI();

		const VkInstance mVulkanInstance;
		DynamicLibrary* mRenderDocLibrary;
		void* mRenderDocAPIPointers = nullptr;
		bool mIsCaptureInProgress = false;

	};
} // namespace b3d
