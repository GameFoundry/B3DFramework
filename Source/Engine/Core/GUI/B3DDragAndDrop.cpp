//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/B3DDragAndDrop.h"
#include "B3DApplication.h"
#include "Platform/B3DPlatform.h"
#include "Utility/B3DTime.h"
#include "RTTI/B3DDragAndDropDataRTTI.h"

using namespace b3d;

RTTIType* DragAndDropData::GetRttiStatic()
{
	return DragAndDropDataRTTI::Instance();
}

RTTIType* DragAndDropData::GetRtti() const
{
	return GetRttiStatic();
}

RTTIType* SceneObjectDragAndDropData::GetRttiStatic()
{
	return SceneObjectDragAndDropDataRTTI::Instance();
}

RTTIType* SceneObjectDragAndDropData::GetRtti() const
{
	return GetRttiStatic();
}

RTTIType* ResourceDragAndDropData::GetRttiStatic()
{
	return ResourceDragAndDropDataRTTI::Instance();
}

RTTIType* ResourceDragAndDropData::GetRtti() const
{
	return GetRttiStatic();
}

DragAndDrop::DragAndDrop()
{
	mMouseCaptureChangedConn = Platform::OnMouseCaptureChanged.Connect([this]() { MouseCaptureChanged(); });
	Input::Instance().OnPointerReleased.Connect([this](const PointerEvent& event) { CursorReleased(event); });
}

DragAndDrop::~DragAndDrop()
{
	mMouseCaptureChangedConn.Disconnect();
}

void DragAndDrop::AddDropCallback(Function<void(bool)>&& dropCallback)
{
	mDropCallbacks.emplace_back(std::move(dropCallback));
}

void DragAndDrop::StartDrag(const SPtr<DragAndDropData>& data, Function<void(bool)>&& dropCallback, bool needsValidDropTarget)
{
	if(IsDropInProgress())
		EndDrag(false);

	mDragData = data;
	mNeedsValidDropTarget = needsValidDropTarget;

	if(dropCallback != nullptr)
		AddDropCallback(std::move(dropCallback));

	mCaptureActive.store(false);
	mCaptureChanged.store(false);

	Platform::CaptureMouse(*GetApplication().GetPrimaryWindow());
}

void DragAndDrop::Update()
{
	if(mDropData != nullptr)
	{
		// Activate drop for this frame if requested
		if(!mIsDropActiveThisFrame)
			mIsDropActiveThisFrame = true;
		else
		{
			// CLear drop data so we don't try to reactivate the drop next frame
			mDropData = nullptr;
			mIsDropActiveThisFrame = false;
		}
	}

	if(IsDragInProgress())
	{
		// This generally happens when window loses focus and capture is lost (for example alt+tab)
		int captureActive = mCaptureActive.load();
		if(!captureActive && mCaptureChanged.load() &&
		   (GetTime().GetCurrentFrameIndex() > mCaptureChangeFrame.load())) // Wait one frame to ensure input (like mouse up) gets a chance to be processed
		{
			EndDrag(false);
			mCaptureChanged.store(false);
		}
	}
}

void DragAndDrop::EndDrag(bool processed)
{
	for(auto& callback : mDropCallbacks)
		callback(processed);

	mDropData = mDragData;
	mIsDropActiveThisFrame = false; // We activate on next update, otherwise IsDropInProgress() checks could return true for two frames in a row
	mDragData = nullptr;

	mDropCallbacks.clear();
}

void DragAndDrop::MouseCaptureChanged()
{
	mCaptureActive.fetch_xor(1); // mCaptureActive = !mCaptureActive;
	mCaptureChanged.store(true);
	mCaptureChangeFrame.store(GetTime().GetCurrentFrameIndex());
}

void DragAndDrop::CursorReleased(const PointerEvent& event)
{
	if(!IsDragInProgress())
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
