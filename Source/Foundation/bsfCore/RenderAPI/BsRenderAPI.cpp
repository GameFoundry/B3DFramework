//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "RenderAPI/BsRenderAPI.h"

#include "CoreThread/BsCoreThread.h"
#include "RenderAPI/BsViewport.h"
#include "RenderAPI/BsRenderTarget.h"
#include "RenderAPI/BsRenderWindow.h"
#include "Mesh/BsMesh.h"
#include "RenderAPI/BsGpuParameters.h"
#include "RenderAPI/BsGpuPipelineState.h"

using namespace std::placeholders;

using namespace bs;

namespace bs { namespace ct
{
RenderAPI::RenderAPI()
{
}

RenderAPI::~RenderAPI()
{
	// Base classes need to call virtual destroy_internal method instead of a destructor
}

SPtr<bs::RenderWindow> RenderAPI::Initialize(const RENDER_WINDOW_DESC& primaryWindowDesc)
{
	GetCoreThread().QueueCommand(std::bind((void(RenderAPI::*)()) & RenderAPI::Initialize, this), CTQF_InternalQueue | CTQF_BlockUntilComplete);

	RENDER_WINDOW_DESC windowDesc = primaryWindowDesc;
	SPtr<bs::RenderWindow> renderWindow = bs::RenderWindow::Create(windowDesc, nullptr);

	// Make sure render window initialization is submitted to the internal queue
	GetCoreThread().SubmitAll();

	GetCoreThread().QueueCommand(std::bind(&RenderAPI::InitializeWithWindow, this, renderWindow->GetCore()), CTQF_InternalQueue | CTQF_BlockUntilComplete);

	return renderWindow;
}

void RenderAPI::Initialize()
{
	// Do nothing
}

void RenderAPI::InitializeWithWindow(const SPtr<RenderWindow>& primaryWindow)
{
	THROW_IF_NOT_CORE_THREAD;
}

void RenderAPI::Destroy()
{
	GetCoreThread().QueueCommand(std::bind(&RenderAPI::DestroyCore, this));
	GetCoreThread().SubmitAll(true);
}

void RenderAPI::DestroyCore()
{
}

u32 RenderAPI::VertexCountToPrimCount(DrawOperationType type, u32 elementCount)
{
	u32 primCount = 0;
	switch(type)
	{
	case DOT_POINT_LIST:
		primCount = elementCount;
		break;

	case DOT_LINE_LIST:
		primCount = elementCount / 2;
		break;

	case DOT_LINE_STRIP:
		primCount = elementCount - 1;
		break;

	case DOT_TRIANGLE_LIST:
		primCount = elementCount / 3;
		break;

	case DOT_TRIANGLE_STRIP:
		primCount = elementCount - 2;
		break;

	case DOT_TRIANGLE_FAN:
		primCount = elementCount - 2;
		break;
	}

	return primCount;
}

}}
