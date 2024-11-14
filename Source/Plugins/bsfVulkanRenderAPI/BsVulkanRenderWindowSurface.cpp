//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsVulkanRenderWindowSurface.h"
#include "BsVulkanGpuBackend.h"
#include "BsCoreApplication.h"
#include "BsVulkanSubmitThread.h"
#include "BsVulkanSwapChain.h"

using namespace bs::ct;

VulkanRenderWindowSurface::VulkanRenderWindowSurface(const RenderWindowSurfaceCreateInformation& createInformation)
	:mPlatformWindowHandle(createInformation.PlatformWindowHandle)
{
#if B3D_PLATFORM == B3D_PLATFORM_ID_WIN32
	// Create Vulkan surface
	VkWin32SurfaceCreateInfoKHR surfaceCreateInfo;
	surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	surfaceCreateInfo.pNext = nullptr;
	surfaceCreateInfo.flags = 0;
	surfaceCreateInfo.hwnd = (HWND)mPlatformWindowHandle;

#ifdef BS_STATIC_LIB
	surfaceCreateInfo.hinstance = GetModuleHandle(NULL);
#else
	surfaceCreateInfo.hinstance = GetModuleHandle("bsfVulkanRenderAPI.dll");
#endif

	VkInstance instance = GetVulkanGpuBackend().GetVkInstance();
	VkSurfaceKHR vkSurface;
	VkResult result = vkCreateWin32SurfaceKHR(instance, &surfaceCreateInfo, gVulkanAllocator, &vkSurface);
	B3D_ASSERT(result == VK_SUCCESS);
#elif B3D_PLATFORM == B3D_PLATFORM_ID_LINUX
	static_assert(false); // TODO
#elif B3D_PLATFORM == B3D_PLATFORM_ID_MACOS
	static_assert(false); // TODO
#else
	static_assert(false);
#endif

	mSurface = B3DMakeShared<VulkanSurface>(vkSurface);

	SPtr<VulkanGpuDevice> presentDevice = GetVulkanGpuBackend().GetPresentDevice();
	VkPhysicalDevice physicalDevice = presentDevice->GetPhysical();

	mPresentQueueFamily = presentDevice->GetQueueFamily(GQT_GRAPHICS);

	VkBool32 supportsPresent;
	vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, mPresentQueueFamily, vkSurface, &supportsPresent);

	if(!supportsPresent)
	{
		// Note: Not supporting present only queues at the moment
		// Note: Also present device can only return one family of graphics queue, while there could be more (some of
		// which support present)
		B3D_EXCEPT(RenderingAPIException, "Cannot find a graphics queue that also supports present operations.");
	}

	SurfaceFormat format = presentDevice->GetSurfaceFormat(vkSurface, createInformation.UseHardwareSRGB);
	mColorFormat = format.ColorFormat;
	mColorSpace = format.ColorSpace;
	mDepthFormat = format.DepthFormat;
	mCreateDepthBuffer = createInformation.CreateDepthBuffer;

	// Create swap chain
	mSwapChain = presentDevice->GetResourceManager().Create<VulkanSwapChain>(mSurface, createInformation.Width, createInformation.Height, createInformation.VSync, mColorFormat, mColorSpace, createInformation.CreateDepthBuffer, mDepthFormat);
}

VulkanRenderWindowSurface::~VulkanRenderWindowSurface()
{
	Destroy();
}

void VulkanRenderWindowSurface::RebuildSwapChain(u32 width, u32 height, bool vsync)
{
	GetVulkanSubmitThread().WaitUntilIdle();

	SPtr<VulkanGpuDevice> presentDevice = GetVulkanGpuBackend().GetPresentDevice();
	VulkanSwapChain* oldSwapChain = mSwapChain;
	oldSwapChain->MarkAsRetired();

	mSwapChain = presentDevice->GetResourceManager().Create<VulkanSwapChain>(mSurface, width, height, vsync, mColorFormat, mColorSpace, mCreateDepthBuffer, mDepthFormat, oldSwapChain);
	oldSwapChain->Destroy();
}

void VulkanRenderWindowSurface::MarkSwapChainAsInvalid()
{
	if(mSwapChain != nullptr)
		mSwapChain->MarkAsInvalid();
}

void VulkanRenderWindowSurface::Destroy()
{
	if(mIsDestroyed)
		return;

	GetVulkanSubmitThread().WaitUntilIdle();
	mSwapChain->Destroy();
	mSwapChain = nullptr;

	mIsDestroyed = true;
}
