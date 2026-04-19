//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2026 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "B3DLongOperationTracker.h"

using namespace b3d;

LongOperationTracker::LongOperationTracker(String name, LongOperationFlags flags, String description, String category)
	: mName(std::move(name)), mDescription(std::move(description)), mCategory(std::move(category)), mFlags(flags)
{
	// TODO - Register with global async operation manager
}

LongOperationTracker::~LongOperationTracker()
{
	// TODO - Unregister the global async operation manager
}

float LongOperationTracker::GetProgressPercent() const
{
	Lock lock(mMutex);
	return mProgressPercent;
}

void LongOperationTracker::NotifyOperationStarted()
{
	Lock lock(mMutex);

	if(mState != LongOperationState::NotStarted)
		return;

	mState = LongOperationState::Running;
}

void LongOperationTracker::NotifyOperationFinished()
{
	Lock lock(mMutex);

	if(mState == LongOperationState::Aborted || mState == LongOperationState::Failed)
		return;

	mState = LongOperationState::Finished;
	mProgressPercent = 1.0f;
}

void LongOperationTracker::NotifyOperationAborted()
{
	Lock lock(mMutex);

	if(mState == LongOperationState::Finished || mState == LongOperationState::Failed)
		return;

	mState = LongOperationState::Aborted;
}

void LongOperationTracker::NotifyOperationFailed(const String& error)
{
	Lock lock(mMutex);

	if(mState == LongOperationState::Aborted || mState == LongOperationState::Finished)
		return;

	if(mError.empty())
		mError = error;
	else
		mError = mError + "\n" + error;

	mState = LongOperationState::Failed;
}

void LongOperationTracker::NotifyProgressChanged(float progressPercent)
{
	Lock lock(mMutex);

	if(mState != LongOperationState::Running)
		return;

	mProgressPercent = progressPercent;
}
