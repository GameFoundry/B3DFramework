//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsNullRenderTargets.h"

namespace bs
{
	SPtr<RenderWindow> NullRenderWindowManager::CreateImpl(RENDER_WINDOW_DESC& desc, u32 windowId, const SPtr<RenderWindow>& parentWindow)
	{
		if(parentWindow != nullptr)
		{
			u64 hWnd;
			parentWindow->GetCustomAttribute("WINDOW", &hWnd);
			desc.PlatformSpecific["parentWindowHandle"] = toString(hWnd);
		}

		// Create the window
		NullRenderWindow* renderWindow = new(bs_alloc<NullRenderWindow>()) NullRenderWindow(desc, windowId);
		return bs_core_ptr<NullRenderWindow>(renderWindow);
	}

	NullRenderWindow::NullRenderWindow(const RENDER_WINDOW_DESC& desc, u32 windowId)
		: RenderWindow(desc, windowId), mProperties(desc)
	{}

	SPtr<ct::NullRenderWindow> NullRenderWindow::GetCore() const
	{
		return std::static_pointer_cast<ct::NullRenderWindow>(mCoreSpecific);
	}

	void NullRenderWindow::SyncProperties()
	{
		ScopedSpinLock lock(GetCore()->mLock);
		mProperties = GetCore()->mSyncedProperties;
	}

	SPtr<ct::CoreObject> NullRenderWindow::CreateCore() const
	{
		RENDER_WINDOW_DESC desc = mDesc;
		SPtr<ct::CoreObject> coreObj = bs_shared_ptr_new<ct::NullRenderWindow>(desc, mWindowId);
		coreObj->SetThisPtrInternal(coreObj);

		return coreObj;
	}

	void NullRenderWindow::GetCustomAttribute(const String& name, void* pData) const
	{
		if(name == "WINDOW")
		{
			u64* pHwnd = (u64*)pData;
			*pHwnd = 0;
			return;
		}

		RenderWindow::GetCustomAttribute(name, pData);
	}

	namespace ct
	{
		NullRenderWindow::NullRenderWindow(const RENDER_WINDOW_DESC& desc, u32 windowId)
			: RenderWindow(desc, windowId), mProperties(desc), mSyncedProperties(desc)
		{}

		void NullRenderWindow::SyncProperties()
		{
			ScopedSpinLock lock(mLock);
			mProperties = mSyncedProperties;
		}

		void NullRenderWindow::GetCustomAttribute(const String& name, void* pData) const
		{
			if(name == "WINDOW")
			{
				u64* pWnd = (u64*)pData;
				*pWnd = 0;
				return;
			}

			RenderWindow::GetCustomAttribute(name, pData);
		}
	} // namespace ct
} // namespace bs
