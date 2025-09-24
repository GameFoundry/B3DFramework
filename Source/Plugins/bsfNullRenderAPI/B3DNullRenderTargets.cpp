//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DNullRenderTargets.h"

using namespace b3d;

SPtr<RenderWindow> NullRenderWindowManager::CreateImpl(RENDER_WINDOW_DESC& desc, u32 windowId, const SPtr<RenderWindow>& parentWindow)
{
	if(parentWindow != nullptr)
	{
		u64 hWnd;
		parentWindow->GetCustomAttribute("WINDOW", &hWnd);
		desc.PlatformSpecific["parentWindowHandle"] = ToString(hWnd);
	}

	// Create the window
	NullRenderWindow* renderWindow = new(B3DAllocate<NullRenderWindow>()) NullRenderWindow(desc, windowId);
	return B3DMakeCoreFromExisting<NullRenderWindow>(renderWindow);
}

NullRenderWindow::NullRenderWindow(const RENDER_WINDOW_DESC& desc, u32 windowId)
	: RenderWindow(desc, windowId), mProperties(desc)
{}

SPtr<render::NullRenderWindow> NullRenderWindow::GetCore() const
{
	return std::static_pointer_cast<render::NullRenderWindow>(mCoreSpecific);
}

void NullRenderWindow::SyncProperties()
{
	ScopedSpinLock lock(GetCore()->mLock);
	mProperties = GetCore()->mSyncedProperties;
}

SPtr<render::CoreObject> NullRenderWindow::CreateCore() const
{
	RENDER_WINDOW_DESC desc = mDesc;
	SPtr<render::CoreObject> coreObj = B3DMakeShared<render::NullRenderWindow>(desc, mWindowId);
	coreObj->SetShared(coreObj);

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

namespace b3d { namespace render {
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
}} // namespace b3d::render
