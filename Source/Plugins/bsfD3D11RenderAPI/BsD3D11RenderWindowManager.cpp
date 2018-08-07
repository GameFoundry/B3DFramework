//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsD3D11RenderWindowManager.h"
#include "BsD3D11RenderAPI.h"
#include "BsD3D11RenderWindow.h"
#include "Threading/BsAsyncOp.h"

namespace bs
{
	D3D11RenderWindowManager::D3D11RenderWindowManager(ct::D3D11RenderAPI* renderSystem)
		:mRenderSystem(renderSystem)
	{
		assert(mRenderSystem != nullptr);
	}

	SPtr<RenderWindow> D3D11RenderWindowManager::createImpl(RENDER_WINDOW_DESC& desc, UINT32 windowId, 
		const SPtr<RenderWindow>& parentWindow)
	{
		ct::RenderAPI* rs = ct::RenderAPI::instancePtr();
		ct::D3D11RenderAPI* d3d11rs = static_cast<ct::D3D11RenderAPI*>(rs);

		if(parentWindow != nullptr)
		{
			UINT64 hWnd;
			parentWindow->getCustomAttribute("WINDOW", &hWnd);
			desc.platformSpecific["parentWindowHandle"] = toString(hWnd);
		}

		// Create the window
		D3D11RenderWindow* renderWindow = new (bs_alloc<D3D11RenderWindow>()) D3D11RenderWindow(desc, windowId);
		return bs_core_ptr<D3D11RenderWindow>(renderWindow);
	}
}