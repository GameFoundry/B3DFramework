//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Extensions/BsAsyncOpEx.h"
#include "CoreThread/BsCoreThread.h"

namespace bs
{
	AsyncOpEx::AsyncOpEx(const AsyncOp& op, const std::function<MonoObject*(const AsyncOp&)>& convertCallback)
		:mAsyncOp(op), mConvertCallback(convertCallback)
	{ }

	bool AsyncOpEx::isComplete() const
	{
		return mAsyncOp.hasCompleted();
	}

	MonoObject* AsyncOpEx::getReturnValue() const
	{
		if (!mAsyncOp.hasCompleted())
			return nullptr;

		if (mConvertCallback == nullptr)
			return nullptr;

		return mConvertCallback(mAsyncOp);
	}

	void AsyncOpEx::blockUntilComplete() const
	{
		if (!mAsyncOp.hasCompleted())
		{
			gCoreThread().submit();
			mAsyncOp.blockUntilComplete();
		}
	}
}