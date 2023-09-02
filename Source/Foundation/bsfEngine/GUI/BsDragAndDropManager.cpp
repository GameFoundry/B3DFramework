//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/BsDragAndDropManager.h"
#include "Platform/BsPlatform.h"
#include "BsCoreApplication.h"
#include "Utility/BsTime.h"

using namespace std::placeholders;

using namespace bs;

DragAndDropManager::DragAndDropManager()
{
	mMouseCaptureChangedConn = Platform::onMouseCaptureChanged.Connect(std::bind(&DragAndDropManager::mCaptureChanged, this));
	Input::Instance().OnPointerReleased.Connect(std::bind(&DragAndDropManager::CursorReleased, this, _1));
}

DragAndDropManager::~DragAndDropManager()
{
	mMouseCaptureChangedConn.Disconnect();
}

void DragAndDropManager::AddDropCallback(std::function<void(bool)> dropCallback)
{
	mDropCallbacks.push_back(dropCallback);
}

void DragAndDropManager::StartDrag(u32 typeId, void* data, std::function<void(bool)> dropCallback, bool needsValidDropTarget)
{
	if(mIsDragInProgress)
		EndDrag(false);

	mDragTypeId = typeId;
	mData = data;
	mNeedsValidDropTarget = needsValidDropTarget;
	AddDropCallback(dropCallback);
	mIsDragInProgress = true;

	mCaptureActive.store(false);
	mCaptureChanged.store(false);

	Platform::CaptureMouse(*GetCoreApplication().GetPrimaryWindow());
}

void DragAndDropManager::UpdateInternal()
{
	if(!mIsDragInProgress)
		return;

	// This generally happens when window loses focus and capture is lost (for example alt+tab)
	int captureActive = mCaptureActive.load();
	if(!captureActive && mCaptureChanged.load() &&
	   (GetTime().GetCurrentFrameIndex() > mCaptureChangeFrame.load())) // Wait one frame to ensure input (like mouse up) gets a chance to be processed
	{
		EndDrag(false);
		mCaptureChanged.store(false);
	}
}

void DragAndDropManager::EndDrag(bool processed)
{
	for(auto& callback : mDropCallbacks)
		callback(processed);

	mDragTypeId = 0;
	mData = nullptr;
	mDropCallbacks.clear();
	mIsDragInProgress = false;
}

void DragAndDropManager::MouseCaptureChanged()
{
	mCaptureActive.fetch_xor(1); // mCaptureActive = !mCaptureActive;
	mCaptureChanged.store(true);
	mCaptureChangeFrame.store(GetTime().GetCurrentFrameIndex());
}

void DragAndDropManager::CursorReleased(const PointerEvent& event)
{
	if(!mIsDragInProgress)
		return;

	if(!OnDragEnded.Empty())
	{
		DragCallbackInfo info;
		OnDragEnded(event, info);

		EndDrag(info.Processed);
	}
	else
		EndDrag(false);

	Platform::ReleaseMouseCapture();
}
