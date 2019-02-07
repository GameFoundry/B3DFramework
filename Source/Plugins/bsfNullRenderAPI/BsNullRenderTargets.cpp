//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsNullRenderTargets.h"

namespace bs
{
	SPtr<RenderWindow> NullRenderWindowManager::createImpl(RENDER_WINDOW_DESC& desc, UINT32 windowId, 
		const SPtr<RenderWindow>& parentWindow)
	{
		if(parentWindow != nullptr)
		{
			UINT64 hWnd;
			parentWindow->getCustomAttribute("WINDOW", &hWnd);
			desc.platformSpecific["parentWindowHandle"] = toString(hWnd);
		}

		// Create the window
		NullRenderWindow* renderWindow = new (bs_alloc<NullRenderWindow>()) NullRenderWindow(desc, windowId);
		return bs_core_ptr<NullRenderWindow>(renderWindow);
	}

	NullRenderWindow::NullRenderWindow(const RENDER_WINDOW_DESC& desc, UINT32 windowId)
		:RenderWindow(desc, windowId), mProperties(desc)
	{ }

	SPtr<ct::NullRenderWindow> NullRenderWindow::getCore() const
	{
		return std::static_pointer_cast<ct::NullRenderWindow>(mCoreSpecific);
	}

	void NullRenderWindow::syncProperties()
	{
		ScopedSpinLock lock(getCore()->mLock);
		mProperties = getCore()->mSyncedProperties;
	}

	SPtr<ct::CoreObject> NullRenderWindow::createCore() const
	{
		RENDER_WINDOW_DESC desc = mDesc;
		SPtr<ct::CoreObject> coreObj = bs_shared_ptr_new<ct::NullRenderWindow>(desc, mWindowId);
		coreObj->_setThisPtr(coreObj);

		return coreObj;
	}

	void NullRenderWindow::getCustomAttribute(const String& name, void* pData) const
	{
		if (name == "WINDOW")
		{
			UINT64 *pHwnd = (UINT64*)pData;
			*pHwnd = 0;
			return;
		}

		RenderWindow::getCustomAttribute(name, pData);
	}

	namespace ct
	{
		NullRenderWindow::NullRenderWindow(const RENDER_WINDOW_DESC& desc, UINT32 windowId)
			: RenderWindow(desc, windowId), mProperties(desc), mSyncedProperties(desc)
		{ }


		void NullRenderWindow::syncProperties()
		{
			ScopedSpinLock lock(mLock);
			mProperties = mSyncedProperties;
		}

		void NullRenderWindow::getCustomAttribute(const String& name, void* pData) const
		{
			if(name == "WINDOW")
			{
				UINT64 *pWnd = (UINT64*)pData;
				*pWnd = 0;
				return;
			}

			RenderWindow::getCustomAttribute(name, pData);
		}
	}
}