//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DRenderDocFrameCapture.h"
#include "Utility/B3DDynamicLibraryManager.h"
#include "Utility/B3DDynamicLibrary.h"
#include <RenderDoc/renderdoc_app.h>

using namespace b3d;

RenderDocFrameCapture::RenderDocFrameCapture(VkInstance vulkanInstance) :
	mVulkanInstance(vulkanInstance), mRenderDocLibrary(nullptr)
{
	LoadRenderDocAPI();
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
		B3D_ENSURE(renderDocAPI->EndFrameCapture(RENDERDOC_DEVICEPOINTER_FROM_VKINSTANCE(mVulkanInstance), nullptr) == 1);

		renderDocAPI->ShowReplayUI();
	}

	mIsCaptureInProgress = false;
}

void RenderDocFrameCapture::LoadRenderDocAPI()
{
	if(mRenderDocLibrary != nullptr)
	{
		mRenderDocLibrary->Unload();
		DynamicLibraryManager::Instance().Unload(mRenderDocLibrary);
		mRenderDocLibrary = nullptr;
	}

	mRenderDocLibrary = DynamicLibraryManager::Instance().Load("renderdoc.dll");

	if (mRenderDocLibrary == nullptr)
	{
		B3D_LOG(Error, LogRenderBackend, "Failed loading RenderDoc API. renderdoc.dll cannot be found/loaded.");
		return;
	}

	pRENDERDOC_GetAPI fnGetAPI = static_cast<pRENDERDOC_GetAPI>(mRenderDocLibrary->GetSymbol("RENDERDOC_GetAPI"));

	if (!B3D_ENSURE(fnGetAPI))
		return;

	if (!B3D_ENSURE(fnGetAPI(eRENDERDOC_API_Version_1_0_0, &mRenderDocAPIPointers) == 1))
		return;
}

