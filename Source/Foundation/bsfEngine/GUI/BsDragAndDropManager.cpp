//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/BsDragAndDropManager.h"
#include "Platform/BsPlatform.h"
#include "BsCoreApplication.h"
#include "Utility/BsTime.h"

using namespace std::placeholders;

namespace bs
{
	DragAndDropManager::DragAndDropManager()
	{
		mMouseCaptureChangedConn = Platform::onMouseCaptureChanged.Connect(std::bind(&DragAndDropManager::mouseCaptureChanged, this));
		Input::instance().onPointerReleased.Connect(std::bind(&DragAndDropManager::cursorReleased, this, _1));
	}

	DragAndDropManager::~DragAndDropManager()
	{
		mMouseCaptureChangedConn.Disconnect();
	}

	void DragAndDropManager::AddDropCallback(std::function<void(bool)> dropCallback)
	{
		mDropCallbacks.push_back(dropCallback);
	}

	void DragAndDropManager::StartDrag(UINT32 typeId, void* data, std::function<void(bool)> dropCallback, bool needsValidDropTarget)
	{
		if (mIsDragInProgress)
			endDrag(false);

		mDragTypeId = typeId;
		mData = data;
		mNeedsValidDropTarget = needsValidDropTarget;
		addDropCallback(dropCallback);
		mIsDragInProgress = true;

		mCaptureActive.Store(false);
		mCaptureChanged.Store(false);

		Platform::captureMouse(*gCoreApplication().GetPrimaryWindow());
	}

	void DragAndDropManager::_update()
	{
		if(!mIsDragInProgress)
			return;

		// This generally happens when window loses focus and capture is lost (for example alt+tab)
		int captureActive = mCaptureActive.Load();
		if (!captureActive && mCaptureChanged.Load() &&
			(gTime().GetFrameIdx() > mCaptureChangeFrame.load())) // Wait one frame to ensure input (like mouse up) gets a chance to be processed
		{
			endDrag(false);
			mCaptureChanged.Store(false);
		}
	}

	void DragAndDropManager::EndDrag(bool processed)
	{
		for(auto& callback : mDropCallbacks)
			callback(processed);

		mDragTypeId = 0;
		mData = nullptr;
		mDropCallbacks.Clear();
		mIsDragInProgress = false;
	}

	void DragAndDropManager::MouseCaptureChanged()
	{
		mCaptureActive.fetch_xor(1); // mCaptureActive = !mCaptureActive;
		mCaptureChanged.Store(true);
		mCaptureChangeFrame.Store(gTime().getFrameIdx());
	}

	void DragAndDropManager::CursorReleased(const PointerEvent& event)
	{
		if(!mIsDragInProgress)
			return;

		if(!onDragEnded.empty())
		{
			DragCallbackInfo info;
			onDragEnded(event, info);

			endDrag(info.processed);
		}
		else
			endDrag(false);

		Platform::releaseMouseCapture();
	}
}
