//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Prerequisites/BsPrerequisitesUtil.h"
#include "Allocators/BsStackAlloc.h"

namespace bs
{
	BS_THREADLOCAL MemStackInternal<1024 * 1024>* MemStack::ThreadMemStack = nullptr;

	void MemStack::BeginThread()
	{
		if(ThreadMemStack != nullptr)
			endThread();

		ThreadMemStack = bs_new<MemStackInternal<1024 * 1024>>();
	}

	void MemStack::EndThread()
	{
		if(ThreadMemStack != nullptr)
		{
			bs_delete(ThreadMemStack);
			ThreadMemStack = nullptr;
		}
	}

	UINT8* MemStack::alloc(UINT32 numBytes)
	{
		assert(ThreadMemStack != nullptr && "Stack allocation failed. Did you call beginThread?");

		return ThreadMemStack->alloc(numBytes);
	}

	void MemStack::DeallocLast(UINT8* data)
	{
		assert(ThreadMemStack != nullptr && "Stack deallocation failed. Did you call beginThread?");

		ThreadMemStack->dealloc(data);
	}
}
