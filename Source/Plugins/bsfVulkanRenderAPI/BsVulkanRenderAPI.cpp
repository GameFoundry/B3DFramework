//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsVulkanRenderAPI.h"
#include "CoreThread/BsCoreThread.h"
#include "Profiling/BsRenderStats.h"
#include "RenderAPI/BsGpuProgramParameterDescription.h"
#include "BsVulkanGpuDevice.h"
#include "Managers/BsVulkanTextureManager.h"
#include "Managers/BsVulkanRenderWindowManager.h"
#include "Managers/BsVulkanQueryManager.h"
#include "BsVulkanGpuCommandBuffer.h"
#include "BsVulkanGpuParameters.h"
#include "Managers/BsVulkanVertexInputManager.h"
#include "BsVulkanGpuBuffer.h"
#include "BsVulkanGpuQueue.h"
#include "BsVulkanFramebuffer.h"
#include "BsVulkanUtility.h"
#include "BsVulkanRenderPass.h"
#include "BsVulkanSubmitThread.h"
#include "BsVulkanSwapChain.h"
#include "Win32/BsWin32RenderWindow.h"

#include <vulkan/vulkan.h>


using namespace bs;
using namespace bs::ct;

const StringID& VulkanRenderAPI::GetName() const
{
	static StringID strName("VulkanRenderAPI");
	return strName;
}

void VulkanRenderAPI::Initialize()
{
	THROW_IF_NOT_CORE_THREAD;

	// TODO - Move this to CoreApplication once I get rid of VulkanRenderAPI
	GpuBackend::StartUp<VulkanGpuBackend>();

	// TODO - Currently the GpuBackend always only initializes a single device. Once we change it to support multiple, pick the device here and initialize it1
	mPrimaryGpuDevice = GpuBackend::Instance().GetDevice(0);
	mPrimaryGpuDevice->Initialize();

	RenderAPI::Initialize();
}

void VulkanRenderAPI::DestroyCore()
{
	THROW_IF_NOT_CORE_THREAD;

	GetVulkanSubmitThread().WaitUntilIdle(true);
	GetVulkanSubmitThread().RefreshCommandBufferCompletionStates();

	mPrimaryGpuDevice = nullptr;

	// TODO - Move this to CoreApplication once I get rid of VulkanRenderAPI
	GpuBackend::ShutDown();

	RenderAPI::DestroyCore();
}

void VulkanRenderAPI::BeginFrame()
{
	THROW_IF_NOT_CORE_THREAD

	GetVulkanSubmitThread().RefreshCommandBufferCompletionStates();
}

void VulkanRenderAPI::EndFrame()
{
	VulkanGpuDevice* const vulkanGpuDevice = static_cast<VulkanGpuDevice*>(mPrimaryGpuDevice.get());
	if(vulkanGpuDevice == nullptr)
		return;

	vulkanGpuDevice->SubmitTransferCommandBuffers();

	GetVulkanSubmitThread().QueueRefreshCommandBufferCompletionStates(vulkanGpuDevice);
}

void VulkanRenderAPI::SwapBuffers(const SPtr<RenderTarget>& target, u32 syncMask)
{
	THROW_IF_NOT_CORE_THREAD

	if(target == nullptr || !target->GetProperties().IsWindow)
		return;

	// Retrieve the swap chain before command buffer submit, as the submit might internally rebuild the swap chain.
	VulkanSwapChain* swapChain = nullptr;

#if B3D_PLATFORM == B3D_PLATFORM_ID_WIN32
	Win32RenderWindow* window = static_cast<Win32RenderWindow*>(target.get());
#elif B3D_PLATFORM == B3D_PLATFORM_ID_LINUX
	LinuxRenderWindow* window = static_cast<LinuxRenderWindow*>(target.get());
#elif B3D_PLATFORM == B3D_PLATFORM_ID_MACOS
	MacOSRenderWindow* window = static_cast<MacOSRenderWindow*>(target.get());
#endif

	window->SwapBuffers();
	swapChain = window->GetSwapChain();

	VulkanGpuQueue* const presentQueue = static_cast<VulkanGpuQueue*>(GetVulkanGpuBackend().GetPresentDevice()->GetQueue(GQT_GRAPHICS, 0).get());
	GetVulkanSubmitThread().QueuePresent(*presentQueue, *swapChain, syncMask);

	// Ensure the acquire operation we queued the previous frame has finished. This also means the old image was presented.
	swapChain->WaitUntilFirstImageAcquired();

	GetVulkanSubmitThread().QueueImageAcquire(*swapChain);

	B3D_INCREMENT_RENDER_STATISTIC(NumPresents);
}

namespace bs { namespace ct {
VulkanRenderAPI& GetVulkanRenderAPI()
{
	return static_cast<VulkanRenderAPI&>(RenderAPI::Instance());
}
}} // namespace bs::ct
