//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DVulkanRenderWindowSurface.h"
#include "B3DVulkanFramebuffer.h"
#include "B3DVulkanGpuBackend.h"
#include "B3DVulkanGpuQueue.h"
#include "GpuBackend/B3DGpuSubmitThread.h"
#include "B3DVulkanSwapChain.h"

using namespace b3d;
using namespace b3d::render;

VulkanRenderWindowSurface::VulkanRenderWindowSurface(const RenderWindowSurfaceCreateInformation& createInformation)
	:mPlatformWindowHandle(createInformation.PlatformWindowHandle)
{
	VkInstance instance = GetVulkanGpuBackend().GetVkInstance();
	VkSurfaceKHR vkSurface;

	// Create Vulkan surface
#if B3D_PLATFORM_WIN32
	VkWin32SurfaceCreateInfoKHR surfaceCreateInfo;
	surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	surfaceCreateInfo.pNext = nullptr;
	surfaceCreateInfo.flags = 0;
	surfaceCreateInfo.hwnd = (HWND)mPlatformWindowHandle;

#ifdef B3D_STATIC_LIB
	surfaceCreateInfo.hinstance = GetModuleHandle(NULL);
#else
	surfaceCreateInfo.hinstance = GetModuleHandle("bsfVulkanGpuBackend.dll");
#endif

	VkResult result = vkCreateWin32SurfaceKHR(instance, &surfaceCreateInfo, gVulkanAllocator, &vkSurface);
	B3D_ASSERT(result == VK_SUCCESS);
#elif B3D_PLATFORM_LINUX
	VkXlibSurfaceCreateInfoKHR surfaceCreateInfo;
	surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
	surfaceCreateInfo.pNext = nullptr;
	surfaceCreateInfo.flags = 0;
	surfaceCreateInfo.window = (::Window)mPlatformWindowHandle;
	surfaceCreateInfo.dpy = LinuxPlatform::getXDisplay();

	// Note: I manually lock Xlib, while Vulkan spec says XInitThreads should be called, since Vulkan
	// surely calls Xlib under the hood as well. I've tried to guess which calls use Xlib and lock them
	// externally, but XInitThreads might be required if problems occur.
	VkResult result = vkCreateXlibSurfaceKHR(instance, &surfaceCreateInfo, gVulkanAllocator, &vkSurface);
	B3D_ASSERT(result == VK_SUCCESS);
#elif B3D_PLATFORM_MACOS
	MacOSPlatform::LockWindows();

		CocoaWindow* const window = MacOSPlatform::GetWindow((u32)mPlatformWindowHandle);
		if(!B3D_ENSURE(window != nullptr))
		{
			MacOSPlatform::UnlockWindows();
			return;
		}

		// Create Vulkan surface
		VkMacOSSurfaceCreateInfoMVK surfaceCreateInfo;
		surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_MACOS_SURFACE_CREATE_INFO_MVK;
		surfaceCreateInfo.pNext = nullptr;
		surfaceCreateInfo.flags = 0;
		surfaceCreateInfo.pView = window->GetLayerInternal();

		VkResult result = vkCreateMacOSSurfaceMVK(instance, &surfaceCreateInfo, gVulkanAllocator, &vkSurface);
		B3D_ASSERT(result == VK_SUCCESS);

		MacOSPlatform::UnlockWindows();
#else
	static_assert(false);
#endif

	mSurface = B3DMakeShared<VulkanSurface>(vkSurface);

	TShared<VulkanGpuDevice> presentDevice = GetVulkanGpuBackend().GetPresentDevice();
	VkPhysicalDevice physicalDevice = presentDevice->GetPhysical();

	mPresentQueueFamily = presentDevice->GetQueueFamily(GQT_GRAPHICS);

	VkBool32 supportsPresent;
	vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, mPresentQueueFamily, vkSurface, &supportsPresent);

	if(!supportsPresent)
	{
		// Note: Not supporting present only queues at the moment
		// Note: Also present device can only return one family of graphics queue, while there could be more (some of
		// which support present)
		B3D_ENSURE_LOG(false, "Cannot find a graphics queue that also supports present operations.");
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

void VulkanRenderWindowSurface::RebuildSwapChain(u32 width, u32 height, bool vsync, u32 vsyncInterval)
{
	TShared<VulkanGpuDevice> presentDevice = GetVulkanGpuBackend().GetPresentDevice();
	presentDevice->GetSubmitThread().WaitUntilIdle();

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

	mSwapChain->GetDevice().GetSubmitThread().WaitUntilIdle();
	mSwapChain->Destroy();
	mSwapChain = nullptr;

	mIsDestroyed = true;
}

void VulkanRenderWindowSurface::SwapBuffers(GpuQueue& queue, GpuQueueMask syncMask)
{
	GpuSubmitThread& submitThread = mSwapChain->GetDevice().GetSubmitThread();

	submitThread.QueuePresent(queue, *mSwapChain, syncMask);

	// Ensure the acquire operation we queued the previous frame has finished. This also means the old image was presented.
	mSwapChain->WaitUntilFirstImageAcquired();

	submitThread.QueueImageAcquire(*mSwapChain);
}

VulkanFramebuffer* VulkanRenderWindowSurface::GetActiveFramebuffer(bool acquireIfUnavailable)
{
	B3D_ASSERT(mSwapChain != nullptr);

	// If there is a swap chain acquire already queued, wait for it
	mSwapChain->WaitUntilFirstImageAcquired();

	// Try to get already-acquired image
	bool isImageAcquired = mSwapChain->TryGetFirstAcquiredImageIndex(mActiveImageIndex);

	// It's possible this is a fresh swap chain and no acquires were queued for it yet
	if(!isImageAcquired && acquireIfUnavailable)
	{
		mSwapChain->GetDevice().GetSubmitThread().QueueImageAcquire(*mSwapChain);
		mSwapChain->WaitUntilFirstImageAcquired();
		isImageAcquired = mSwapChain->TryGetFirstAcquiredImageIndex(mActiveImageIndex);
	}

	if(!isImageAcquired)
		return nullptr;

	return mSwapChain->GetFramebufferForImage(mActiveImageIndex);
}

bool VulkanRenderWindowSurface::AppendWaitSemaphoresIfRequired(TInlineArray<VulkanSemaphore*, 8>& outSemaphores)
{
	B3D_ASSERT(mSwapChain != nullptr);

	return mSwapChain->AppendWaitSemaphoreIfRequired(mActiveImageIndex, outSemaphores);
}

bool VulkanRenderWindowSurface::IsSwapChainValid() const
{
	B3D_ASSERT(mSwapChain != nullptr);

	return mSwapChain->IsValid();
}

VulkanImage* VulkanRenderWindowSurface::GetCurrentColorImage() const
{
	if(mSwapChain == nullptr)
		return nullptr;

	VulkanFramebuffer* framebuffer = mSwapChain->GetFramebufferForImage(mActiveImageIndex);
	if(framebuffer == nullptr)
		return nullptr;

	return framebuffer->GetColorAttachment(0).Image;
}

PixelFormat VulkanRenderWindowSurface::GetColorPixelFormat() const
{
	switch(mColorFormat)
	{
	case VK_FORMAT_R8G8B8A8_UNORM:
	case VK_FORMAT_R8G8B8A8_SRGB:
		return PF_RGBA8;
	case VK_FORMAT_B8G8R8A8_UNORM:
	case VK_FORMAT_B8G8R8A8_SRGB:
		return PF_BGRA8;
	case VK_FORMAT_R16G16B16A16_SFLOAT:
		return PF_RGBA16F;
	case VK_FORMAT_A2B10G10R10_UNORM_PACK32:
		return PF_RGB10A2;
	default:
		B3D_LOG(Warning, LogRenderBackend, "Unhandled swap chain format {0} in GetColorPixelFormat(), defaulting to PF_RGBA8", (u32)mColorFormat);
		return PF_RGBA8;
	}
}

