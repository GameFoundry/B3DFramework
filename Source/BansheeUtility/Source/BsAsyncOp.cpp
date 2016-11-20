//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsAsyncOp.h"
#include "BsDebug.h"

namespace bs
{
	bool AsyncOp::hasCompleted() const 
	{ 
		return mData->mIsCompleted.load(std::memory_order_acquire);
	}

	void AsyncOp::_completeOperation(Any returnValue) 
	{ 
		mData->mReturnValue = returnValue; 
		mData->mIsCompleted.store(true, std::memory_order_release);

		if (mSyncData != nullptr)
			mSyncData->mCondition.notify_all();
	}

	void AsyncOp::_completeOperation() 
	{ 
		mData->mIsCompleted.store(true, std::memory_order_release);

		if (mSyncData != nullptr)
			mSyncData->mCondition.notify_all();
	}

	void AsyncOp::blockUntilComplete() const
	{
		if (mSyncData == nullptr)
		{
			LOGERR("No sync data is available. Cannot block until AsyncOp is complete.");
			return;
		}

		Lock lock(mSyncData->mMutex);
		while (!hasCompleted())
			mSyncData->mCondition.wait(lock);
	}
}