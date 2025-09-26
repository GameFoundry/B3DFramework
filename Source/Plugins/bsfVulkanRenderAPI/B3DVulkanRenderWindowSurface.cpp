//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DVulkanRenderWindowSurface.h"
#include "B3DVulkanGpuBackend.h"
#include "B3DVulkanSubmitThread.h"
#include "B3DVulkanSwapChain.h"

using namespace b3d::render;

VulkanRenderWindowSurface::VulkanRenderWindowSurface(const RenderWindowSurfaceCreateInformation& createInformation)
	:mPlatformWindowHandle(createInformation.PlatformWindowHandle)
{
	VkInstance instance = GetVulkanGpuBackend().GetVkInstance();
	VkSurfaceKHR vkSurface;

	// Create Vulkan surface
#if B3D_PLATFORM == B3D_PLATFORM_ID_WIN32
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

	VkResult result = vkCreateWin32SurfaceKHR(instance, &surfaceCreateInfo, gVulkanAllocator, &vkSurface);
	B3D_ASSERT(result == VK_SUCCESS);
#elif B3D_PLATFORM == B3D_PLATFORM_ID_LINUX
	VkXlibSurfaceCreateInfoKHR surfaceCreateInfo;
	surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
	surfaceCreateInfo.pNext = nullptr;
	surfaceCreateInfo.flags = -1;
	surfaceCreateInfo.window = (::Window)mPlatformWindowHandle;
	surfaceCreateInfo.dpy = LinuxPlatform::getXDisplay();

	// Note: I manually lock Xlib, while Vulkan spec says XInitThreads should be called, since Vulkan
	// surely calls Xlib under the hood as well. I've tried to guess which calls use Xlib and lock them
	// externally, but XInitThreads might be required if problems occur.
	VkResult result = vkCreateXlibSurfaceKHR(instance, &surfaceCreateInfo, gVulkanAllocator, &vkSurface);
	B3D_ASSERT(result == VK_SUCCESS);
#elif B3D_PLATFORM == B3D_PLATFORM_ID_MACOS
	MacOSPlatform::lockWindows();

		CocoaWindow* const window = MacOSPlatform::getWindow(mCocoaWindowId);
		if(B3D_ENSURE(window != nullptr))
		{
			MacOSPlatform::unlockWindows();
			return;
		}

		// Create Vulkan surface
		VkMacOSSurfaceCreateInfoMVK surfaceCreateInfo;
		surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_MACOS_SURFACE_CREATE_INFO_MVK;
		surfaceCreateInfo.pNext = nullptr;
		surfaceCreateInfo.flags = 0;
		surfaceCreateInfo.pView = window->_getLayer();

		VkResult result = vkCreateMacOSSurfaceMVK(instance, &surfaceCreateInfo, gVulkanAllocator, &vkSurface);
		B3D_ASSERT(result == VK_SUCCESS);

		MacOSPlatform::unlockWindows();
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
