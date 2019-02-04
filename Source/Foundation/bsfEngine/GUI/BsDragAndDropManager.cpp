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
		mMouseCaptureChangedConn = Platform::onMouseCaptureChanged.connect(std::bind(&DragAndDropManager::mouseCaptureChanged, this));
		Input::instance().onPointerReleased.connect(std::bind(&DragAndDropManager::cursorReleased, this, _1));
	}

	DragAndDropManager::~DragAndDropManager()
	{
		mMouseCaptureChangedConn.disconnect();
	}

	void DragAndDropManager::addDropCallback(std::function<void(bool)> dropCallback)
	{
		mDropCallbacks.push_back(dropCallback);
	}

	void DragAndDropManager::startDrag(UINT32 typeId, void* data, std::function<void(bool)> dropCallback, bool needsValidDropTarget)
	{
		if (mIsDragInProgress)
			endDrag(false);

		mDragTypeId = typeId;
		mData = data;
		mNeedsValidDropTarget = needsValidDropTarget;
		addDropCallback(dropCallback);
		mIsDragInProgress = true;

		mCaptureActive.store(false);
		mCaptureChanged.store(false);

		Platform::captureMouse(*gCoreApplication().getPrimaryWindow());
	}

	void DragAndDropManager::_update()
	{
		if(!mIsDragInProgress)
			return;

		// This generally happens when window loses focus and capture is lost (for example alt+tab)
		int captureActive = mCaptureActive.load();
		if (!captureActive && mCaptureChanged.load() && 
			(gTime().getFrameIdx() > mCaptureChangeFrame.load())) // Wait one frame to ensure input (like mouse up) gets a chance to be processed
		{
			endDrag(false);
			mCaptureChanged.store(false);
		}
	}

	void DragAndDropManager::endDrag(bool processed)
	{
		for(auto& callback : mDropCallbacks)
			callback(processed);

		mDragTypeId = 0;
		mData = nullptr;
		mDropCallbacks.clear();
		mIsDragInProgress = false;
	}

	void DragAndDropManager::mouseCaptureChanged()
	{
		mCaptureActive.fetch_xor(1); // mCaptureActive = !mCaptureActive;
		mCaptureChanged.store(true);
		mCaptureChangeFrame.store(gTime().getFrameIdx());
	}

	void DragAndDropManager::cursorReleased(const PointerEvent& event)
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