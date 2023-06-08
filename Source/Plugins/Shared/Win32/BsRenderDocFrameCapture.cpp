//************************************ bs::framework - Copyright 2023 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsRenderDocFrameCapture.h"
#include "Utility/BsDynamicLibraryManager.h"
#include "Utility/BsDynamicLibrary.h"
#include <RenderDoc/renderdoc_app.h>

using namespace bs;

RenderDocFrameCapture::RenderDocFrameCapture(VkInstance vulkanInstance) :
	mVulkanInstance(vulkanInstance), mRenderDocLibrary(nullptr)
{
	mRenderDocLibrary = DynamicLibraryManager::Instance().Load("renderdoc.dll");

	if (mRenderDocLibrary == nullptr)
	{
		B3D_LOG(Error, RenderBackend, "Failed loading RenderDoc API. renderdoc.dll cannot be found/loaded.");
		return;
	}

	pRENDERDOC_GetAPI fnGetAPI = static_cast<pRENDERDOC_GetAPI>(mRenderDocLibrary->GetSymbol("RENDERDOC_GetAPI"));

	if (!B3D_ENSURE(fnGetAPI))
		return;

	if (!B3D_ENSURE(fnGetAPI(eRENDERDOC_API_Version_1_0_0, &mRenderDocAPIPointers) == 1))
		return;
}

RenderDocFrameCapture::~RenderDocFrameCapture()
{
	if (mRenderDocLibrary != nullptr)
		DynamicLibraryManager::Instance().Unload(mRenderDocLibrary);
}

void RenderDocFrameCapture::Start()
{
	if (!mRenderDocAPIPointers)
		return;

	RENDERDOC_API_1_0_0* renderDocAPI = static_cast<RENDERDOC_API_1_0_0*>(mRenderDocAPIPointers);
	renderDocAPI->StartFrameCapture(RENDERDOC_DEVICEPOINTER_FROM_VKINSTANCE(mVulkanInstance), nullptr);

	mIsCaptureInProgress = true;
}

void RenderDocFrameCapture::Stop()
{
	if (!mIsCaptureInProgress)
		return;

	if (mRenderDocAPIPointers)
	{
		RENDERDOC_API_1_0_0* renderDocAPI = static_cast<RENDERDOC_API_1_0_0*>(mRenderDocAPIPointers);
		renderDocAPI->EndFrameCapture(RENDERDOC_DEVICEPOINTER_FROM_VKINSTANCE(mVulkanInstance), nullptr);
	}

	mIsCaptureInProgress = false;
}
